// SPDX-License-Identifier: LGPL-3.0+

#include "./nct677x.hxx"

#include "../../../../utility/utility.hxx"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cmath>
#include <limits>
#include <thread>
#include <type_traits>

using namespace std::chrono_literals;

namespace {
	using namespace wm_sensors::stdtypes;

	const u8 ADDRESS_REGISTER_OFFSET = 0x05;
	const u8 BANK_SELECT_REGISTER = 0x4E;
	const u8 DATA_REGISTER_OFFSET = 0x06;

	// NCT668X
	const u8 EC_SPACE_PAGE_REGISTER_OFFSET = 0x04;
	const u8 EC_SPACE_INDEX_REGISTER_OFFSET = 0x05;
	const u8 EC_SPACE_DATA_REGISTER_OFFSET = 0x06;
	const u8 EC_SPACE_PAGE_SELECT = 0xFF;

	const u16 NUVOTON_VENDOR_ID = 0x5CA3;
} // namespace

wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::TempSrcDef::TempSrcDef(
    Source src, u16 rg, u16 hRg, int hBit, u16 sReg, std::optional<u16> altReg)
    : alternateReg{altReg}
    , halfBit{hBit}
    , reg{rg}
    , halfReg{hRg}
    , sourceReg{sReg}
    , source{src}
{
}

wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::Nct67xx(
    MotherboardId board, Chip chip, u8 revision, u16 address, const SingleBankPort& port)
    : base{board, chip, address, channelCount(chip, revision)}
    , port_{portAddress(chip, address)}
    , lpcPort_{port}
    , vBatMonitorControlRegister_{0}
{
	if (!isNuvotonVendor())
		return; // TODO throw

	if (chip == Chip::NCT610XD) {
		fanPWMOutReg_ = {0x04A, 0x04B, 0x04C};
		fanPWMCommandReg_ = {0x119, 0x129, 0x139};
		fanControlModeReg_ = {0x113, 0x123, 0x133};

		vBatMonitorControlRegister_ = 0x0318;
	} else if (chip == Chip::NCT6687D || chip == Chip::NCT6683D) {
		fanPWMOutReg_ = {0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167};
		fanPWMCommandReg_ = {0xA28, 0xA29, 0xA2A, 0xA2B, 0xA2C, 0xA2D, 0xA2E, 0xA2F};
		fanControlModeReg_ = {0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xA00};
		fanPWMRequestReg_ = {0xA01, 0xA01, 0xA01, 0xA01, 0xA01, 0xA01, 0xA01, 0xA01};
	} else {
		if (chip == Chip::NCT6797D || chip == Chip::NCT6798D)
			fanPWMOutReg_ = {0x001, 0x003, 0x011, 0x013, 0x015, 0xA09, 0xB09};
		else
			fanPWMOutReg_ = {0x001, 0x003, 0x011, 0x013, 0x015, 0x017, 0x029};

		fanPWMCommandReg_ = {0x109, 0x209, 0x309, 0x809, 0x909, 0xA09, 0xB09};
		fanControlModeReg_ = {0x102, 0x202, 0x302, 0x802, 0x902, 0xA02, 0xB02};

		vBatMonitorControlRegister_ = 0x005D;
	}

	setupChipParameters(chip);
}

wm_sensors::hardware::motherboard::lpc::AddressWithBank
wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::portAddress(Chip chip, u16 address)
{
	if (chip == Chip::NCT6687D || chip == Chip::NCT6683D) {
		return {
		    {address, {EC_SPACE_INDEX_REGISTER_OFFSET, EC_SPACE_DATA_REGISTER_OFFSET}},
		    {EC_SPACE_PAGE_REGISTER_OFFSET, EC_SPACE_PAGE_REGISTER_OFFSET},
		    EC_SPACE_PAGE_SELECT};
	} else {
		return {
		    {address, {ADDRESS_REGISTER_OFFSET, DATA_REGISTER_OFFSET}},
		    {ADDRESS_REGISTER_OFFSET, DATA_REGISTER_OFFSET},
		    BANK_SELECT_REGISTER};
	}
}

std::map<wm_sensors::SensorType, std::size_t>
wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::channelCount(Chip chip, u8 /*version*/)
{
	std::map<SensorType, std::size_t> res;
	switch (chip) {
		case Chip::NCT6771F:
		case Chip::NCT6776F:
			res[SensorType::fan] = chip == Chip::NCT6771F ? 4u : 5u;
			res[SensorType::pwm] = 3;
			res[SensorType::in] = 9;
			res[SensorType::temp] = 4; // but 9 source data?
			break;
		case Chip::NCT6779D:
		case Chip::NCT6791D:
		case Chip::NCT6792D:
		case Chip::NCT6792DA:
		case Chip::NCT6793D:
		case Chip::NCT6795D:
		case Chip::NCT6796D:
		case Chip::NCT6796DR:
		case Chip::NCT6797D:
		case Chip::NCT6798D: {
			switch (chip) {
				case Chip::NCT6779D:
					res[SensorType::fan] = 5;
					res[SensorType::pwm] = 5;
					break;
				case Chip::NCT6796DR:
				case Chip::NCT6797D:
				case Chip::NCT6798D:
					res[SensorType::fan] = 7;
					res[SensorType::pwm] = 7;
					break;
				default:
					res[SensorType::fan] = 6;
					res[SensorType::pwm] = 6;
					break;
			}

			res[SensorType::in] = 15;
			switch (chip) {
				case Chip::NCT6796D:
				case Chip::NCT6796DR:
				case Chip::NCT6797D:
				case Chip::NCT6798D: res[SensorType::temp] = 24; break;
				default: res[SensorType::temp] = 7; break;
			}
			break;
		}
		case Chip::NCT610XD:
			res[SensorType::temp] = 4;
			res[SensorType::in] = 9;
			res[SensorType::fan] = 3;
			res[SensorType::pwm] = 3;
			break;
		case Chip::NCT6683D:
		case Chip::NCT6687D:
			res[SensorType::temp] = 7;
			res[SensorType::in] = 14;
			res[SensorType::fan] = 8;
			res[SensorType::pwm] = 8;
			break;
		default: break;
	}
	return res;
}

bool wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::isNuvotonVendor()
{
	if (chip() == Chip::NCT6687D || chip() == Chip::NCT6683D) {
		return true;
	}

	const u16 VENDOR_ID_HIGH_REGISTER = chip() == Chip::NCT610XD ? 0x80FE_u16 : 0x804F_u16;
	const u16 VENDOR_ID_LOW_REGISTER = chip() == Chip::NCT610XD ? 0x00FE_u16 : 0x004F_u16;

	return utility::word(port_.readByte(VENDOR_ID_HIGH_REGISTER), port_.readByte(VENDOR_ID_LOW_REGISTER)) ==
	       NUVOTON_VENDOR_ID;
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::saveDefaultFanPwmControl(std::size_t channel)
{
	if (!restoreDefaultFanControlRequired_[channel]) {
		if (chip() != Chip::NCT6687D && chip() != Chip::NCT6683D) {
			initialFanControlMode_[channel] = port_.readByte(fanControlModeReg_[channel]);
		} else {
			u8 mode = port_.readByte(fanControlModeReg_[channel]);
			u8 bitMask = static_cast<u8>(0x01 << channel);
			initialFanControlMode_[channel] = static_cast<u8>(mode & bitMask);
		}

		initialFanPwmCommand_[channel] = port_.readByte(fanPWMCommandReg_[channel]);
		restoreDefaultFanControlRequired_[channel] = true;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::restoreDefaultFanPwmControl(std::size_t channel)
{
	if (restoreDefaultFanControlRequired_[channel]) {
		if (chip() != Chip::NCT6687D && chip() != Chip::NCT6683D) {
			port_.writeByte(fanControlModeReg_[channel], initialFanControlMode_[channel]);
			port_.writeByte(fanPWMCommandReg_[channel], initialFanPwmCommand_[channel]);
		} else {
			u8 mode = port_.readByte(fanControlModeReg_[channel]);
			mode = static_cast<u8>(mode & ~initialFanControlMode_[channel]);
			port_.writeByte(fanControlModeReg_[channel], mode);

			port_.writeByte(fanPWMRequestReg_[channel], 0x80);
			std::this_thread::sleep_for(50ms);

			port_.writeByte(fanPWMCommandReg_[channel], initialFanPwmCommand_[channel]);
			port_.writeByte(fanPWMRequestReg_[channel], 0x40);
			std::this_thread::sleep_for(50ms);
		}

		restoreDefaultFanControlRequired_[channel] = false;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::setupChipParameters(Chip chip)
{
	switch (chip) {
		case Chip::NCT6771F:
		case Chip::NCT6776F:
			if (chip == Chip::NCT6771F) {
				// min value RPM value with 16-bit fan counter
				minFanRpm_ = static_cast<int>(1.35e6 / 0xFFFF);
			} else {
				// min value RPM value with 13-bit fan counter
				minFanRpm_ = static_cast<int>(1.35e6 / 0x1FFF);
			}

			fanRpmRegister_.resize(5);
			for (std::size_t i = 0; i < fanRpmRegister_.size(); i++)
				fanRpmRegister_[i] = static_cast<u16>(0x656 + (i << 1));

			voltageRegisters_ = {0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x550, 0x551};
			voltageVBatRegister_ = 0x551;
			temperaturesSource_.assign(
			    {{chip == Chip::NCT6771F ? Source::Nct6771F_PECI_0 : Source::Nct6776F_PECI_0, 0x027, 0, -1, 0x621},
			     {chip == Chip::NCT6771F ? Source::CPUTIN : Source::CPUTIN, 0x073, 0x074, 7, 0x100},
			     {chip == Chip::NCT6771F ? Source::AUXTIN : Source::AUXTIN, 0x075, 0x076, 7, 0x200},
			     {chip == Chip::NCT6771F ? Source::SYSTIN : Source::SYSTIN, 0x077, 0x078, 7, 0x300},
			     {Source::None, 0x150, 0x151, 7, 0x622},
			     {Source::None, 0x250, 0x251, 7, 0x623},
			     {Source::None, 0x62B, 0x62E, 0, 0x624},
			     {Source::None, 0x62C, 0x62E, 1, 0x625},
			     {Source::None, 0x62D, 0x62E, 2, 0x626}});
			break;
		case Chip::NCT6779D:
		case Chip::NCT6791D:
		case Chip::NCT6792D:
		case Chip::NCT6792DA:
		case Chip::NCT6793D:
		case Chip::NCT6795D:
		case Chip::NCT6796D:
		case Chip::NCT6796DR:
		case Chip::NCT6797D:
		case Chip::NCT6798D: {
			fanCountRegister_.assign({0x4B0, 0x4B2, 0x4B4, 0x4B6, 0x4B8, 0x4BA, 0x4CC});

			// max value for 13-bit fan counter
			maxFanCount_ = 0x1FFF;

			// min value that could be transferred to 16-bit RPM registers
			minFanCount_ = 0x15;

			voltageRegisters_ = {0x480, 0x481, 0x482, 0x483, 0x484, 0x485, 0x486, 0x487,
			                     0x488, 0x489, 0x48A, 0x48B, 0x48C, 0x48D, 0x48E};
			voltageVBatRegister_ = 0x488;
			switch (chip) {
				case Chip::NCT6796D:
				case Chip::NCT6796DR:
				case Chip::NCT6797D:
				case Chip::NCT6798D:
					temperaturesSource_.insert(
					    temperaturesSource_.end(),
					    {{Source::Nct67Xxd_PECI_0, 0x073_u16, 0x074_u16, 7, 0x100_u16},
					     {Source::CPUTIN, 0x075_u16, 0x076_u16, 7, 0x200, 0x491_u16},
					     {Source::SYSTIN, 0x077_u16, 0x078_u16, 7, 0x300, 0x490_u16},
					     {Source::Nct67Xxd_AUXTIN0, 0x079_u16, 0x07A_u16, 7, 0x800_u16, 0x492_u16},
					     {Source::Nct67Xxd_AUXTIN1, 0x07B_u16, 0x07C_u16, 7, 0x900_u16, 0x493_u16},
					     {Source::Nct67Xxd_AUXTIN2, 0x07D_u16, 0x07E_u16, 7, 0xA00_u16, 0x494_u16},
					     {Source::Nct67Xxd_AUXTIN3, 0x4A0_u16, 0x49E_u16, 6, 0xB00_u16, 0x495_u16},
					     {Source::Nct67Xxd_AUXTIN4, 0x027_u16, 0, -1, 0x621_u16},
					     {Source::Nct67Xxd_SMBUSMASTER0, 0x150_u16, 0x151_u16, 7, 0x622_u16},
					     {Source::Nct67Xxd_SMBUSMASTER1, 0x670_u16, 0, -1, 0xC26_u16},
					     {Source::Nct67Xxd_PECI_1, 0x672_u16, 0, -1, 0xC27_u16},
					     {Source::Nct67Xxd_PCH_CHIP_CPU_MAX_TEMP, 0x674_u16, 0, -1, 0xC28_u16, 0x400_u16},
					     {Source::Nct67Xxd_PCH_CHIP_TEMP, 0x676_u16, 0, -1, 0xC29, 0x401_u16},
					     {Source::Nct67Xxd_PCH_CPU_TEMP, 0x678_u16, 0, -1, 0xC2A_u16, 0x402_u16},
					     {Source::Nct67Xxd_PCH_MCH_TEMP, 0x67A_u16, 0, -1, 0xC2B_u16, 0x404_u16},
					     {Source::Nct67Xxd_AGENT0_DIMM0, 0},
					     {Source::Nct67Xxd_AGENT0_DIMM1, 0},
					     {Source::Nct67Xxd_AGENT1_DIMM0, 0},
					     {Source::Nct67Xxd_AGENT1_DIMM1, 0},
					     {Source::Nct67Xxd_BYTE_TEMP0, 0},
					     {Source::Nct67Xxd_BYTE_TEMP1, 0},
					     {Source::Nct67Xxd_PECI_0_CAL, 0},
					     {Source::Nct67Xxd_PECI_1_CAL, 0},
					     {Source::Nct67Xxd_VIRTUAL_TEMP, 0}});
					break;
				default:
					temperaturesSource_.insert(
					    temperaturesSource_.end(),
					    {{Source::Nct67Xxd_PECI_0, 0x027_u16, 0_u16, -1, 0x621_u16},
					     {Source::CPUTIN, 0x073_u16, 0x074_u16, 7, 0x100_u16, 0x491_u16},
					     {Source::SYSTIN, 0x075_u16, 0x076_u16, 7, 0x200_u16, 0x490_u16},
					     {Source::Nct67Xxd_AUXTIN0, 0x077_u16, 0x078_u16, 7, 0x300_u16, 0x492_u16},
					     {Source::Nct67Xxd_AUXTIN1, 0x079_u16, 0x07A_u16, 7, 0x800_u16, 0x493_u16},
					     {Source::Nct67Xxd_AUXTIN2, 0x07B_u16, 0x07C_u16, 7, 0x900_u16, 0x494_u16},
					     {Source::Nct67Xxd_AUXTIN3, 0x150_u16, 0x151_u16, 7, 0x622_u16, 0x495_u16}});
					break;
			}
			break;
		}
		case Chip::NCT610XD: {
			fanRpmRegister_.resize(3);
			for (std::size_t i = 0; i < fanRpmRegister_.size(); i++) {
				fanRpmRegister_[i] = static_cast<u16>(0x030 + (i << 1));
			}

			// min value RPM value with 13-bit fan counter
			minFanRpm_ = static_cast<int>(1.35e6 / 0x1FFF);

			voltageRegisters_ = {0x300, 0x301, 0x302, 0x303, 0x304, 0x305, 0x307, 0x308, 0x309};
			voltageVBatRegister_ = 0x308;
			temperaturesSource_ = {
			    {Source::Nct610X_PECI_0, 0x027_u16, 0, -1, 0x621_u16},
			    {Source::SYSTIN, 0x018_u16, 0x01B_u16, 7, 0x100_u16, 0x018_u16},
			    {Source::CPUTIN, 0x019_u16, 0x11B_u16, 7, 0x200_u16, 0x019_u16},
			    {Source::AUXTIN, 0x01A_u16, 0x21B_u16, 7, 0x300_u16, 0x01A_u16}};
			break;
		}
		case Chip::NCT6683D:
		case Chip::NCT6687D: {
			// CPU
			// System
			// MOS
			// PCH
			// CPU Socket
			// PCIE_1
			// M2_1
			temperaturesSource_ = {
			    {Source::None, 0x100}, {Source::None, 0x102}, {Source::None, 0x104}, {Source::None, 0x106},
			    {Source::None, 0x108}, {Source::None, 0x10A}, {Source::None, 0x10C},
			};

			// VIN0 +12V
			// VIN1 +5V
			// VIN2 VCore
			// VIN3 SIO
			// VIN4 DRAM
			// VIN5 CPU IO
			// VIN6 CPU SA
			// VIN7 SIO
			// 3VCC I/O +3.3
			// SIO VTT
			// SIO VREF
			// SIO VSB
			// SIO AVSB
			// SIO VBAT
			voltageRegisters_ = {0x120, 0x122, 0x124, 0x126, 0x128, 0x12A, 0x12C,
			                     0x12E, 0x130, 0x13A, 0x13E, 0x136, 0x138, 0x13C};

			// CPU Fan
			// PUMP Fan
			// SYS Fan 1
			// SYS Fan 2
			// SYS Fan 3
			// SYS Fan 4
			// SYS Fan 5
			// SYS Fan 6
			fanRpmRegister_ = {0x140, 0x142, 0x144, 0x146, 0x148, 0x14A, 0x14C, 0x14E};

			restoreDefaultFanControlRequired_.assign(fanRpmRegister_.size(), false);
			initialFanControlMode_.assign(fanRpmRegister_.size(), 0);
			initialFanPwmCommand_.assign(fanRpmRegister_.size(), 0);

			// initialize
			u16 initRegister = 0x180;
			u8 data = port_.readByte(initRegister);
			if ((data & 0x80) == 0) {
				port_.writeByte(initRegister, static_cast<u8>(data | 0x80));
			}

			// enable SIO voltage
			port_.writeByte(0x1BB, 0x61);
			port_.writeByte(0x1BC, 0x62);
			port_.writeByte(0x1BD, 0x63);
			port_.writeByte(0x1BE, 0x64);
			port_.writeByte(0x1BF, 0x65);

			break;
		}
		default: break;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::readSIO(
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

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::writeSIO(
    SensorType type, std::size_t channel, double value)
{
	switch (type) {
		case SensorType::pwm: return writePWM(channel, value);
		default: break;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::writePWM(std::size_t channel, double value)
{
	if (std::isnan(value)) {
		restoreDefaultFanPwmControl(channel);
	} else {
		saveDefaultFanPwmControl(channel);

		if (chip() != Chip::NCT6687D && chip() != Chip::NCT6683D) {
			// set manual mode
			port_.writeByte(fanControlModeReg_[channel], 0);

			// set output value
			port_.writeByte(fanPWMCommandReg_[channel], static_cast<u8>(value));
		} else {
			// Manual mode, bit(1 : set, 0 : unset)
			// bit 0 : CPU Fan
			// bit 1 : PUMP Fan
			// bit 2 : SYS Fan 1
			// bit 3 : SYS Fan 2
			// bit 4 : SYS Fan 3
			// bit 5 : SYS Fan 4
			// bit 6 : SYS Fan 5
			// bit 7 : SYS Fan 6

			u8 mode = port_.readByte(fanControlModeReg_[channel]);
			u8 bitMask = static_cast<u8>(0x01 << channel);
			mode |= bitMask;
			port_.writeByte(fanControlModeReg_[channel], mode);

			port_.writeByte(fanPWMRequestReg_[channel], 0x80);
			std::this_thread::sleep_for(50ms);

			port_.writeByte(fanPWMCommandReg_[channel], static_cast<u8>(value));
			port_.writeByte(fanPWMRequestReg_[channel], 0x40);
			std::this_thread::sleep_for(50ms);
		}
	}
}

bool wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::disableIOSpaceLock()
{
	const auto chip = this->chip();
	if (chip != Chip::NCT6791D && chip != Chip::NCT6792D && chip != Chip::NCT6792DA && chip != Chip::NCT6793D &&
	    chip != Chip::NCT6795D && chip != Chip::NCT6796D && chip != Chip::NCT6796DR && chip != Chip::NCT6797D &&
	    chip != Chip::NCT6798D) {
		return true;
	}

	// the lock is disabled already if the vendor ID can be read
	if (isNuvotonVendor())
		return true;


	WinbondNuvotonFintekEnterExit guard{lpcPort_};
	nuvotonDisableIOSpaceLock(lpcPort_);
	return true;
}

bool wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::beginRead()
{
	return base::beginRead() && disableIOSpaceLock();
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::readVoltages(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		if (chip() != Chip::NCT6687D) {
			float value = 0.008f * port_.readByte(voltageRegisters_[i]);
			bool valid = value > 0;

			// check if battery voltage monitor is enabled
			if (valid && voltageRegisters_[i] == voltageVBatRegister_)
				valid = (port_.readByte(vBatMonitorControlRegister_) & 0x01) > 0;

			values[i - channelMin] = valid ? value : std::numeric_limits<float>::quiet_NaN();
		} else {
			float value = 0.001f * static_cast<float>(
			                           16 * port_.readByte(voltageRegisters_[i]) +
			                           (port_.readByte(static_cast<u16>(voltageRegisters_[i] + 1)) >> 4));

			const auto finalValue = [](std::size_t channel, float v) {
				switch (channel) {
					case 0: // 12V
						return v * 12.f;
					case 1: // 5V
						return v * 5.f;
					case 4: // DRAM
						return v * 2.f;
					default: return v;
				}
			};
			values[i - channelMin] = finalValue(i, value);
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::readTemperatures(
    std::size_t channelMin, std::size_t count, double* values) const
{
	long temperatureSourceMask = 0;
	for (auto i = channelMin; i < channelMin + count; i++) {
		const auto& ts = temperaturesSource_[i];
		switch (chip()) {
			case Chip::NCT6687D:
			case Chip::NCT6683D: {
				int value = static_cast<s8>(port_.readByte(ts.reg));
				int half = (port_.readByte(static_cast<u16>((ts.reg + 1) >> 7))) & 0x1;
				float temperature = static_cast<float>(value) + (0.5f * static_cast<float>(half));
				values[i - channelMin] = temperature;
				break;
			}
			case Chip::NCT6796D:
			case Chip::NCT6796DR:
			case Chip::NCT6797D:
			case Chip::NCT6798D: {
				if (ts.reg == 0) {
					spdlog::debug("Temperature register {0} skipped, address 0.", i);
					continue;
				}

				int value = static_cast<s8>(port_.readByte(ts.reg)) << 1;
				spdlog::debug("Temperature register {0} at 0x{1:3X} value (integer): {2}/2", i, ts.reg, value);
				if (ts.halfBit > 0) {
					value |= (port_.readByte(ts.halfReg) >> ts.halfBit) & 0x1;
					spdlog::debug(
					    "Temperature register {0} value updated from 0x{1:3X} (fractional): {2}/2", i, ts.halfReg,
					    value);
				}

				Source source;
				if (ts.sourceReg > 0) {
					source = static_cast<Source>(port_.readByte(ts.sourceReg) & 0x1F);
					spdlog::debug(
					    "Temperature register {0} source at 0x{1:3X}: {2} ({2:x})", i, ts.sourceReg,
					    utility::to_underlying(source));
				} else {
					source = ts.source;
					spdlog::debug(
					    "Temperature register {0} source register is 0, source set to: {1} ({1:x})", i,
					    utility::to_underlying(source));
				}

				// Skip reading when already filled, because later values are without fractional
				if ((temperatureSourceMask & (1L << utility::to_underlying(source))) > 0) {
					spdlog::debug("Temperature register {0} discarded, because source seen before.", i);
					continue;
				}

				float temperature = 0.5f * static_cast<float>(value);
				spdlog::debug("Temperature register {0} final temperature: {1}.", i, temperature);
				if (temperature > 125 || temperature < -55) {
					temperature = std::numeric_limits<float>::quiet_NaN();
					spdlog::debug("Temperature register {0} discarded: Out of range.", i);
				} else {
					temperatureSourceMask |= 1L << utility::to_underlying(source);
					spdlog::debug("Temperature register {0} accepted.", i);
				}

				for (std::size_t j = 0; j < temperaturesSource_.size(); j++) {
					if (temperaturesSource_[j].source == source) {
						values[i - channelMin] = temperature;
						spdlog::debug(
						    "Temperature register {0}, value from source {1} ({1:x}), written at position {2}.", i,
						    utility::to_underlying(temperaturesSource_[j].source), j);
					}
				}
				break;
			}
			default: {
				int value = static_cast<s8>(port_.readByte(ts.reg)) << 1;
				if (ts.halfBit > 0) {
					value |= (port_.readByte(ts.halfReg) >> ts.halfBit) & 0x1;
				}

				Source source = static_cast<Source>(port_.readByte(ts.sourceReg));
				temperatureSourceMask |= 1L << utility::to_underlying(source);

				float temperature = 0.5f * static_cast<float>(value);
				if (temperature > 125 || temperature < -55)
					temperature = std::numeric_limits<float>::quiet_NaN();

				for (std::size_t j = 0; j < temperaturesSource_.size(); j++) {
					if (temperaturesSource_[j].source == source) {
						values[i - channelMin] = temperature;
					}
				}
				break;
			}
		}
	}

	for (auto i = channelMin; i < channelMin + count; i++) {
		const TempSrcDef& ts = temperaturesSource_[i];
		if (!ts.alternateReg.has_value()) {
			spdlog::debug(
			    "Alternate temperature register for temperature {0}, {1} ({1:x}), skipped, because address is null.", i,
			    utility::to_underlying(ts.source));
			continue;
		}

		if ((temperatureSourceMask & (1L << utility::to_underlying(ts.source))) > 0) {
			spdlog::debug(
			    "Alternate temperature register for temperature {0}, {1} ({1:x}), at 0x{2:3X} skipped, because value "
			    "already set.",
			    i, utility::to_underlying(ts.source), ts.alternateReg.value());
			continue;
		}

		float temperature = static_cast<s8>(port_.readByte(ts.alternateReg.value()));
		spdlog::debug(
		    "Alternate temperature register for temperature {0}, {1} ({1:x}), at 0x{2:3x} final temperature: {3}.", i,
		    utility::to_underlying(ts.source), ts.alternateReg.value(), temperature);

		if (temperature > 125 || temperature <= 0) {
			temperature = std::numeric_limits<float>::quiet_NaN();
			;
			spdlog::debug(
			    "Alternate Temperature register for temperature {0}, {1} ({1:x}), discarded: Out of range.", i,
			    utility::to_underlying(ts.source));
		}

		values[i - channelMin] = temperature;
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::readFans(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		if (chip() != Chip::NCT6687D && chip() != Chip::NCT6683D) {
			if (fanCountRegister_.size()) {
				u8 high = port_.readByte(fanCountRegister_[i]);
				u8 low = port_.readByte(static_cast<u16>(fanCountRegister_[i] + 1));

				int cnt = (high << 5) | (low & 0x1F);
				if (cnt < maxFanCount_) {
					values[i - channelMin] = cnt >= minFanCount_ ? 1.35e6f / static_cast<float>(cnt) :
                                                                   std::numeric_limits<float>::quiet_NaN();
				} else {
					values[i - channelMin] = 0;
				}
			} else {
				auto value = port_.readWord(fanRpmRegister_[i]);
				values[i - channelMin] = value > minFanRpm_ ? value : 0;
			}
		} else {
			auto value = port_.readWord(fanRpmRegister_[i]);
			values[i - channelMin] = value;
		}
	}
}

void wm_sensors::hardware::motherboard::lpc::superio::Nct67xx::readPWMs(
    std::size_t channelMin, std::size_t count, double* values) const
{
	for (auto i = channelMin; i < channelMin + count; i++) {
		if (chip() != Chip::NCT6687D && chip() != Chip::NCT6683D) {
			auto value = port_.readByte(fanPWMOutReg_[i]);
			values[i - channelMin] = value / 2.55f;
		} else {
			auto value = port_.readByte(fanPWMOutReg_[i]);
			values[i - channelMin] = std::round(value / 2.55f);
		}
	}
}
