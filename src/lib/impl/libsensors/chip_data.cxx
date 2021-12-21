// SPDX-License-Identifier: LGPL-3.0+

// implementation of the chip state for the libsensors API

#include "./chip_data.hxx"

#include "../../utility/string.hxx"
#include "../../utility/utility.hxx"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <stdexcept>

namespace {
	using namespace wm_sensors;
	using namespace wm_sensors::attributes;

	constexpr const sensors_subfeature_type noSubFeature = SENSORS_SUBFEATURE_UNKNOWN;

	const sensors_subfeature_type sensorSubfeaturesTemp[] = {
	    /*temp_enable_bit*/ noSubFeature,
	    /*temp_input_bit*/ SENSORS_SUBFEATURE_TEMP_INPUT,
	    /*temp_label_bit*/ noSubFeature,
	    /*temp_type_bit*/ SENSORS_SUBFEATURE_TEMP_TYPE,
	    /*temp_lcrit_bit*/ SENSORS_SUBFEATURE_TEMP_LCRIT,
	    /*temp_lcrit_hyst_bit*/ SENSORS_SUBFEATURE_TEMP_LCRIT_HYST,
	    /*temp_min_bit*/ SENSORS_SUBFEATURE_TEMP_MIN,
	    /*temp_min_hyst_bit*/ SENSORS_SUBFEATURE_TEMP_MIN_HYST,
	    /*temp_max_bit*/ SENSORS_SUBFEATURE_TEMP_MAX,
	    /*temp_max_hyst_bit*/ SENSORS_SUBFEATURE_TEMP_MAX_HYST,
	    /*temp_crit_bit*/ SENSORS_SUBFEATURE_TEMP_CRIT,
	    /*temp_crit_hyst_bit*/ SENSORS_SUBFEATURE_TEMP_CRIT_HYST,
	    /*temp_emergency_bit*/ SENSORS_SUBFEATURE_TEMP_EMERGENCY,
	    /*temp_emergency_hyst_bit*/ SENSORS_SUBFEATURE_TEMP_EMERGENCY_HYST,
	    /*temp_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_ALARM,
	    /*temp_lcrit_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_LCRIT_ALARM,
	    /*temp_min_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_MIN_ALARM,
	    /*temp_max_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_MAX_ALARM,
	    /*temp_crit_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_CRIT_ALARM,
	    /*temp_emergency_alarm_bit*/ SENSORS_SUBFEATURE_TEMP_EMERGENCY_ALARM,
	    /*temp_fault_bit*/ SENSORS_SUBFEATURE_TEMP_FAULT,
	    /*temp_offset_bit*/ SENSORS_SUBFEATURE_TEMP_OFFSET,
	    /*temp_lowest_bit*/ SENSORS_SUBFEATURE_TEMP_LOWEST,
	    /*temp_highest_bit*/ SENSORS_SUBFEATURE_TEMP_HIGHEST,
	    /*temp_reset_history*/ noSubFeature,
	    /*temp_rated_min*/ noSubFeature,
	    /*temp_rated_max*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesTemp) == temp_bit_max);

	const sensors_subfeature_type sensorSubfeaturesIn[] = {
	    /*in_enable_bit*/ noSubFeature,
	    /*in_input_bit*/ SENSORS_SUBFEATURE_IN_INPUT,
	    /*in_label_bit*/ noSubFeature,
	    /*in_min_bit*/ SENSORS_SUBFEATURE_IN_MIN,
	    /*in_max_bit*/ SENSORS_SUBFEATURE_IN_MAX,
	    /*in_lcrit_bit*/ SENSORS_SUBFEATURE_IN_LCRIT,
	    /*in_crit_bit*/ SENSORS_SUBFEATURE_IN_CRIT,
	    /*in_average_bit*/ SENSORS_SUBFEATURE_IN_AVERAGE,
	    /*in_lowest_bit*/ SENSORS_SUBFEATURE_IN_LOWEST,
	    /*in_highest_bit*/ SENSORS_SUBFEATURE_IN_HIGHEST,
	    /*in_reset_history_bit*/ noSubFeature,
	    /*in_alarm_bit*/ SENSORS_SUBFEATURE_IN_ALARM,
	    /*in_min_alarm_bit*/ SENSORS_SUBFEATURE_IN_MIN_ALARM,
	    /*in_max_alarm_bit*/ SENSORS_SUBFEATURE_IN_MAX_ALARM,
	    /*in_lcrit_alarm_bit*/ SENSORS_SUBFEATURE_IN_LCRIT_ALARM,
	    /*in_crit_alarm_bit*/ SENSORS_SUBFEATURE_IN_CRIT_ALARM,
	    /*in_rated_min_bit*/ noSubFeature,
	    /*in_rated_max_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesIn) == in_bit_max);

	const sensors_subfeature_type sensorSubfeaturesCurr[] = {
	    /*curr_enable_bit*/ noSubFeature,
	    /*curr_input_bit*/ SENSORS_SUBFEATURE_CURR_INPUT,
	    /*curr_label_bit*/ noSubFeature,
	    /*curr_min_bit*/ SENSORS_SUBFEATURE_CURR_MIN,
	    /*curr_max_bit*/ SENSORS_SUBFEATURE_CURR_MAX,
	    /*curr_lcrit_bit*/ SENSORS_SUBFEATURE_CURR_LCRIT,
	    /*curr_crit_bit*/ SENSORS_SUBFEATURE_CURR_CRIT,
	    /*curr_average_bit*/ SENSORS_SUBFEATURE_CURR_AVERAGE,
	    /*curr_lowest_bit*/ SENSORS_SUBFEATURE_CURR_LOWEST,
	    /*curr_highest_bit*/ SENSORS_SUBFEATURE_CURR_HIGHEST,
	    /*curr_reset_history_bit*/ noSubFeature,
	    /*curr_alarm_bit*/ SENSORS_SUBFEATURE_CURR_ALARM,
	    /*curr_min_alarm_bit*/ SENSORS_SUBFEATURE_CURR_MIN_ALARM,
	    /*curr_max_alarm_bit*/ SENSORS_SUBFEATURE_CURR_MAX_ALARM,
	    /*curr_lcrit_alarm_bit*/ SENSORS_SUBFEATURE_CURR_LCRIT_ALARM,
	    /*curr_crit_alarm_bit*/ SENSORS_SUBFEATURE_CURR_CRIT_ALARM,
	    /*curr_rated_min_bit*/ noSubFeature,
	    /*curr_rated_max_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesCurr) == curr_bit_max);

	const sensors_subfeature_type sensorSubfeaturesPower[] = {
	    /*power_enable_bit*/ noSubFeature,
	    /*power_input_bit*/ SENSORS_SUBFEATURE_POWER_INPUT,
	    /*power_label_bit*/ noSubFeature,
	    /*power_average_bit*/ SENSORS_SUBFEATURE_POWER_AVERAGE,
	    /*power_average_interval_bit*/ SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL,
	    /*power_average_interval_max_bit*/ noSubFeature,
	    /*power_average_interval_min_bit*/ noSubFeature,
	    /*power_average_highest_bit*/ SENSORS_SUBFEATURE_POWER_AVERAGE_HIGHEST,
	    /*power_average_lowest_bit*/ SENSORS_SUBFEATURE_POWER_AVERAGE_LOWEST,
	    /*power_average_max_bit*/ noSubFeature,
	    /*power_average_min_bit*/ noSubFeature,
	    /*power_input_highest_bit*/ SENSORS_SUBFEATURE_POWER_INPUT_HIGHEST,
	    /*power_input_lowest_bit*/ SENSORS_SUBFEATURE_POWER_INPUT_LOWEST,
	    /*power_reset_history_bit*/ noSubFeature,
	    /*power_accuracy_bit*/ noSubFeature,
	    /*power_cap_bit*/ SENSORS_SUBFEATURE_POWER_CAP,
	    /*power_cap_hyst_bit*/ SENSORS_SUBFEATURE_POWER_CAP_HYST,
	    /*power_cap_max_bit*/ noSubFeature,
	    /*power_cap_min_bit*/ noSubFeature,
	    /*power_min_bit*/ SENSORS_SUBFEATURE_POWER_MIN,
	    /*power_max_bit*/ SENSORS_SUBFEATURE_POWER_MAX,
	    /*power_crit_bit*/ SENSORS_SUBFEATURE_POWER_CRIT,
	    /*power_lcrit_bit*/ SENSORS_SUBFEATURE_POWER_LCRIT,
	    /*power_alarm_bit*/ SENSORS_SUBFEATURE_POWER_ALARM,
	    /*power_cap_alarm_bit*/ SENSORS_SUBFEATURE_POWER_CAP_ALARM,
	    /*power_min_alarm_bit*/ SENSORS_SUBFEATURE_POWER_MIN_ALARM,
	    /*power_max_alarm_bit*/ SENSORS_SUBFEATURE_POWER_MAX_ALARM,
	    /*power_lcrit_alarm_bit*/ SENSORS_SUBFEATURE_POWER_LCRIT_ALARM,
	    /*power_crit_alarm_bit*/ SENSORS_SUBFEATURE_POWER_CRIT_ALARM,
	    /*power_rated_min_bit*/ noSubFeature,
	    /*power_rated_max_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesPower) == power_bit_max);

	const sensors_subfeature_type sensorSubfeaturesEnergy[] = {
	    /*energy_enable_bit*/ noSubFeature,
	    /*energy_input_bit*/ SENSORS_SUBFEATURE_ENERGY_INPUT,
	    /*energy_label_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesEnergy) == energy_bit_max);

	const sensors_subfeature_type sensorSubfeaturesHumidity[] = {
	    /*humidity_enable_bit*/ noSubFeature,
	    /*humidity_input_bit*/ SENSORS_SUBFEATURE_HUMIDITY_INPUT,
	    /*humidity_label_bit*/ noSubFeature,
	    /*humidity_min_bit*/ noSubFeature,
	    /*humidity_min_hyst_bit*/ noSubFeature,
	    /*humidity_max_bit*/ noSubFeature,
	    /*humidity_max_hyst_bit*/ noSubFeature,
	    /*humidity_alarm_bit*/ noSubFeature,
	    /*humidity_fault_bit*/ noSubFeature,
	    /*humidity_rated_min_bit*/ noSubFeature,
	    /*humidity_rated_max_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesHumidity) == humidity_bit_max);

	const sensors_subfeature_type sensorSubfeaturesFan[] = {
	    /*fan_enable_bit*/ noSubFeature,
	    /*fan_input_bit*/ SENSORS_SUBFEATURE_FAN_INPUT,
	    /*fan_label_bit*/ noSubFeature,
	    /*fan_min_bit*/ SENSORS_SUBFEATURE_FAN_MIN,
	    /*fan_max_bit*/ SENSORS_SUBFEATURE_FAN_MAX,
	    /*fan_div_bit*/ SENSORS_SUBFEATURE_FAN_DIV,
	    /*fan_pulses_bit*/ SENSORS_SUBFEATURE_FAN_PULSES,
	    /*fan_target_bit*/ noSubFeature,
	    /*fan_alarm_bit*/ SENSORS_SUBFEATURE_FAN_ALARM,
	    /*fan_min_alarm_bit*/ SENSORS_SUBFEATURE_FAN_MIN_ALARM,
	    /*fan_max_alarm_bit*/ SENSORS_SUBFEATURE_FAN_MAX_ALARM,
	    /*fan_fault_bit*/ SENSORS_SUBFEATURE_FAN_FAULT,
	};
	static_assert(utility::array_size(sensorSubfeaturesFan) == fan_bit_max);

	// TODO
	const sensors_subfeature_type sensorSubfeaturesPWM[] = {
	    /*pwm_enable_bit*/ noSubFeature,
	    /*pwm_input_bit*/ noSubFeature,
	    /*pwm_label_bit*/ noSubFeature,
	    /*pwm_mode_bit*/ noSubFeature,
	    /*pwm_freq_bit*/ noSubFeature};
	static_assert(utility::array_size(sensorSubfeaturesPWM) == pwm_bit_max);

	const sensors_subfeature_type sensorSubfeaturesIntrusion[] = {
	    /*intrusion_alarm_bit*/ SENSORS_SUBFEATURE_INTRUSION_ALARM,
	    /*intrusion_beep_bit*/ SENSORS_SUBFEATURE_INTRUSION_BEEP};
	static_assert(utility::array_size(sensorSubfeaturesIntrusion) == intrusion_bit_max);

	const sensors_subfeature_type sensorSubfeaturesDuration[] = {
	    /*duration_enable_bit*/ noSubFeature,
	    /*duration_input_bit*/ noSubFeature,
	    /*duration_label_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesDuration) == duration_bit_max);

	const sensors_subfeature_type sensorSubfeaturesFrequency[] = {
	    /*frequency_enable_bit*/ noSubFeature,
	    /*frequency_input_bit*/ noSubFeature,
	    /*frequency_label_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesFrequency) == frequency_bit_max);

	const sensors_subfeature_type sensorSubfeaturesLoad[] = {
	    /*load_enable_bit*/ noSubFeature,
	    /*load_input_bit*/ noSubFeature,
	    /*load_label_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesDuration) == duration_bit_max);

	const sensors_subfeature_type sensorSubfeaturesRaw[] = {
	    /*raw_enable_bit*/ noSubFeature,
	    /*raw_input_bit*/ noSubFeature,
	    /*raw_label_bit*/ noSubFeature,
	};
	static_assert(utility::array_size(sensorSubfeaturesRaw) == raw_bit_max);

	SensorType featureTypeToSensorType(sensors_feature_type t)
	{
		switch (t) {
			case SENSORS_FEATURE_IN: return SensorType::in;
			case SENSORS_FEATURE_FAN: return SensorType::fan;
			case SENSORS_FEATURE_TEMP: return SensorType::temp;
			case SENSORS_FEATURE_POWER: return SensorType::power;
			case SENSORS_FEATURE_ENERGY: return SensorType::energy;
			case SENSORS_FEATURE_CURR: return SensorType::curr;
			case SENSORS_FEATURE_HUMIDITY: return SensorType::humidity;
			case SENSORS_FEATURE_INTRUSION: return SensorType::intrusion;
			default: throw std::runtime_error("Unsupported feature type");
		}
	}

	sensors_feature_type sensorTypeToFeatureType(SensorType t)
	{
		switch (t) {
			case SensorType::temp: return SENSORS_FEATURE_TEMP;
			case SensorType::in: return SENSORS_FEATURE_IN;
			case SensorType::curr: return SENSORS_FEATURE_CURR;
			case SensorType::power: return SENSORS_FEATURE_POWER;
			case SensorType::energy: return SENSORS_FEATURE_ENERGY;
			case SensorType::humidity: return SENSORS_FEATURE_HUMIDITY;
			case SensorType::fan: return SENSORS_FEATURE_FAN;
			case SensorType::pwm: return SENSORS_FEATURE_FAN;
			case SensorType::intrusion: return SENSORS_FEATURE_INTRUSION;
			default: return SENSORS_FEATURE_UNKNOWN;
		}
	}

} // namespace

wm_sensors::impl::libsensors::ChipData::ChipData(const SensorChip* chip)
    : chip_{chip}
    , config_{chip->config()}
{
	for (const auto& s: config_.sensors) {
		const auto channelCount = s.second.channelAttributes.size();
		std::vector<std::pair<u32, sensors_subfeature_type>> subfeatures;
		for (std::size_t i = 0; i < channelCount; ++i) {
			sensors_feature f;
			f.name = strndup(fmt::format("{0}{1}", s.first, i));
			f.number = static_cast<int>(features_.size());
			f.type = sensorTypeToFeatureType(s.first);
			f.first_subfeature = static_cast<int>(subfeatures_.size());
			f.padding1 =static_cast<int>(i);
			features_.push_back(std::move(f));
			featureChannels_.push_back({s.first, i});

			expandAttributes(s.first, s.second.channelAttributes[i], subfeatures);
			for (auto sft: subfeatures) {
				sensors_subfeature sf;
				auto vis = chip->isVisible(s.first, sft.first, i);
				sf.flags = 0;
				if (vis.test(SensorVisibility::Readable)) {
					sf.flags |= SENSORS_MODE_R;
				}
				if (vis.test(SensorVisibility::Writable)) {
					sf.flags |= SENSORS_MODE_W;
				}
				sf.mapping = f.number;
				sf.number = static_cast<int>(subfeatures_.size());
				sf.type = sft.second;
				subfeatures_.push_back(std::move(sf));
				subfeatureAttributes_.push_back(sft.first);
			}
		}
	}
}

wm_sensors::impl::libsensors::ChipData::~ChipData()
{
	for (auto& f: features_) {
		free(f.name);
	}
}

char* wm_sensors::impl::libsensors::ChipData::label(const sensors_feature* feature) const
{
	auto featureNr = utility::to_unsigned(feature - &features_.front());
	return strndup(chip().channelLabel(featureChannels_[featureNr].first, featureChannels_[featureNr].second));
}

int wm_sensors::impl::libsensors::ChipData::read(std::size_t subfeatureNr, double& value) const
{
	const sensors_subfeature& sf = subfeatures_[subfeatureNr];
	const sensors_feature& f = features_[utility::to_unsigned_checked(sf.mapping)];
	return chip().read(featureTypeToSensorType(f.type), 1u << subfeatureAttributes_[subfeatureNr], utility::to_unsigned_checked(f.padding1), value);
}

const sensors_subfeature*
wm_sensors::impl::libsensors::ChipData::subfeature(const sensors_feature* feature, sensors_subfeature_type type) const
{
	for (std::size_t i = utility::to_unsigned_checked(feature->first_subfeature); i < subfeatures_.size(); i++) {
		const sensors_subfeature& sf = subfeatures_[i];
		if (sf.mapping != feature->number) {
			return nullptr;
		}
		if (sf.type == type) {
			return &sf;
		}
	}
	return nullptr;
}

void wm_sensors::impl::libsensors::ChipData::expandAttributes(
    SensorType type, u32 attribute, std::vector<std::pair<u32, sensors_subfeature_type>>& subfeatureValues)
{
	subfeatureValues.clear();
	const auto map = [&subfeatureValues, attribute](int maxBit, const sensors_subfeature_type subfs[]) {
		for (int i = 0; i < maxBit; ++i) {
			if (subfs[i] != noSubFeature && utility::is_bit_set(attribute, i)) {
				subfeatureValues.push_back({i, subfs[i]});
			}
		}
	};
	switch (type) {
		case SensorType::temp: map(utility::to_underlying(temp_bit_max), sensorSubfeaturesTemp); break;
		case SensorType::in: map(utility::to_underlying(in_bit_max), sensorSubfeaturesIn); break;
		case SensorType::curr: map(utility::to_underlying(curr_bit_max), sensorSubfeaturesCurr); break;
		case SensorType::power: map(utility::to_underlying(power_bit_max), sensorSubfeaturesPower); break;
		case SensorType::energy: map(utility::to_underlying(energy_bit_max), sensorSubfeaturesEnergy); break;
		case SensorType::humidity: map(utility::to_underlying(humidity_bit_max), sensorSubfeaturesHumidity); break;
		case SensorType::fan: map(utility::to_underlying(fan_bit_max), sensorSubfeaturesFan); break;
		case SensorType::pwm: map(utility::to_underlying(pwm_bit_max), sensorSubfeaturesPWM); break;
		case SensorType::intrusion: map(utility::to_underlying(intrusion_bit_max), sensorSubfeaturesIntrusion); break;
		case SensorType::duration: map(utility::to_underlying(duration_bit_max), sensorSubfeaturesDuration); break;
		case SensorType::frequency: map(utility::to_underlying(frequency_bit_max), sensorSubfeaturesFrequency); break;
		case SensorType::load: map(utility::to_underlying(load_bit_max), sensorSubfeaturesLoad); break;
		case SensorType::raw: map(utility::to_underlying(raw_bit_max), sensorSubfeaturesRaw); break;
		default: break;
	}
}
