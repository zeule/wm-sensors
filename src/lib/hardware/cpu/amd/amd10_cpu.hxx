// SPDX-License-Identifier: LGPL-3.0+
//
// Copyright (C) LibreHardwareMonitor and Contributors.
// Partial Copyright (C) Michael Möller <mmoeller@openhardwaremonitor.org> and Contributors.

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_AMD10_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_AMD10_CPU_HXX

#include "./amd_cpu.hxx"

namespace wm_sensors::hardware::cpu {

	class Amd10Cpu final: public AMDCPU {
		using base = AMDCPU;

	public:
		Amd10Cpu(unsigned processorIndex, CpuIdDataArray&& cpuId);

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

	private:
		double estimateTimeStampCounterMultiplier(double timeWindow);
		double estimateTimeStampCounterMultiplier();


#if 0
		override uint[] GetMsrs()
		{
			return new[]{PERF_CTL_0, PERF_CTR_0, HWCR, P_STATE_0, COFVID_STATUS};
		}

	public
		override string GetReport()
		{
			StringBuilder r = new StringBuilder();
			r.Append(base.GetReport());
			r.Append("Miscellaneous Control Address: 0x");
			r.AppendLine(_miscellaneousControlAddress.ToString("X", CultureInfo.InvariantCulture));
			r.Append("Time Stamp Counter Multiplier: ");
			r.AppendLine(_timeStampCounterMultiplier.ToString(CultureInfo.InvariantCulture));
			if (_family == 0x14) {
				Ring0.ReadPciConfig(
				    _miscellaneousControlAddress, CLOCK_POWER_TIMING_CONTROL_0_REGISTER, out uint value);
				r.Append("PCI Register D18F3xD4: ");
				r.AppendLine(value.ToString("X8", CultureInfo.InvariantCulture));
			}

			r.AppendLine();
			return r.ToString();
		}
#endif

		double coreMultiplier(u32 cofVidEax) const;

	private:
		void update() const;
		static bool readSMURegister(u32 address, u32& value);
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Amd10Cpu)

		Config::ChannelCounts baseChannels_;

		mutable double busClock_;
		mutable std::vector<double> coreClock_;
		mutable std::vector<double> cStatesResidency_;
		// AMD family 1Xh processors support only one temperature sensor
		mutable double coreTemperature_;
		mutable double coreVoltage_;
		mutable double northbridgeVoltage_;
		mutable std::chrono::steady_clock::time_point lastUpdate_;

		u8 cStatesIoOffset_;
		bool isSvi2_;
		bool hasSmuTemperatureRegister_;
		u32 miscellaneousControlAddress_;
		double timeStampCounterMultiplier_;
	};
} // namespace wm_sensors::hardware::cpu

#endif
