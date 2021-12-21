// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_SENSOR_COLLECTION_HXX
#define WM_SENSORS_LIB_IMPL_SENSOR_COLLECTION_HXX

#include "../sensor.hxx"

#include <algorithm>
#include <concepts>
#include <limits>
#include <memory>
#include <stdexcept>

namespace wm_sensors::impl {
	class Sensor {
	public:
		Sensor(std::string label)
		    : value_{std::numeric_limits<double>::quiet_NaN()}
		    , label_{std::move(label)}
		{
		}

		double value() const
		{
			return value_;
		}

		void value(double v)
		{
			value_ = v;
		}

		const std::string& label() const
		{
			return label_;
		}

	private:
		mutable double value_;
		std::string label_;
	};

	template <class SensorT>
	requires std::derived_from<SensorT, Sensor>
	class SensorCollection {
	public:

		class Handle {
		public:
			Handle() noexcept
			    : index{static_cast<std::size_t>(-1)}
			    , type{SensorType::max}
			{
			}

			Handle(std::nullptr_t) noexcept
			    : Handle()
			{
			}

			Handle(const Handle&) noexcept = default;
			Handle& operator=(const Handle&) noexcept = default;
			Handle(Handle&&) noexcept = default;
			Handle& operator=(Handle&&) noexcept = default;

			explicit operator bool() const noexcept
			{
				return type != SensorType::max;
			}

		private:
			Handle(std::size_t ind, SensorType t) noexcept
			    : index{ind}
			    , type{t}
			{
			}

			std::size_t index;
			SensorType type;

			friend class SensorCollection;
		};


		SensorCollection(SensorChip::Config::ChannelCounts baseCounts)
		    : baseCounts_{std::move(baseCounts)}
		{
		}

		SensorCollection(const SensorChip::Config& baseConfig)
		    : SensorCollection(baseConfig.nrChannels())
		{
		}

		Handle add(SensorType type, SensorT&& sensor, bool active = false)
		{
			auto& sensorsForType = sensors_[type];
			std::size_t index = sensorsForType.size();
			sensorsForType.push_back({std::move(sensor), active});
			return {index, type};
		}

		template <class... OtherCtorArgs>
		Handle add(std::string label, SensorType type, bool active, OtherCtorArgs... otherCtorArgs)
		{
			return add(type, SensorT{std::move(label), std::forward<OtherCtorArgs...>(otherCtorArgs)...}, active);
		}

		const SensorT& operator[](Handle handle) const
		{
			return sensors_.at(handle.type).at(handle.index).sensor;
		}

		SensorT& operator[](Handle handle)
		{
			return const_cast<SensorT&>(const_cast<const SensorCollection*>(this)->operator[](handle));
		}
#if 0
		void remove(Sensor&& sensor)
		{
			auto& sensorsForType = sensors_[sensor.type()];
			// non-optimal, as at most one record will be removed. TODO
			std::erase_if(sensorsForType, [&sensor](const SensorInfo& info) { return &info.sensor == &sensor; });
		}
#endif

		void setSensorActive(Handle handle, bool isEnabled)
		{
			sensors_[handle.type].at(handle.index).isEnabled = isEnabled;
		}
		void activateSensor(Handle sensor)
		{
			setSensorActive(sensor, true);
		}

		void deactivateSensor(Handle sensor)
		{
			setSensorActive(sensor, false);
		}

		SensorChip::Config::ChannelCounts config(SensorChip::Config& cfg) const
		{
			auto res = cfg.nrChannels();
			for (const auto& ts: sensors_) {
				for (const auto& si: ts.second) {
					if (si.isEnabled) {
						// TODO min/max and hysteresis
						cfg.appendChannels(ts.first, 1, attributes::generic_input | attributes::generic_label);
					}
				}
			}

			return res;
		}

		int read(SensorType type, u32 attr, std::size_t channel, double& val) const
		{
			std::size_t myChannel;

			if (SensorChip::Config::isInRange(baseCounts_, type, channel, &myChannel)) {
				const auto it = sensors_.find(type);
				if (it != sensors_.end() && myChannel < it->second.size()) {
					switch (attr) {
						case attributes::generic_input: val = it->second[myChannel].sensor.value(); return 0;
					}
				}
			}
			return -EOPNOTSUPP;
		}

		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
		{
			std::size_t myChannel;

			if (SensorChip::Config::isInRange(baseCounts_, type, channel, &myChannel)) {
				const auto it = sensors_.find(type);
				if (it != sensors_.end() && myChannel < it->second.size()) {
					switch (attr) {
						case attributes::generic_label: str = it->second[myChannel].sensor.label(); return 0;
					}
				}
			}
			return -EOPNOTSUPP;
		}

		int write(SensorType type, u32 attr, unsigned channel, float val)
		{
			// TODO
			return -EOPNOTSUPP;
		}

	private:
		struct SensorInfo {
			SensorT sensor;
			bool isEnabled;
		};

		SensorChip::Config::ChannelCounts baseCounts_;
		std::map<SensorType, std::vector<SensorInfo>> sensors_;
	};
} // namespace wm_sensors::impl

#endif
