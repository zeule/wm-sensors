// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_LOAD_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_LOAD_HXX

#include "./cpuid.hxx"
#include "../../utility/macro.hxx"

namespace wm_sensors::hardware::cpu {
	class CpuLoad {
	public:
		CpuLoad(const std::vector<std::vector<CPUIDData>>& cpuid);

		bool available() const
		{
			return isAvailable_;
		}

		void update();

		float totalLoad() const
		{
			return totalLoad_;
		}

		float coreLoad(std::size_t core) const
		{
			return coreLoads_[core];
		}

	private:
		static bool getTimes(std::vector<s64>& idle, std::vector<s64>& total);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(CpuLoad)

		const std::vector<std::vector<CPUIDData>>& cpuid_;
		std::vector<float> coreLoads_;
		std::vector<s64> idleTimes_;
		std::vector<s64> totalTimes_;
		float totalLoad_;
		bool isAvailable_;
	};
}

#endif
