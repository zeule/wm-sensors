// SPDX-License-Identifier: GPL-3.0+

#include "./winring0.hxx"

#include "./ioctl.hxx"

#include "../../../impl/group_affinity.hxx"
#include "../../../utility/utility.hxx"

#include <algorithm>
#include <filesystem>
#include <limits>

#include "ring0-drv-config.h"


namespace {
	namespace ioctl = wm_sensors::hardware::impl::ioctl;
	
	// const uint INVALID_PCI_ADDRESS = 0xFFFFFFFF;
	const DWORD OLS_TYPE = 40000;

	const auto IOCTL_OLS_GET_REFCOUNT = ioctl::controlCode(OLS_TYPE, 0x801);

	const auto IOCTL_OLS_READ_MSR = ioctl::controlCode(OLS_TYPE, 0x821);
	const auto IOCTL_OLS_WRITE_MSR = ioctl::controlCode(OLS_TYPE, 0x822);
	const auto IOCTL_OLS_READ_IO_PORT_BYTE = ioctl::controlCode(OLS_TYPE, 0x833, ioctl::Access::Read);
	const auto IOCTL_OLS_WRITE_IO_PORT_BYTE = ioctl::controlCode(OLS_TYPE, 0x836, ioctl::Access::Write);
	const auto IOCTL_OLS_READ_PCI_CONFIG = ioctl::controlCode(OLS_TYPE, 0x851, ioctl::Access::Read);
	const auto IOCTL_OLS_WRITE_PCI_CONFIG = ioctl::controlCode(OLS_TYPE, 0x852, ioctl::Access::Write);
	const auto IOCTL_OLS_READ_MEMORY = ioctl::controlCode(OLS_TYPE, 0x841, ioctl::Access::Read);
} // namespace

wm_sensors::hardware::impl::WinRing0::WinRing0()
    : driver_{driverFileName(WINRING_DRIVER_FILE), serviceName(L"R0"), L"WinRing0_1_2_0"}
{
}

wm_sensors::hardware::impl::WinRing0::~WinRing0()
{
	if (driver_.isOpen()) {
		u32 refCount = 0;
		driver_.deviceIOControl(IOCTL_OLS_GET_REFCOUNT, nullptr, 0, &refCount, sizeof(refCount));
		driver_.deleteOnClose(refCount <= 1);
	}
}

bool wm_sensors::hardware::impl::WinRing0::readMSR(u32 index, u32& eax, u32& edx)
{
	if (!driver_.isOpen()) {
		eax = 0;
		edx = 0;
		return false;
	}

	std::uint64_t buffer = 0;
	bool result = driver_.deviceIOControl(IOCTL_OLS_READ_MSR, index, buffer);
	edx = ((buffer >> 32) & 0xFFFFFFFF);
	eax = (buffer & 0xFFFFFFFF);
	return result;
}

bool wm_sensors::hardware::impl::WinRing0::readMSR(
    u32 index, u32& eax, u32& edx, wm_sensors::impl::GroupAffinity affinity)
{
	wm_sensors::impl::ThreadGroupAffinityGuard tga{affinity};
	bool result = readMSR(index, eax, edx);
	return result;
}

bool wm_sensors::hardware::impl::WinRing0::writeMSR(u32 index, u32 eax, u32 edx)
{
	if (!driver_.isOpen()) {
		return false;
	}

	struct WriteMsrInput {
		u32 reg;
		u64 value;
	};

	WriteMsrInput input{index, (static_cast<u64>(edx) << 32) | eax};
	return driver_.deviceIOControl(IOCTL_OLS_WRITE_MSR, input);
}

wm_sensors::u8 wm_sensors::hardware::impl::WinRing0::readIOPort(u32 port)
{
	if (!driver_.isOpen()) {
		return 0;
	}

	u32 value = 0;
	driver_.deviceIOControl(IOCTL_OLS_READ_IO_PORT_BYTE, port, value);
	return (value & 0xFF);
}

void wm_sensors::hardware::impl::WinRing0::writeIOPort(u32 port, u8 value)
{
	if (!driver_.isOpen()) {
		return;
	}

	struct WriteIoPortInput {
		u32 portNumber;
		u8 value;
	};

	WriteIoPortInput input{port, value};
	driver_.deviceIOControl(IOCTL_OLS_WRITE_IO_PORT_BYTE, input);
}

bool wm_sensors::hardware::impl::WinRing0::readPciConfig(u32 pciAddress, u32 regAddress, u32& value)
{
	if (!driver_.isOpen() || (regAddress & 3) != 0) {
		value = 0;
		return false;
	}

	struct ReadPciConfigInput {
		u32 pciAddress;
		u32 regAddress;
	};

	ReadPciConfigInput input{pciAddress, regAddress};

	value = 0;
	return driver_.deviceIOControl(IOCTL_OLS_READ_PCI_CONFIG, input, value);
}

bool wm_sensors::hardware::impl::WinRing0::writePciConfig(u32 pciAddress, u32 regAddress, u32 value)
{
	if (!driver_.isOpen() || (regAddress & 3) != 0) {
		return false;
	}

	struct WritePciConfigInput {
		u32 pciAddress;
		u32 regAddress;
		u32 value;
	};

	WritePciConfigInput input{pciAddress, regAddress, value};
	return driver_.deviceIOControl(IOCTL_OLS_WRITE_PCI_CONFIG, input);
}

#if 0
bool wm_sensors::hardware::impl::WinRing0::readMemory(const void* address, void* buffer, std::size_t size)
{
	if (!driver_.isOpen() || size > std::numeric_limits<u32>::max()) {
		return false;
	}

	struct ReadMemoryInput {
		u64 address;
		u32 unitSize;
		u32 count;
	};

	ReadMemoryInput input{reinterpret_cast<u64>(address), 1, static_cast<u32>(size)};
	return driver_.deviceIOControl(IOCTL_OLS_READ_MEMORY, &input, sizeof(input), buffer, size);
}
#endif
