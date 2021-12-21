// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_LIBSENSORS_CHIP_DATA_HXX
#define WM_SENSORS_LIB_IMPL_LIBSENSORS_CHIP_DATA_HXX

#include "../../sensor.hxx" // wm_sensors sensor API
#include "../../sensors.h" // libsensors API

namespace wm_sensors::impl::libsensors {
	class ChipData {
	public:
		ChipData(const SensorChip* chip);
		~ChipData();

		ChipData(ChipData&&) noexcept = default;
		ChipData& operator=(ChipData&&) = default;

		const SensorChip& chip() const
		{
			return *chip_;
		}

		const SensorChip::Config& config() const
		{
			return config_;
		}

		const std::vector<sensors_feature>& features() const
		{
			return features_;
		}

		const std::vector<sensors_subfeature>& subfeatures() const
		{
			return subfeatures_;
		}

		const std::vector<u32>& subfeatureAttributes() const
		{
			return subfeatureAttributes_;
		}

		char* label(const sensors_feature* feature) const;
		int read(std::size_t subfeatureNr, double& value) const;
		const sensors_subfeature* subfeature(const sensors_feature* feature, sensors_subfeature_type type) const;
	private:
		ChipData(const ChipData&) = delete;
		ChipData& operator=(const ChipData&) = delete;

		void expandAttributes(
		    SensorType type, u32 attributes, std::vector<std::pair<u32, sensors_subfeature_type>>& subfeatureValues);

		const SensorChip* chip_;
		SensorChip::Config config_;
		std::vector<sensors_feature> features_;
		std::vector<std::pair<SensorType, std::size_t>> featureChannels_;
		std::vector<sensors_subfeature> subfeatures_;
		std::vector<u32> subfeatureAttributes_;
	};
} // namespace wm_sensors::impl::libsensors

#endif
