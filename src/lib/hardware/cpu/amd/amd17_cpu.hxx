// SPDX-License-Identifier: LGPL-3.0+

// Copyright (C) LibreHardwareMonitor contributors

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_AMD17_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_AMD17_CPU_HXX

#include "./amd_cpu.hxx"
#include "./ryzen_smu.hxx"


namespace wm_sensors::hardware::cpu {

	class Amd17Cpu final: public AMDCPU {
		using base = AMDCPU;

	public:
		Amd17Cpu(unsigned processorIndex, CpuIdDataArray&& cpuId);

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;
#if 0
	protected
		override uint[] GetMsrs()
		{
			return new[]{PERF_CTL_0, PERF_CTR_0, HWCR, MSR_PSTATE_0, COFVID_STATUS};
		}

	public
		override string GetReport()
		{
			StringBuilder r = new ();
			r.Append(base.GetReport());
			r.Append(_smu.GetReport());
			return r.ToString();
		}

#endif

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Amd17Cpu)

		class Impl;
		std::unique_ptr<Impl> impl_;
	};
} // namespace wm_sensors::hardware::cpu

#endif
