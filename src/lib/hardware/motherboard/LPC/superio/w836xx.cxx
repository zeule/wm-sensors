// SPDX-License-Identifier: LGPL-3.0+

#include "./w836xx.hxx"

#include <cmath>
#include <limits>

namespace {
	using namespace wm_sensors::stdtypes;

	const u8 ADDRESS_REGISTER_OFFSET = 0x05;
	const u8 BANK_SELECT_REGISTER = 0x4E;
	const u8 DATA_REGISTER_OFFSET = 0x06;
	const u8 HIGH_BYTE = 0x80;
	const u8 TEMPERATURE_SOURCE_SELECT_REG = 0x49;
	const u8 VENDOR_ID_REGISTER = 0x4F;
	const u8 VOLTAGE_VBAT_REG = 0x51;

	const u16 WINBOND_VENDOR_ID = 0x5CA3;

	const u8 FAN_BIT_REG[] = {0x47, 0x4B, 0x4C, 0x59, 0x5D};
	const u8 FAN_DIV_BIT0[] = {36, 38, 30, 8, 10};
	const u8 FAN_DIV_BIT1[] = {37, 39, 31, 9, 11};
	const u8 FAN_DIV_BIT2[] = {5, 6, 7, 23, 15};
	const u8 FAN_TACHO_BANK[] = {0, 0, 0, 0, 5};
	const u8 FAN_TACHO_REG[] = {0x28, 0x29, 0x2A, 0x3F, 0x53};
	const u8 TEMPERATURE_BANK[] = {1, 2, 0};
	const u8 TEMPERATURE_REG[] = {0x50, 0x50, 0x27};
} // namespace

wm_sensors::hardware::motherboard::lpc::superio::W836xx::W836xx(
    MotherboardId board, Chip chip, u8 revision, u16 address)
    : base{board, chip, address, channelCount(chip, revision)}
    , port_{{address, {ADDRESS_REGISTER_OFFSET, DATA_REGISTER_OFFSET}}, BANK_SELECT_REGISTER}
    , peciTemperature_(base::nrChannels(SensorType::temp), false)
    , revision_{revision}
{
	if (!isWinbondVendor())
		return;

	switch (chip) {
		case Chip::W83667HG:
		case Chip::W83667HGB: {
			// note temperature sensor registers that read PECI
			u8 flag = port_.readByte(0, TEMPERATURE_SOURCE_SELECT_REG);
			peciTemperature_[0] = (flag & 0x04) != 0;
			peciTemperature_[1] = (flag & 0x40) != 0;
			peciTemperature_[2] = false;
			break;
		}
		case Chip::W83627DHG:
		case Chip::W83627DHGP: {
			// note temperature sensor registers that read PECI
			u8 sel = port_.readByte(0, TEMPERATURE_SOURCE_SELECT_REG);
			peciTemperature_[0] = (sel & 0x07) != 0;
			peciTemperature_[1] = (sel & 0x70) != 0;
			peciTemperature_[2] = false;
			break;
		}
		default: {
			// no PECI support
			peciTemperature_[0] = false;
			peciTemperature_[1] = false;
			peciTemperature_[2] = false;
			break;
		}
	}

	switch (chip) {
		case Chip::W83627EHF: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x50, 0x51, 0x52};
			voltageBank_ = {0, 0, 0, 0, 0, 0, 0, 5, 5, 5};
			voltageGain_ = 0.008f;
			fanPwmRegister_ = {0x01, 0x03, 0x11};                           // Fan PWM values.
			fanPrimaryControlModeRegister_ = {0x04, 0x04, 0x12};            // Primary control register.
			fanPrimaryControlValue_ = {0b11110011, 0b11001111, 0b11111001}; // Values to gain control of fans.
			initialFanControlValue_ = {0, 0, 0};                            // To store primary default value.
			initialFanSecondaryControlValue_ = {0, 0, 0};                   // To store secondary default value.
			break;
		}
		case Chip::W83627DHG:
		case Chip::W83627DHGP: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x50, 0x51};
			voltageBank_ = {0, 0, 0, 0, 0, 0, 0, 5, 5};
			voltageGain_ = 0.008f;
			fanPwmRegister_ = {0x01, 0x03, 0x11};                           // Fan PWM values
			fanPrimaryControlModeRegister_ = {0x04, 0x04, 0x12};            // added. Primary control register
			fanPrimaryControlValue_ = {0b11110011, 0b11001111, 0b11111001}; // Values to gain control of fans
			initialFanControlValue_ = {0, 0, 0};                            // To store primary default value
			initialFanSecondaryControlValue_ = {0, 0, 0};                   // To store secondary default value.
			break;
		}
		case Chip::W83667HG:
		case Chip::W83667HGB: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x50, 0x51};
			voltageBank_ = {0, 0, 0, 0, 0, 0, 0, 5, 5};
			voltageGain_ = 0.008f;
			fanPwmRegister_ = {0x01, 0x03, 0x11};                           // Fan PWM values.
			fanPrimaryControlModeRegister_ = {0x04, 0x04, 0x12};            // Primary control register.
			fanPrimaryControlValue_ = {0b11110011, 0b11001111, 0b11111001}; // Values to gain control of fans.
			fanSecondaryControlModeRegister_ = {0x7c, 0x7c, 0x7c};          // Secondary control register for SmartFan4.
			fanSecondaryControlValue_ = {
			    0b11101111, 0b11011111, 0b10111111}; // Values for secondary register to gain control of fans.
			fanTertiaryControlModeRegister_ = {
			    0x62, 0x7c, 0x62}; // Tertiary control register. 2nd fan doesn't have Tertiary control, same as
			                       // secondary to avoid change.
			fanTertiaryControlValue_ = {
			    0b11101111, 0b11011111,
			    0b11011111}; // Values for tertiary register to gain control of fans. 2nd fan doesn't have Tertiary
			                 // control, same as secondary to avoid change.
			initialFanControlValue_ = {0, 0, 0};          // To store primary default value.
			initialFanSecondaryControlValue_ = {0, 0, 0}; // To store secondary default value.
			initialFanTertiaryControlValue_ = {0, 0, 0};  // To store tertiary default value.
			break;
		}
		case Chip::W83627HF: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x50, 0x51};
			voltageBank_ = {0, 0, 0, 0, 0, 5, 5};
			voltageGain_ = 0.016f;
			fanPwmRegister_ = {0x5A, 0x5B}; // Fan PWM values.
			break;
		}
		case Chip::W83627THF: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x50, 0x51};
			voltageBank_ = {0, 0, 0, 0, 0, 5, 5};
			voltageGain_ = 0.016f;
			fanPwmRegister_ = {0x01, 0x03, 0x11};                           // Fan PWM values.
			fanPrimaryControlModeRegister_ = {0x04, 0x04, 0x12};            // Primary control register.
			fanPrimaryControlValue_ = {0b11110011, 0b11001111, 0b11111001}; // Values to gain control of fans.
			initialFanControlValue_ = {0, 0, 0};                            // To store primary default value.
			break;
		}
		case Chip::W83687THF: {
			voltageRegister_ = {0x20, 0x21, 0x22, 0x23, 0x24, 0x50, 0x51};
			voltageBank_ = {0, 0, 0, 0, 0, 5, 5};
			voltageGain_ = 0.016f;
			break;
			default: break;
		}
	}
}

bool wm_sensors::hardware::motherboard::lpc::superio::W836xx::isWinbondVendor()
{
	u16 vendorId = utility::word(port_.readByte(HIGH_BYTE, VENDOR_ID_REGISTER), port_.readByte(0, VENDOR_ID_REGISTER));
	return vendorId == WINBOND_VENDOR_ID;
}

std::map<wm_sensors::SensorType, std::size_t>
wm_sensors::hardware::motherboard::lpc::superio::W836xx::channelCount(Chip chip, u8 /*revision*/)
{
	std::map<SensorType, std::size_t> res;
	res[SensorType::temp] = 3;

	switch (chip) {
		case Chip::W83627EHF:
			res[SensorType::in] = 10;
			res[SensorType::fan] = 5;
			res[SensorType::pwm] = 3;
			break;
		case Chip::W83627DHG:
		case Chip::W83627DHGP:
		case Chip::W83667HG:
		case Chip::W83667HGB:
			res[SensorType::in] = 9;
			res[SensorType::fan] = 5;
			res[SensorType::pwm] = 3;
			break;
		case Chip::W83627HF:
			res[SensorType::in] = 7;
			res[SensorType::fan] = 3;
			res[SensorType::pwm] = 2;
			break;
		case Chip::W83627THF:
			res[SensorType::in] = 7;
			res[SensorType::fan] = 3;
			res[SensorType::pwm] = 3;
			break;
		case Chip::W83687THF:
			res[SensorType::in] = 7;
			res[SensorType::fan] = 3;
			break;
		default: break;
	}
	return res;
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::writeSIO(
    SensorType type, std::size_t channel, double value)
{
	switch (type) {
		case SensorType::pwm: return writePWM(channel, value);
		default: break;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::readSIO(
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

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::writePWM(std::size_t channel, double value)
{
	// 	impl::GlobalMutexLock isaMutex{impl::GlobalMutex::ISABus, 10};
	if (std::isnan(value)) {
		restoreDefaultFanPwmControl(channel);
	} else {
		saveDefaultFanPwmControl(channel);
		if (fanPrimaryControlModeRegister_.size()) {
			port_.writeByte(
			    0, fanPrimaryControlModeRegister_[channel],
			    static_cast<u8>(fanPrimaryControlValue_[channel] & port_.readByte(0, fanPrimaryControlModeRegister_[channel])));
			if (fanSecondaryControlModeRegister_.size()) {
				if (fanSecondaryControlModeRegister_[channel] != fanPrimaryControlModeRegister_[channel]) {
					port_.writeByte(
					    0, fanSecondaryControlModeRegister_[channel],
					    static_cast<u8>(fanSecondaryControlValue_[channel] &
					        port_.readByte(0, fanSecondaryControlModeRegister_[channel])));
				}
				if (fanTertiaryControlModeRegister_.size()) {
					if (fanTertiaryControlModeRegister_[channel] != fanSecondaryControlModeRegister_[channel]) {
						port_.writeByte(
						    0, fanTertiaryControlModeRegister_[channel],
						    static_cast<u8>(fanTertiaryControlValue_[channel] &
						        port_.readByte(0, fanTertiaryControlModeRegister_[channel])));
					}
				}
			}
		}
		// set output value
		port_.writeByte(0, fanPwmRegister_[channel], static_cast<u8>(value));
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::saveDefaultFanPwmControl(std::size_t channel)
{
	if (fanPrimaryControlModeRegister_.size() && initialFanControlValue_.size() && fanPrimaryControlValue_.size() &&
	    restoreDefaultFanPwmControlRequired_.size()) {
		if (!restoreDefaultFanPwmControlRequired_[channel]) {
			initialFanControlValue_[channel] = port_.readByte(0, fanPrimaryControlModeRegister_[channel]);
			if (fanSecondaryControlModeRegister_.size() && initialFanSecondaryControlValue_.size() &&
			    fanSecondaryControlValue_.size()) {
				if (fanSecondaryControlModeRegister_[channel] != fanPrimaryControlModeRegister_[channel]) {
					initialFanSecondaryControlValue_[channel] =
					    port_.readByte(0, fanSecondaryControlModeRegister_[channel]);
				}
				if (fanTertiaryControlModeRegister_.size() && initialFanTertiaryControlValue_.size() &&
				    fanTertiaryControlValue_.size()) {
					if (fanTertiaryControlModeRegister_[channel] != fanSecondaryControlModeRegister_[channel]) {
						initialFanTertiaryControlValue_[channel] =
						    port_.readByte(0, fanTertiaryControlModeRegister_[channel]);
					}
				}
			}
			restoreDefaultFanPwmControlRequired_[channel] = true;
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::restoreDefaultFanPwmControl(std::size_t channel)
{
	if (fanPrimaryControlModeRegister_.size() && initialFanControlValue_.size() && fanPrimaryControlValue_.size() &&
	    restoreDefaultFanPwmControlRequired_.size()) {
		if (restoreDefaultFanPwmControlRequired_[channel]) {
			port_.writeByte(
			    0, fanPrimaryControlModeRegister_[channel],
			    static_cast<u8>((initialFanControlValue_[channel] & ~fanPrimaryControlValue_[channel]) |
			        port_.readByte(
			            0, fanPrimaryControlModeRegister_[channel]))); // bitwise operands to change only desired bits
			if (fanSecondaryControlModeRegister_.size() && initialFanSecondaryControlValue_.size() &&
			    fanSecondaryControlValue_.size()) {
				if (fanSecondaryControlModeRegister_[channel] != fanPrimaryControlModeRegister_[channel]) {
					port_.writeByte(
					    0, fanSecondaryControlModeRegister_[channel],
					    static_cast<u8>((initialFanSecondaryControlValue_[channel] & ~fanSecondaryControlValue_[channel]) |
					     port_.readByte(0, fanSecondaryControlModeRegister_[channel]))); // bitwise operands to change
					                                                                     // only desired bits
				}
				if (fanTertiaryControlModeRegister_.size() && initialFanTertiaryControlValue_.size() &&
				    fanTertiaryControlValue_.size()) {
					if (fanTertiaryControlModeRegister_[channel] != fanSecondaryControlModeRegister_[channel]) {
						port_.writeByte(
						    0, fanTertiaryControlModeRegister_[channel],
						    static_cast<u8>((initialFanTertiaryControlValue_[channel] & ~fanTertiaryControlValue_[channel]) |
						     port_.readByte(0, fanTertiaryControlModeRegister_[channel]))); // bitwise operands to
						                                                                    // change only desired bits
					}
				}
			}
			restoreDefaultFanPwmControlRequired_[channel] = false;
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::readVoltages(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		if (voltageRegister_[i] != VOLTAGE_VBAT_REG) {
			// two special VCore measurement modes for W83627THF
			float fValue;
			if ((chip() == Chip::W83627HF || chip() == Chip::W83627THF || chip() == Chip::W83687THF) && i == 0) {
				u8 vrmConfiguration = port_.readByte(0, 0x18);
				float value = static_cast<float>(port_.readByte(voltageBank_[i], voltageRegister_[i]));
				if ((vrmConfiguration & 0x01) == 0)
					fValue = 0.016f * value; // VRM8 formula
				else
					fValue = 0.00488f * value + 0.69f; // VRM9 formula
			} else {
				float value = static_cast<float>(port_.readByte(voltageBank_[i], voltageRegister_[i]));
				fValue = voltageGain_ * value;
			}

			values[i - channelMin] = fValue > 0 ? fValue : std::numeric_limits<float>::quiet_NaN();
		} else {
			// Battery voltage
			bool valid = (port_.readByte(0, 0x5D) & 0x01) > 0;
			values[i - channelMin] =
			    valid ? voltageGain_ * port_.readByte(5, VOLTAGE_VBAT_REG) : std::numeric_limits<float>::quiet_NaN();
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::readTemperatures(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		int value = static_cast<s8>(port_.readByte(TEMPERATURE_BANK[i], TEMPERATURE_REG[i])) << 1;
		if (TEMPERATURE_BANK[i] > 0)
			value |= port_.readByte(TEMPERATURE_BANK[i], static_cast<u8>(TEMPERATURE_REG[i] + 1)) >> 7;

		float temperature = static_cast<float>(value) / 2.0f;
		values[i - channelMin] = temperature <= 125 && temperature >= -55 && !peciTemperature_[i] ?
		                             temperature :
                                     std::numeric_limits<float>::quiet_NaN();
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::readFans(
    std::size_t channelMin, std::size_t count, double* values) const
{
	u64 bits = 0;
	for (u8 t: FAN_BIT_REG)
		bits = (bits << 8) | port_.readByte(0, t);

	// ulong newBits = bits; unused?
	for (auto i = channelMin; i < channelMin + count; i++) {
		int cnt = port_.readByte(FAN_TACHO_BANK[i], FAN_TACHO_REG[i]);

		// assemble fan divisor
		auto divisorBits =
		    ((((bits >> FAN_DIV_BIT2[i]) & 1) << 2) | (((bits >> FAN_DIV_BIT1[i]) & 1) << 1) |
		     ((bits >> FAN_DIV_BIT0[i]) & 1));

		int divisor = 1 << divisorBits;

		float value = cnt < 0xff ? 1.35e6f / static_cast<float>(cnt * divisor) : 0;
		values[i - channelMin] = value;

		// update fan divisor
		if (cnt > 192 && divisorBits < 7)
			divisorBits++;

		if (cnt < 96 && divisorBits > 0)
			divisorBits--;

		// newBits = SetBit(newBits, FAN_DIV_BIT2[i], (divisorBits >> 2) & 1);
		// newBits = SetBit(newBits, FAN_DIV_BIT1[i], (divisorBits >> 1) & 1);
		// newBits = SetBit(newBits, FAN_DIV_BIT0[i], divisorBits & 1);
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::W836xx::readPWMs(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		u8 value = port_.readByte(0, fanPwmRegister_[i]);
		values[i - channelMin] = std::round(value * 100.0f / 0xFF);
	}
}
