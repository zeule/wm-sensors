// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_AMD_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_AMD_CPU_HXX

#include "../generic_cpu.hxx"

namespace wm_sensors::hardware::cpu {
	class AMDCPU: public GenericCPU {
		using base = GenericCPU;

	public:
		AMDCPU(unsigned processorIndex, std::vector<std::vector<CPUIDData>>&& cpuId);

	protected:
		u32 PCIAddress(u8 function, u16 deviceId) const;

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(AMDCPU)
	};
} // namespace wm_sensors::hardware::cpu

#endif
