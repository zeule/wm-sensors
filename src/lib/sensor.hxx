// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_SENSOR_HXX
#define WM_SENSORS_LIB_SENSOR_HXX

#include "./sensor_path.hxx"
#include "./utility/enum_bitset.hxx"
#include "./utility/macro.hxx"
#include "./wm_sensor_types.hxx"

#include <sigslot/signal.hpp>

#include <array>
#include <iosfwd>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "wm-sensors_export.h"

namespace wm_sensors {

	class WM_SENSORS_EXPORT SensorChip {
	public:
		virtual ~SensorChip();

		using VisibilityFlags = utility::enum_bitset<SensorVisibility>;

		virtual VisibilityFlags isVisible(SensorType type, u32 attr, std::size_t channel) const;
		virtual int read(SensorType type, u32 attr, std::size_t channel, double& val) const;
		virtual int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const;
		virtual int write(SensorType type, u32 attr, std::size_t channel, double val);

		struct TypeConfig {
			std::vector<u32> channelAttributes;
		};

		struct Config {
			std::map<SensorType, TypeConfig> sensors;

			Config& appendChannels(SensorType type, std::size_t count, u32 attributes);
			Config& append(const Config& other);

			using ChannelCounts = std::array<std::size_t, sensor_type_max>;
			ChannelCounts nrChannels() const;

			static bool
			isInRange(const ChannelCounts& counts, SensorType type, std::size_t channel, std::size_t* shiftedChannel);
		};

		virtual Config config() const = 0;

		const Identifier& identifier() const;

		std::string_view channelLabel(SensorType type, std::size_t channel) const;

		sigslot::signal<void(const SensorChip& chip, SensorType type)> sensorAdded;
		sigslot::signal<void(const SensorChip& chip, SensorType type)> sensorRemoved;

	protected:
		SensorChip(Identifier id);

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(SensorChip)

		Identifier identifier_;
	};

	std::ostream& operator<<(std::ostream& os, SensorType t);

} // namespace wm_sensors

#endif
