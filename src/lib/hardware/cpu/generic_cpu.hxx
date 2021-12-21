// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_GENERIC_CPU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_GENERIC_CPU_HXX

#include "./cpuid.hxx"

#include "../../sensor.hxx"

#include <chrono>

#include "cpu_load.hxx"

namespace wm_sensors::hardware::cpu {
	class GenericCPU: public SensorChip {
		using base = SensorChip;

	public:
		using CpuIdDataArray = std::vector<std::vector<CPUIDData>>;

		GenericCPU(unsigned processorIndex, CpuIdDataArray&& cpuId);

		const CpuIdDataArray& cpuId() const
		{
			return cpuIdData_;
		}

		bool hasModelSpecificRegisters() const
		{
			return hasModelSpecificRegisters_;
		}

		bool hasTimeStampCounter() const
		{
			return hasTimeStampCounter_;
		}

		/** Gets the CPU index. */
		unsigned index() const
		{
			return index_;
		}

		double timeStampCounterFrequency() const
		{
			return timeStampCounterFrequency_;
		}

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

	protected:
		std::string coreString(std::size_t i) const;
		std::size_t coreCount() const
		{
			return coreCount_;
		}

		const std::vector<std::vector<CPUIDData>>& cpuIdData() const
		{
			return cpuIdData_;
		}

		const CPUIDData& cpu0IdData() const
		{
			return cpuIdData_.front().front();
		}

		u32 family() const
		{
			return family_;
		}

		u32 model() const
		{
			return model_;
		}

		u32 stepping() const
		{
			return stepping_;
		}

		u32 packageType() const
		{
			return packageType_;
		}

		const std::vector<std::string>& coreLabels() const
		{
			return coreLabels_;
		}

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(GenericCPU)

		void estimateTimeStampCounterFrequency(double& frequency, double& error);
		static void estimateTimeStampCounterFrequency(double timeWindow, double& frequency, double& error);
		void updateLoads() const;
		void update() const;
		void maybeUpdateFrequencies() const;

		// virtual uint[] GetMsrs()
		//{
		//	return null;
		//}

		const CpuIdDataArray cpuIdData_;
		std::size_t coreCount_;
		std::size_t logicalCoreCount_;
		const CPUIDData::Vendor vendor_;
		const u32 family_;
		const u32 model_;
		const u32 packageType_;
		const u32 stepping_;
		unsigned index_;
		mutable CpuLoad cpuLoad_;
		const bool hasModelSpecificRegisters_;
		const bool hasTimeStampCounter_;
		const bool isInvariantTimeStampCounter_;

		mutable std::vector<float> coreLoads_;
		mutable std::vector<unsigned long> coreFrequencies_;

		std::vector<std::string> coreLabels_;
		double estimatedTimeStampCounterFrequency_;
		double estimatedTimeStampCounterFrequencyError_;

		mutable s64 lastTime_;
		mutable u64 lastTimeStampCount_;
		mutable double timeStampCounterFrequency_;
		mutable std::chrono::steady_clock::time_point lastUpdateFreq_;
	};
} // namespace wm_sensors::hardware::cpu

#endif
