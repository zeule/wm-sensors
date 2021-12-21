// SPDX-License-Identifier: LGPL-3.0+

#include "./f718xx.hxx"

#include "../../../impl/ring0.hxx"

#include <cmath>
#include <limits>

namespace {
	using wm_sensors::u8;

	const u8 addressRegisterOffset = 0x05;
	const u8 dataRegisterOffset = 0x06;
	const u8 pwmValuesOffset = 0x2D;
	const u8 temperatureBaseReg = 0x70;
	const u8 temperatureConfigReg = 0x69;

	const u8 voltageBaseReg = 0x20;
	const u8 fanPwmReg[] = {0xA3, 0xB3, 0xC3, 0xD3};
	const u8 fanTachometerReg[] = {0xA0, 0xB0, 0xC0, 0xD0};
} // namespace

wm_sensors::hardware::motherboard::lpc::superio::F718xx::F718xx(MotherboardId board, Chip chip, u16 address)
    : base{board, chip, address, channelCount(chip)}
    , port_{{address, {addressRegisterOffset, dataRegisterOffset}}}
    , restoreDefaultFanPWMValuesRequired_{false, false, false, false}
{
}

std::map<wm_sensors::SensorType, std::size_t>
wm_sensors::hardware::motherboard::lpc::superio::F718xx::channelCount(Chip chip)
{
	std::map<SensorType, std::size_t> res;
	res[SensorType::in] = chip == Chip::F71858 ? 3u : 9u;
	res[SensorType::temp] = chip == Chip::F71808E ? 2u : 3u;
	res[SensorType::fan] = chip == Chip::F71882 || chip == Chip::F71858 ? 4u : 3u;
	res[SensorType::pwm] = chip == Chip::F71878AD ? 3u : 0;
	return res;
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::readSIO(
    SensorType type, std::size_t channelMin, std::size_t count, double* values) const
{
	switch (type) {
		case SensorType::in: return readVoltages(channelMin, count, values);
		case SensorType::temp: return readTemperatures(channelMin, count, values);
		case SensorType::fan: return readFans(channelMin, count, values);
		case SensorType::pwm: return readPWMs(channelMin, count, values);
		default: break;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::readVoltages(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		if (chip() == Chip::F71808E && i == 6) {
			// 0x26 is reserved on F71808E
			values[i - channelMin] = 0;
		} else {
			values[i - channelMin] = 0.008f * port_.readByte(static_cast<u8>(voltageBaseReg + i));
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::readTemperatures(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		switch (chip()) {
			case Chip::F71858: {
				int tableMode = 0x3 & port_.readByte(temperatureConfigReg);
				u8 high = port_.readByte(static_cast<u8>(temperatureBaseReg + 2 * i));
				u8 low = port_.readByte(static_cast<u8>(temperatureBaseReg + 2 * i + 1));
				if (high != 0xbb && high != 0xcc) {
					u16 bits = 0;
					switch (tableMode) {
						case 0: bits = 0; break;
						case 1: bits = 0; break;
						case 2: bits = static_cast<u16>((high & 0x80) << 8); break;
						case 3: bits = static_cast<u16>((low & 0x01) << 15); break;
					}

					bits |= high << 7;
					bits |= (low & 0xe0) >> 1;
					u16 value = static_cast<u16>(bits & 0xfff0);
					values[i - channelMin] = static_cast<float>(value) / 128.0f;
				} else {
					values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
				}
			}

			break;
			default: {
				s8 value = static_cast<s8>(port_.readByte(static_cast<u8>(temperatureBaseReg + 2 * (i + 1))));
				if (value < std::numeric_limits<s8>::max() && value > 0)
					values[i - channelMin] = value;
				else
					values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
				;
			} break;
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::readFans(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		s16 value = static_cast<s16>(port_.readWord(fanTachometerReg[i]));
		if (value > 0)
			values[i - channelMin] = value < 0x0fff ? 1.5e6f / value : 0;
		else
			values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::readPWMs(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		values[i - channelMin] = port_.readByte(static_cast<u8>(pwmValuesOffset + i)) * 100.0f / 0xFF;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::writeSIO(
    SensorType type, std::size_t channel, double value)
{
	if (type != SensorType::pwm || channel >= this->nrChannels(type)) return;

	// 	impl::GlobalMutexLock isaMutex{impl::GlobalMutex::ISABus, 10};

	if (std::isnan(value)) {
		restoreDefaultFanPwmControl(channel);
	} else {
		saveDefaultFanPwmControl(channel);
		port_.writeByte(fanPwmReg[channel], static_cast<u8>(value));
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::saveDefaultFanPwmControl(std::size_t channel)
{
	if (!restoreDefaultFanPWMValuesRequired_[channel]) {
		initialFanPWMValues_[channel] = port_.readByte(fanPwmReg[channel]);
		restoreDefaultFanPWMValuesRequired_[channel] = true;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::F718xx::restoreDefaultFanPwmControl(std::size_t channel)
{
	if (restoreDefaultFanPWMValuesRequired_[channel]) {
		port_.writeByte(fanPwmReg[channel], initialFanPWMValues_[channel]);
		restoreDefaultFanPWMValuesRequired_[channel] = false;
	}
}
