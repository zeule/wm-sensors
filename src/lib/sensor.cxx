// SPDX-License-Identifier: LGPL-3.0+

#include "./sensor.hxx"

#include "./utility/utility.hxx"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

	// statically allocated strings for nameless channels
	constexpr const std::size_t numberOfDefaultLabels = 10;
	std::string_view defaultChannelLabels[][numberOfDefaultLabels] = {
	    {"chip0", "chip1", "chip2", "chip3", "chip4", "chip5", "chip6", "chip7", "chip8", "chip9"}, // chip
	    {"temp0", "temp1", "temp2", "temp3", "temp4", "temp5", "temp6", "temp7", "temp8", "temp9"}, // temp
	    {"in0", "in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8", "in9"},                     // in or voltage
	    {"curr0", "curr1", "curr2", "curr3", "curr4", "curr5", "curr6", "curr7", "curr8", "curr9"}, // curr,
	    {"power0", "power1", "power2", "power3", "power4", "power5", "power6", "power7", "power8", "power9"}, // power,
	    {"energy0", "energy1", "energy2", "energy3", "energy4", "energy5", "energy6", "energy7", "energy8",
	     "energy9"}, // energy,
	    {"humidity0", "humidity01", "humidity02", "humidity03", "humidity04", "humidity05", "humidity06", "humidity07",
	     "humidity08", "humidity09"},                                                     // humidity,
	    {"fan0", "fan1", "fan2", "fan3", "fan4", "fan5", "fan6", "fan7", "fan8", "fan9"}, // fan,
	    {"pwm0", "pwm1", "pwm2", "pwm3", "pwm4", "pwm5", "pwm6", "pwm7", "pwm8", "pwm9"}, // pwm,
	    {"intrusion0", "intrusion1", "intrusion2", "intrusion3", "intrusion4", "intrusion5", "intrusion6", "intrusion7",
	     "intrusion8", "intrusion9"}, // intrusion,
	    {"data0", "data1", "data2", "data3", "data4", "data5", "data6", "data7", "data8", "data9"},
	    {"datarate0"},
	    {"duration0", "duration1", "duration2", "duration3", "duration4", "duration5", "duration6", "duration7",
	     "duration8", "duration9"},                                                                 // duration
	    {"freq0", "freq1", "freq2", "freq3", "freq4", "freq5", "freq6", "freq7", "freq8", "freq9"}, // frequency, // Hz
	    {"flow0", "flow1", "flow2", "flow3", "flow4", "flow5", "flow6", "flow7", "flow8", "flow9"}, // flow
	    {"load0", "load1", "load2", "load3", "load4", "load5", "load6", "load7", "load8", "load9"}, // load,
	    {"raw0", "raw1", "raw2", "raw3", "raw4", "raw5", "raw6", "raw7", "raw8", "raw9"}, // raw, // Raw number
	    {"fraction0", "fraction1", "fraction2", "fraction3", "fraction4", "fraction5", "fraction6", "fraction7",
	     "fraction8", "fraction9"}, // fraction
	};

	static_assert(
	    wm_sensors::utility::array_size(defaultChannelLabels) ==
	    wm_sensors::utility::to_underlying(wm_sensors::SensorType::max));
} // namespace

wm_sensors::SensorChip::SensorChip(Identifier id)
    : identifier_{std::move(id)}
{
}

wm_sensors::SensorChip::~SensorChip() = default;

wm_sensors::SensorChip::VisibilityFlags wm_sensors::SensorChip::isVisible(SensorType /*type*/, u32 /*attr*/, std::size_t /*channel*/) const
{
	return SensorVisibility::Readable; // TODO implement in  subclasses
}

int wm_sensors::SensorChip::read(SensorType /*type*/, u32 /*attr*/, std::size_t /*channel*/, double& /*val*/) const
{
	return -EOPNOTSUPP;
}

int wm_sensors::SensorChip::read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	if (attr == attributes::generic_label && channel < numberOfDefaultLabels) {
		str = defaultChannelLabels[utility::to_underlying(type)][channel];
		return 0;
	}
	return -EOPNOTSUPP;
}

int wm_sensors::SensorChip::write(SensorType /*type*/, u32 /*attr*/, std::size_t /*channel*/, double /*val*/)
{
	return -EOPNOTSUPP;
}

const wm_sensors::Identifier& wm_sensors::SensorChip::identifier() const
{
	return identifier_;
}

std::string_view wm_sensors::SensorChip::channelLabel(SensorType type, std::size_t channel) const
{
	std::string_view result;
	int status = this->read(type, attributes::generic_label, channel, result);
	if (status) {
		throw std::runtime_error("No default label found for channel");
	}
	return result;
}

std::ostream& wm_sensors::operator<<(std::ostream& os, SensorType t)
{
	switch (t) {
		case SensorType::chip: os << "chip"; break;
		case SensorType::temp: os << "temp"; break;
		case SensorType::in: os << "in"; break;
		case SensorType::curr: os << "curr"; break;
		case SensorType::power: os << "power"; break;
		case SensorType::energy: os << "energy"; break;
		case SensorType::humidity: os << "humidity"; break;
		case SensorType::fan: os << "fan"; break;
		case SensorType::pwm: os << "pwm"; break;
		case SensorType::intrusion: os << "intrusion"; break;
		case SensorType::data: os << "data"; break;
		case SensorType::dataRate: os << "datarate"; break;
		case SensorType::duration: os << "duration"; break;
		case SensorType::frequency: os << "frequency"; break;
		case SensorType::flow: os << "flow"; break;
		case SensorType::load: os << "load"; break;
		case SensorType::raw: os << "raw"; break;
		case SensorType::fraction: os << "frac"; break;
		case SensorType::max: os << "max"; break;
	}
	return os;
}

wm_sensors::SensorChip::Config&
wm_sensors::SensorChip::Config::appendChannels(SensorType type, std::size_t count, u32 attributes)
{
	auto& dst = sensors[type].channelAttributes;
	for (std::size_t i = 0; i < count; ++i) {
		dst.push_back(attributes);
	}
	return *this;
}

wm_sensors::SensorChip::Config& wm_sensors::SensorChip::Config::append(const Config& other)
{
	for (const auto& s : other.sensors) {
		auto& ca = this->sensors[s.first].channelAttributes;
		std::copy(s.second.channelAttributes.begin(), s.second.channelAttributes.end(), std::back_inserter(ca));
	}
	return *this;
}

wm_sensors::SensorChip::Config::ChannelCounts wm_sensors::SensorChip::Config::nrChannels() const
{
	ChannelCounts res{0};
	for (u8 i = 0; i < sensor_type_max; ++i) {
		const auto it = sensors.find(static_cast<SensorType>(i));
		res[i] = it != sensors.end() ? it->second.channelAttributes.size() : 0;
	}
	return res;
}

bool wm_sensors::SensorChip::Config::isInRange(
    const ChannelCounts& counts, SensorType type, std::size_t channel, std::size_t* shiftedChannel)
{
	std::size_t baseChannelCount = counts[utility::to_underlying(type)];
	if (shiftedChannel) {
		*shiftedChannel = channel - baseChannelCount;
	}
	return channel >= baseChannelCount;
}
