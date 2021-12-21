// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_AMD0F_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_AMD0F_CPU_HXX

#include "./amd_cpu.hxx"

namespace wm_sensors::hardware::cpu {

	class Amd0FCpu final: public AMDCPU {
		using base = AMDCPU;


	public:
		Amd0FCpu(unsigned processorIndex, CpuIdDataArray&& cpuId);

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

	protected:
		/* uint[] GetMsrs()
		{
		    return new[]{FIDVID_STATUS};
		}*/

#if 0
	override string GetReport()
	{
		StringBuilder r = new StringBuilder();
		r.Append(base.GetReport());
		r.Append("Miscellaneous Control Address: 0x");
		r.AppendLine(_miscellaneousControlAddress.ToString("X", CultureInfo.InvariantCulture));
		r.AppendLine();
		return r.ToString();
	}
#endif

	private:
		void update() const;
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Amd0FCpu)

		Config::ChannelCounts baseChannels_;
		unsigned miscellaneousControlAddress_;
		u8 thermSenseCoreSelCPU0_;
		u8 thermSenseCoreSelCPU1_;
		float temperatureOffset_;

		mutable double busClock_;
		mutable std::vector<double> coreClocks_;
		mutable std::vector<double> coreTemperatures_;
		mutable std::chrono::steady_clock::time_point lastUpdate_;
	};
} // namespace wm_sensors::hardware::cpu

#endif
