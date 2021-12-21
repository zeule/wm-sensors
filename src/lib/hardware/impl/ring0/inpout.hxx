// SPDX-License-Identifier: GPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_RING0_INPOUT_HXX
#define WM_SENSORS_LIB_IMPL_RING0_INPOUT_HXX

#include "./kernel_driver.hxx"

namespace wm_sensors::hardware::impl {
	class InpOut {
	public:
		InpOut();
		~InpOut();

		void* mapPhysycalMemory(void* physAddr, std::size_t physSize, HANDLE& physicalMemoryHandle);
		bool unmapPhysicalMemory(HANDLE physicalMemoryHandle, void* linAddr);

	private:
		InpOut(const InpOut&) = delete;
		InpOut& operator=(const InpOut&) = delete;

		KernelDriver driver_;
	};
} // namespace wm_sensors::hardware::impl
#endif
