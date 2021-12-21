#include "./sensors_data_model.hxx"

#include <cmath>
#include <limits>

wsensors::SensorValueFloat::SensorValueFloat(std::size_t channel)
    : channel_{channel}
{
	reset();
}

double wsensors::SensorValueFloat::stDev() const
{
	return std::sqrt((sum2_ - (sum_ * sum_) / static_cast<double>(count_)) / static_cast<double>(count_ - 1));
}

void wsensors::SensorValueFloat::update(const wm_sensors::SensorChip& chip, wm_sensors::SensorType type)
{
	double val;
	if (chip.read(type, wm_sensors::attributes::generic_input, channel_, val) == 0) {
		update(val);
	}
}

void wsensors::SensorValueFloat::update(double newValue)
{
	if (count_ == 0) {
		shift_ = newValue;
	}

	if (newValue < min_) {
		min_ = newValue;
	}
	if (newValue > max_) {
		max_ = newValue;
	}
	count_ += 1;
	unshiftesSum_ += newValue;
	sum_ += newValue - shift_;
	sum2_ += (newValue - shift_) * (newValue - shift_);
	current_ = newValue;
}

void wsensors::SensorValueFloat::reset()
{
	count_ = 0;
	current_ = std::numeric_limits<decltype(current_)>::quiet_NaN();
	min_ = std::numeric_limits<double>::max();
	max_ = -std::numeric_limits<double>::max();
	sum_ = sum2_ = unshiftesSum_=  0.;
}

wsensors::ChipData::ChipData(const wm_sensors::SensorChip& chip)
    : chip_{chip}
{
}

wsensors::ChipData wsensors::ChipData::fromSensorChip(const wm_sensors::SensorChip& chip)
{
	ChipData res{chip};
	const auto cfg = chip.config();
	for (const auto& s: cfg.sensors) {
		for (std::size_t i = 0; i < s.second.channelAttributes.size(); ++i) {
			if (chip.isVisible(s.first, wm_sensors::attributes::generic_input, i).none()) {
				continue;
			}
			if (s.second.channelAttributes[i] & wm_sensors::attributes::generic_input) {
				res.sensorValues_[s.first].emplace_back(i);
			}
		}
	}
	return res;
}

void wsensors::ChipData::update()
{
	for (auto& sv: sensorValues_) {
		for (auto& cd: sv.second) {
			cd.update(chip_, sv.first);
		}
	}
}

void wsensors::ChipData::reset()
{
	for (auto& sv: sensorValues_) {
		for (auto& cd: sv.second) {
			cd.reset();
		}
	}
}
