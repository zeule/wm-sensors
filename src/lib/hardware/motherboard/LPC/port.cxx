// SPDX-License-Identifier: LGPL-3.0+

#include "./port.hxx"

#include "../../impl/ring0.hxx"

namespace {
	const std::uint8_t configurationControlRegister = 0x02;
	const std::uint8_t deviceSelectRegister = 0x07;
	const std::uint8_t nuvotonHardwareMonitorIOSpaceLock = 0x28;

	auto& ring0()
	{
		return wm_sensors::hardware::impl::Ring0::instance();
	}
} // namespace


wm_sensors::hardware::motherboard::lpc::Port::Port(u16 address)
    : address_{address}
{
}

void wm_sensors::hardware::motherboard::lpc::Port::outByte(u8 portOffset, u8 value) const
{
	ring0().writeIOPOrt(static_cast<u16>(address_ + portOffset), value);
}

wm_sensors::u8 wm_sensors::hardware::motherboard::lpc::Port::inByte(u8 portOffset) const
{
	return ring0().readIOPort(static_cast<u16>(address_ + portOffset));
}

wm_sensors::hardware::motherboard::lpc::SingleBankPort::SingleBankPort(SingleBankAddress address)
	: base{address.address}
	, regs_{address.regs}
{
}

void wm_sensors::hardware::motherboard::lpc::SingleBankPort::writeToRegister(
	IndexDataRegisters regs, u8 registerIndex, u8 value) const
{
	outByte(regs.indexRegOffset, registerIndex);
	outByte(regs.dataRegOffset, value);
}

wm_sensors::u8 wm_sensors::hardware::motherboard::lpc::SingleBankPort::readFromRegister(
	IndexDataRegisters regs, wm_sensors::stdtypes::u8 registerIndex) const
{
	outByte(regs.indexRegOffset, registerIndex);
	return inByte(regs.dataRegOffset);
}

std::uint8_t wm_sensors::hardware::motherboard::lpc::SingleBankPort::readByte(u8 registerIndex) const
{
	return readFromRegister(regs_, registerIndex);
}

void wm_sensors::hardware::motherboard::lpc::SingleBankPort::writeByte(u8 registerIndex, u8 value) const
{
	writeToRegister(regs_, registerIndex, value);
}

std::uint16_t wm_sensors::hardware::motherboard::lpc::SingleBankPort::readWord(u8 registerIndex) const
{
	return utility::word(readByte(registerIndex), readByte(static_cast<u8>(registerIndex + 1)));
}

void wm_sensors::hardware::motherboard::lpc::SingleBankPort::select(u8 logicalDeviceNumber) const
{
	writeByte(deviceSelectRegister, logicalDeviceNumber);
}

wm_sensors::hardware::motherboard::lpc::PortWithBanks::PortWithBanks(
    SingleBankAddress a, IndexDataRegisters bankRegs, u8 bankSelectionRegister)
	: base{a}
	, bankRegs_{bankRegs}
	, bankSelectionRegister_{bankSelectionRegister}
{
}

wm_sensors::hardware::motherboard::lpc::PortWithBanks::PortWithBanks(AddressWithBank a)
	: PortWithBanks{a, a.bankSelectionPorts, a.bankSelectionRegister}
{
}

wm_sensors::hardware::motherboard::lpc::PortWithBanks::PortWithBanks(
	SingleBankAddress address, u8 bankSelectionRegister)
	: PortWithBanks{address, address.regs, bankSelectionRegister}
{
}

void wm_sensors::hardware::motherboard::lpc::PortWithBanks::switchBank(u8 bank) const
{
	writeToRegister(bankRegs_, bankSelectionRegister_, bank);
}

wm_sensors::u8 wm_sensors::hardware::motherboard::lpc::PortWithBanks::readByte(u8 bank, u8 registerIndex) const
{
	switchBank(bank);
	return base::readByte(registerIndex);
}

wm_sensors::u16 wm_sensors::hardware::motherboard::lpc::PortWithBanks::readWord(u8 bank, u8 registerIndex) const
{
	return utility::word(readByte(bank, registerIndex), readByte(bank, static_cast<u8>(registerIndex + 1)));
}

void wm_sensors::hardware::motherboard::lpc::PortWithBanks::writeByte(u8 bank, u8 registerIndex, u8 value) const
{
	switchBank(bank);
	base::writeByte(registerIndex, value);
}

wm_sensors::hardware::motherboard::lpc::PortGuard::PortGuard(const SingleBankPort& port)
	: port_{port}
{
}

wm_sensors::hardware::motherboard::lpc::WinbondNuvotonFintekEnterExit::WinbondNuvotonFintekEnterExit(const SingleBankPort& port)
    : PortGuard{port}
{
	port_.outByte(port_.regs().indexRegOffset, 0x87);
	port_.outByte(port_.regs().indexRegOffset, 0x87);
}

wm_sensors::hardware::motherboard::lpc::WinbondNuvotonFintekEnterExit::~WinbondNuvotonFintekEnterExit()
{
	port_.outByte(port_.regs().indexRegOffset, 0xaa);
}

wm_sensors::hardware::motherboard::lpc::IT87EnterExit::IT87EnterExit(const SingleBankPort& port)
    : PortGuard{port}
{

	port_.outByte(port_.regs().indexRegOffset, 0x87);
	port_.outByte(port_.regs().indexRegOffset, 0x01);
	port_.outByte(port_.regs().indexRegOffset, 0x55);
	port_.outByte(port_.regs().indexRegOffset, static_cast<u8>(port_.regs().indexRegOffset == 0x4e ? 0xaa : 0x55));
}

wm_sensors::hardware::motherboard::lpc::IT87EnterExit::~IT87EnterExit()
{
	// Do not exit config mode for secondary super IO.
	if (port_.regs().indexRegOffset != 0x4E) { port_.writeByte(configurationControlRegister, 0x02); }
}

wm_sensors::hardware::motherboard::lpc::SmscEnterExit::SmscEnterExit(const SingleBankPort& port)
	: PortGuard{port}
{
	 port_.outByte(port_.regs().indexRegOffset, 0x55);
}

wm_sensors::hardware::motherboard::lpc::SmscEnterExit::~SmscEnterExit()
{
	port_.outByte(port_.regs().indexRegOffset, 0xaa);
}

void wm_sensors::hardware::motherboard::lpc::nuvotonDisableIOSpaceLock(const SingleBankPort& port)
{
	auto options = port.readByte(nuvotonHardwareMonitorIOSpaceLock);
	// if the i/o space lock is enabled
	if ((options & 0x10) > 0) {
		// disable the i/o space lock
		port.writeByte(nuvotonHardwareMonitorIOSpaceLock, static_cast<u8>(options & ~0x10));
	}
}
