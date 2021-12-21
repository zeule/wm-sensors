// SPDX-License-Identifier: LGPL-3.0+

#include "./it87xx.hxx"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
	using namespace wm_sensors::stdtypes;

	const u8 ADDRESS_REGISTER_OFFSET = 0x05;

	const u8 CONFIGURATION_REGISTER = 0x00;
	const u8 DATA_REGISTER_OFFSET = 0x06;
	const u8 FAN_TACHOMETER_16BIT_REGISTER = 0x0C;
	const u8 FAN_TACHOMETER_DIVISOR_REGISTER = 0x0B;

	const u8 ITE_VENDOR_IDS[] = {0x90, 0x7F};

	const u8 TEMPERATURE_BASE_REG = 0x29;
	const u8 VENDOR_ID_REGISTER = 0x58;
	const u8 VOLTAGE_BASE_REG = 0x20;

	const u8 FAN_PWM_CTRL_EXT_REG[] = {0x63, 0x6b, 0x73, 0x7b, 0xa3};
	const u8 FAN_TACHOMETER_EXT_REG[] = {0x18, 0x19, 0x1a, 0x81, 0x83, 0x4d};
	const u8 FAN_TACHOMETER_REG[] = {0x0d, 0x0e, 0x0f, 0x80, 0x82, 0x4c};

	// Address of the Fan Controller Main Control Register.
	// No need for the 2nd control register (bit 7 of 0x15 0x16 0x17),
	// as PWM value will set it to manual mode when new value is set.
	const u8 FAN_MAIN_CTRL_REG = 0x13;
} // namespace

wm_sensors::hardware::motherboard::lpc::superio::IT87xx::IT87xxPort::IT87xxPort(SingleBankAddress ports, bool validationPosible)
    : base{ports}
    , validationPosible_{validationPosible}
{
}

wm_sensors::u8
wm_sensors::hardware::motherboard::lpc::superio::IT87xx::IT87xxPort::readByte(u8 registerIndex, bool& valid) const
{
	u8 res = base::readByte(registerIndex);
	valid = !validationPosible_ || registerIndex == base::inByte(base::regs().indexRegOffset);
	return res;
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::IT87xxPort::writeByte(u8 registerIndex, u8 value)
{
	base::writeByte(registerIndex, value);
	base::inByte(base::regs().indexRegOffset);
}

wm_sensors::hardware::motherboard::lpc::superio::IT87xx::IT87xx(
    MotherboardId board, Chip chip, u16 address, u16 gpioAddress, u8 version)
    : base{board, chip, address, channelCount(chip, version)}
    // // IT8688E doesn't return the value we wrote to addressReg when we read it back.
    , port_{{address, {ADDRESS_REGISTER_OFFSET, DATA_REGISTER_OFFSET}}, chip != Chip::IT8688E}
    , gpioPort_{gpioAddress}
    , fansDisabled_(nrChannels(SensorType::fan), false)
    , fanPWMControlReg_{chip == Chip::IT8665E ? std::vector<u8>{0x15, 0x16, 0x17, 0x1e, 0x1f} : std::vector<u8>{0x15, 0x16, 0x17, 0x7f, 0xa7}}
    // Older IT8705F and IT8721F revisions do not have 16-bit fan counters.
    , has16BitFanCounter_{(chip != Chip::IT8705F || version >= 3) && (chip != Chip::IT8712F || version >= 8)}
    , hasExtReg_{hasExtReg(chip)}
    , initialFanOutputModeEnabled_{}
    , initialFanPwmControl_{}
    , initialFanPwmControlExt_{}
    , restoreDefaultFanPwmControlRequired_{}
    , voltageGain_{voltageGain(chip)}
    , version_{version}
    , gpioCount_{gpioPinCount(chip)}
{
	// Check vendor id
	bool valid;
	u8 vendorId = port_.readByte(VENDOR_ID_REGISTER, valid);
	if (!valid) { return; }
	bool hasMatchingVendorId =
	    std::any_of(std::begin(ITE_VENDOR_IDS), std::end(ITE_VENDOR_IDS), [vendorId](u8 id) { return id == vendorId; });
	if (!hasMatchingVendorId) { return; }

	// Bit 0x10 of the configuration register should always be 1
	u8 configuration = port_.readByte(CONFIGURATION_REGISTER, valid);
	if (!valid || ((configuration & 0x10) == 0 && chip != Chip::IT8655E && chip != Chip::IT8665E)) { return; }

	// Disable any fans that aren't set with 16-bit fan counters
	if (has16BitFanCounter_) {
		int modes = port_.readByte(FAN_TACHOMETER_16BIT_REGISTER, valid);

		if (!valid) return;


		if (nrChannels(SensorType::fan) >= 5) {
			fansDisabled_[3] = (modes & (1 << 4)) == 0;
			fansDisabled_[4] = (modes & (1 << 5)) == 0;
		}

		if (nrChannels(SensorType::fan) >= 6) fansDisabled_[5] = (modes & (1 << 2)) == 0;
	}
}

std::map<wm_sensors::SensorType, std::size_t>
wm_sensors::hardware::motherboard::lpc::superio::IT87xx::channelCount(Chip chip, u8 /*version*/)
{
	switch (chip) {
		case Chip::IT8628E:
			return {
			    {SensorType::in, 10},
			    {SensorType::temp, 6},
			    {SensorType::fan, 4},
			    {SensorType::pwm, 4},
			};
		case Chip::IT8631E:
			return {
			    {SensorType::in, 9},
			    {SensorType::temp, 2},
			    {SensorType::fan, 2},
			    {SensorType::pwm, 2},
			};
		case Chip::IT8665E:
		case Chip::IT8686E:
			return {
			    {SensorType::in, 10},
			    {SensorType::temp, 6},
			    {SensorType::fan, 6},
			    {SensorType::pwm, 5},
			};
		case Chip::IT8688E:
			return {
			    {SensorType::in, 11},
			    {SensorType::temp, 6},
			    {SensorType::fan, 6},
			    {SensorType::pwm, 5},
			};
		case Chip::IT8689E:
			return {
			    {SensorType::in, 10},
			    {SensorType::temp, 6},
			    {SensorType::fan, 5},
			    {SensorType::pwm, 5},
			};
		case Chip::IT8655E:
			return {
			    {SensorType::in, 9},
			    {SensorType::temp, 6},
			    {SensorType::fan, 3},
			    {SensorType::pwm, 3},
			};
		case Chip::IT879XE:
			return {
			    {SensorType::in, 9},
			    {SensorType::temp, 3},
			    {SensorType::fan, 3},
			    {SensorType::pwm, 3},
			};
		case Chip::IT8705F:
			return {
			    {SensorType::in, 9},
			    {SensorType::temp, 3},
			    {SensorType::fan, 3},
			    {SensorType::pwm, 3},
			};
		default:
			return {
			    {SensorType::in, 9},
			    {SensorType::temp, 3},
			    {SensorType::fan, 5},
			    {SensorType::pwm, 3},
			};
	}
}

bool wm_sensors::hardware::motherboard::lpc::superio::IT87xx::hasExtReg(Chip chip)
{
	return chip == Chip::IT8721F || chip == Chip::IT8728F || chip == Chip::IT8665E || chip == Chip::IT8686E ||
	       chip == Chip::IT8688E || chip == Chip::IT8689E || chip == Chip::IT8628E || chip == Chip::IT8620E ||
	       chip == Chip::IT879XE || chip == Chip::IT8655E || chip == Chip::IT8631E;
}

float wm_sensors::hardware::motherboard::lpc::superio::IT87xx::voltageGain(Chip chip)
{
	switch (chip) {
			// IT8620E, IT8628E, IT8721F, IT8728F, IT8772E and IT8686E use a 12mV resolution.
			// All others 16mV.
		case Chip::IT8620E:
		case Chip::IT8628E:
		case Chip::IT8631E:
		case Chip::IT8721F:
		case Chip::IT8728F:
		case Chip::IT8771E:
		case Chip::IT8772E:
		case Chip::IT8686E:
		case Chip::IT8688E:
		case Chip::IT8689E: return 0.012f;
		case Chip::IT8655E:
		case Chip::IT8665E:
		case Chip::IT879XE: return 0.0109f;
		default: return 0.016f;
	}
}

wm_sensors::u8 wm_sensors::hardware::motherboard::lpc::superio::IT87xx::gpioPinCount(Chip chip)
{
	switch (chip) {
		case Chip::IT8712F:
		case Chip::IT8716F:
		case Chip::IT8718F:
		case Chip::IT8726F: return 5;
		case Chip::IT8720F:
		case Chip::IT8721F: return 8;
		default: return 0;
	}
}

std::optional<wm_sensors::u8> wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readGPIO(u8 index)
{
	return index < gpioCount_ ? std::make_optional(gpioPort_.inByte(index)) :
                                std::optional<u8>();
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::writeGPIO(u8 index, u8 value)
{
	if (index < gpioCount_) { gpioPort_.outByte(index, value); }
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readSIO(
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

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::writeSIO(
    SensorType type, std::size_t channel, double value)
{
	if (type != SensorType::pwm || channel >= this->nrChannels(type)) return;

	// 	impl::GlobalMutexLock isaMutex{impl::GlobalMutex::ISABus, 10};

	if (std::isnan(value)) {
		restoreDefaultFanPwmControl(channel);
	} else {
		saveDefaultFanPwmControl(channel);
		bool valid;
		if (channel < 3 && !initialFanOutputModeEnabled_[channel]) {
			port_.writeByte(FAN_MAIN_CTRL_REG, static_cast<u8>(port_.readByte(FAN_MAIN_CTRL_REG, valid) | (1 << channel)));
		}

		if (hasExtReg_) {
			port_.writeByte(fanPWMControlReg_[channel], static_cast<u8>(initialFanPwmControl_[channel] & 0x7F));
			port_.writeByte(FAN_PWM_CTRL_EXT_REG[channel], static_cast<u8>(value));
		} else {
			port_.writeByte(fanPWMControlReg_[channel], static_cast<u8>(static_cast<u8>(value) >> 1));
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readVoltages(
    std::size_t channelMin, std::size_t count, double* values) const
{
	bool valid;
	for (auto i = channelMin; i < channelMin + count; i++) {
		float value = voltageGain_ * port_.readByte(static_cast<u8>(VOLTAGE_BASE_REG + i), valid);

		if (valid) { values[i - channelMin] = value > 0 ? value : std::numeric_limits<float>::quiet_NaN(); }
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readTemperatures(
    std::size_t channelMin, std::size_t count, double* values) const
{
	bool valid;
	for (auto i = channelMin; i < channelMin + count; i++) {
		s8 value = static_cast<s8>(port_.readByte(static_cast<u8>(TEMPERATURE_BASE_REG + i), valid));
		if (valid) { values[i - channelMin] = value > 0 ? value : std::numeric_limits<float>::quiet_NaN(); }
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readFans(
    std::size_t channelMin, std::size_t count, double* values) const
{
	bool valid;
	if (has16BitFanCounter_) {
		for (auto i = channelMin; i < channelMin + count; i++) {
			if (fansDisabled_[i]) continue;

			int value = port_.readByte(FAN_TACHOMETER_REG[i], valid);
			if (!valid) continue;

			value |= port_.readByte(FAN_TACHOMETER_EXT_REG[i], valid) << 8;
			if (!valid) continue;


			if (value > 0x3f)
				values[i - channelMin] = value < 0xffff ? 1.35e6f / (static_cast<float>(value * 2)) : 0;
			else
				values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
		}
	} else {
		for (auto i = channelMin; i < channelMin + count; i++) {
			int value = port_.readByte(FAN_TACHOMETER_REG[i], valid);
			if (!valid) continue;

			int divisor = 2;
			if (i < 2) {
				int divisors = port_.readByte(FAN_TACHOMETER_DIVISOR_REGISTER, valid);
				if (!valid) continue;

				divisor = 1 << ((divisors >> (3 * i)) & 0x7);
			}

			if (value > 0)
				values[i - channelMin] = value < 0xff ? 1.35e6f / static_cast<float>(value * divisor) : 0.f;
			else
				values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::readPWMs(
    std::size_t channelMin, std::size_t count, double* values) const
{
	bool valid;
	for (auto i = channelMin; i < channelMin + count; i++) {
		u8 value = port_.readByte(fanPWMControlReg_[i], valid);
		if (!valid) continue;

		if ((value & 0x80) > 0) {
			// Automatic operation (value can't be read).
			values[i - channelMin] = std::numeric_limits<float>::quiet_NaN();
		} else {
			// Software operation.
			if (hasExtReg_) {
				value = port_.readByte(FAN_PWM_CTRL_EXT_REG[i], valid);
				if (valid) values[i - channelMin] = std::round(value * 100.0f / 0xFF);
			} else {
				values[i - channelMin] = std::round(static_cast<float>(value & 0x7F) * 100.0f / 0x7F);
			}
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::saveDefaultFanPwmControl(std::size_t channel)
{
	bool valid;
	if (!restoreDefaultFanPwmControlRequired_[channel]) {
		initialFanPwmControl_[channel] = port_.readByte(fanPWMControlReg_[channel], valid);

		if (channel < 3)
			initialFanOutputModeEnabled_[channel] =
			    port_.readByte(FAN_MAIN_CTRL_REG, valid) != 0; // Save default control reg value.

		if (hasExtReg_) initialFanPwmControlExt_[channel] = port_.readByte(FAN_PWM_CTRL_EXT_REG[channel], valid);
	}

	restoreDefaultFanPwmControlRequired_[channel] = true;
}

void wm_sensors::hardware::motherboard::lpc::superio::IT87xx::restoreDefaultFanPwmControl(std::size_t channel)
{
	bool valid;
	if (restoreDefaultFanPwmControlRequired_[channel]) {
		port_.writeByte(fanPWMControlReg_[channel], initialFanPwmControl_[channel]);

		if (channel < 3) {
			u8 value = port_.readByte(FAN_MAIN_CTRL_REG, valid);

			bool isEnabled = (value & (1 << channel)) != 0;
			if (isEnabled != initialFanOutputModeEnabled_[channel])
				port_.writeByte(FAN_MAIN_CTRL_REG, static_cast<u8>(value ^ (1 << channel)));
		}

		if (hasExtReg_) { port_.writeByte(FAN_PWM_CTRL_EXT_REG[channel], initialFanPwmControlExt_[channel]); }

		restoreDefaultFanPwmControlRequired_[channel] = false;
	}
}
