// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_INTEL_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_INTEL_CPU_HXX

#include "../generic_cpu.hxx"

#include <chrono>
#include <optional>

namespace wm_sensors::hardware::cpu {
	class IntelCPU: public GenericCPU {
		using base = GenericCPU;

	public:
		IntelCPU(unsigned processorIndex, std::vector<std::vector<CPUIDData>>&& cpuId);

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

#if 0
		override string GetReport()
		{
			StringBuilder r = new StringBuilder();
			r.Append(base.GetReport());
			r.Append("MicroArchitecture: ");
			r.AppendLine(_microArchitecture.ToString());
			r.Append("Time Stamp Counter Multiplier: ");
			r.AppendLine(_timeStampCounterMultiplier.ToString(CultureInfo.InvariantCulture));
			r.AppendLine();
			return r.ToString();
		}
#endif

	private:
		enum class MicroArchitecture
		{
			Airmont,
			AlderLake,
			Atom,
			Broadwell,
			CannonLake,
			CometLake,
			Core,
			Goldmont,
			GoldmontPlus,
			Haswell,
			IceLake,
			IvyBridge,
			JasperLake,
			KabyLake,
			Nehalem,
			NetBurst,
			RocketLake,
			SandyBridge,
			Silvermont,
			Skylake,
			TigerLake,
			Tremont,
			Unknown
		};

		void update() const;

		Config::ChannelCounts baseChannels_;

		std::vector<float> tjsFromMSR();

		std::vector<std::string> temperatureLabels_;
		std::vector<std::string> frequencyLabels_;
		std::vector<std::string> powerLabels_;

		mutable std::optional<double> busClock_;
		mutable std::vector<double> coreClocks_;
		struct CoreTempData {
			double tjMax;
			double slope;
			double value;
			double deltaT;

			void update(double newDeltaT);
		};
		mutable std::vector<CoreTempData> coreTemperatures_;
		mutable std::optional<double> coreMaxTemperature_;
		mutable std::optional<double> coreAvgTemperature_;

		float energyUnitMultiplier_;
		mutable std::vector<unsigned> lastEnergyConsumed_;
		mutable std::vector<std::chrono::steady_clock::time_point> lastEnergyTime_;
		mutable MicroArchitecture microArchitecture_;
		mutable std::optional<CoreTempData> packageTemperature_;
		mutable std::vector<float> powerSensors_;
		double timeStampCounterMultiplier_;
		mutable std::chrono::steady_clock::time_point lastUpdate_;

		DELETE_COPY_CTOR_AND_ASSIGNMENT(IntelCPU)
	};
} // namespace wm_sensors::hardware::cpu

#endif
