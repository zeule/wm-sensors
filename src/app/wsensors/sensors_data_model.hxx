#pragma once

#include <sensor.hxx>
#include <sensor_tree.hxx>

namespace wsensors {
	class SensorValueFloat {
	public:
		SensorValueFloat(std::size_t channel);

		std::size_t channel() const
		{
			return channel_;
		}

		double value() const
		{
			return current_;
		}

		double min() const
		{
			return min_;
		}

		double max() const
		{
			return max_;
		}

		double average() const
		{
			return unshiftesSum_ / static_cast<double>(count_);
		}

		double stDev() const;
		unsigned long long count() const
		{
			return count_;
		}

		void update(const wm_sensors::SensorChip& chip, wm_sensors::SensorType type);
		void update(double newValue);
		void reset();

	private:
		std::size_t channel_; // have to keep channel here because there might be gaps in channel numbers

		double current_;
		double max_;
		double min_;
		unsigned long long count_;
		double unshiftesSum_;
		double sum_;
		double sum2_;
		double shift_;
	};

	class ChipData {
	public:
		static ChipData fromSensorChip(const wm_sensors::SensorChip& chip);
		ChipData(const ChipData&) = default;

		void update();
		void reset();

		const std::map<wm_sensors::SensorType, std::vector<SensorValueFloat>>& values() const
		{
			return sensorValues_;
		}
	private:
		ChipData(const wm_sensors::SensorChip& chip);

		ChipData& operator=(const ChipData&) = delete;
		ChipData& operator=(ChipData&&) = delete;

		const wm_sensors::SensorChip& chip_;
		std::map<wm_sensors::SensorType, std::vector<SensorValueFloat>> sensorValues_;
	};

	using ChipDataTreeNode = wm_sensors::SensorTreeNode<wm_sensors::HardwareType, ChipData>;

} // namespace wsensors
