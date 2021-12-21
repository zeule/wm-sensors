// SPDX-License-Identifier: GPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_WINRING0_WINRING0_HXX
#define WM_SENSORS_LIB_IMPL_WINRING0_WINRING0_HXX

#include "./kernel_driver.hxx"
#include "../../../wm_sensor_types.hxx"
#include "../../../impl/group_affinity.hxx"

#include <Windows.h>

#include <cstddef>

namespace wm_sensors::hardware::impl {
	class WinRing0 {
	public:
		WinRing0();
		~WinRing0();

		bool readMSR(u32 index, u32& eax, u32& edx);
		bool readMSR(u32 index, u32& eax, u32& edx, wm_sensors::impl::GroupAffinity affinity);
		bool writeMSR(u32 index, u32 eax, u32 edx);
		u8 readIOPort(u32 port);
		void writeIOPort(u32 port, u8 value);
		u32 getPciAddress(u8 bus, u8 device, u8 function)
		{
			return (u32)(((bus & 0xFF) << 8) | ((device & 0x1F) << 3) | (function & 7));
		}

		bool readPciConfig(u32 pciAddress, u32 regAddress, u32& value);
		bool writePciConfig(u32 pciAddress, u32 regAddress, u32 value);
#if 0
		// The available driver file was compiled without memory read support
		bool readMemory(const void* address, void* buffer, std::size_t size);
#endif

	private:
		WinRing0(const WinRing0&) = delete;
		WinRing0& operator=(const WinRing0&) = delete;

		KernelDriver driver_;
	};
} // namespace wm_sensors::hardware::impl

#endif
