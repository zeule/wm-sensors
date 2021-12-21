// SPDX-License-Identifier: LGPL-3.0+

// Copyright (C) LibreHardwareMonitor contributors

#include "./amd_cpu.hxx"

#include "../../impl/ring0.hxx"

namespace {
	using namespace wm_sensors::stdtypes;

	const u16 AMD_VENDOR_ID = 0x1022;
	const u8 DEVICE_VENDOR_ID_REGISTER = 0;
	const u8 PCI_BASE_DEVICE = 0x18;
	const u8 PCI_BUS = 0;
} // namespace

wm_sensors::hardware::cpu::AMDCPU::AMDCPU(unsigned processorIndex, std::vector<std::vector<CPUIDData>>&& cpuId)
    : base{processorIndex, std::move(cpuId)}
{
}

wm_sensors::u32 wm_sensors::hardware::cpu::AMDCPU::PCIAddress(u8 function, u16 deviceId) const
{
	using wm_sensors::hardware::impl::Ring0;

	// assemble the pci address
	u32 address = Ring0::PCIAddress(PCI_BUS, static_cast<u8>(PCI_BASE_DEVICE + this->index()), function);

	// verify that we have the correct bus, device and function
	u32 deviceVendor;
	if (!Ring0::instance().readPciConfig(address, DEVICE_VENDOR_ID_REGISTER, deviceVendor) ||
	    deviceVendor != static_cast<u32>(deviceId << 16 | AMD_VENDOR_ID)) {
		return Ring0::INVALID_PCI_ADDRESS;
	}

	return address;
}
