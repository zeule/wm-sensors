// SPDX-License-Identifier: LGPL-3.0+

#include "./super_io_sensor_chip.hxx"

#include "../../../../utility/utility.hxx"
#include "../../../impl/ring0.hxx"
#include "./super_io_channel_config.hxx"

#include <algorithm>

namespace {
	using namespace wm_sensors;

	const u32 sensorAttributes[] = {
	    0, // chip
	    attributes::temp_input | attributes::temp_label,
	    attributes::in_input | attributes::in_label,
	    attributes::curr_input | attributes::curr_label,
	    attributes::power_input | attributes::power_label,
	    attributes::energy_input | attributes::energy_label,
	    attributes::humidity_input | attributes::humidity_label,
	    attributes::fan_input | attributes::fan_label,
	    attributes::pwm_input | attributes::pwm_label,
	    0, // intrusion,
		0, // data
		0, // data rate
	    attributes::duration_input | attributes::duration_label,
		0, // frequency
		0, // flow
	    0, // load
		0, // raw
		0, // fraction
	};

	static_assert(wm_sensors::utility::array_size(sensorAttributes) == static_cast<std::size_t>(SensorType::max));
} // namespace

wm_sensors::hardware::motherboard::lpc::ChannelConfig::ChannelConfig(std::string lbl, std::size_t src, bool hide)
    : label{std::move(lbl)}
    , sourceIndex{src}
    , hidden{hide}
{
}

wm_sensors::hardware::motherboard::lpc::VoltageChannelConfig::VoltageChannelConfig(
    std::string lbl, std::size_t src, float pRi, float pRf, float pVf, bool hide)
    : ChannelConfig{std::move(lbl), src, hide}
    , ri{pRi}
    , rf{pRf}
    , vf{pVf}
{
}

wm_sensors::hardware::motherboard::lpc::VoltageChannelConfig::VoltageChannelConfig(
    std::string lbl, std::size_t src, bool hide)
    : VoltageChannelConfig(std::move(lbl), src, .0f, 1.f, .0f, hide)
{
}

wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::~SuperIOSensorChip() = default;

wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::SuperIOSensorChip(
    MotherboardId board, lpc::Chip chip, u16 address, const std::map<SensorType, std::size_t>& nrChannels)
    : base{{std::string(chip_name(chip)), "sio", BusType::ISA}}
    , config_{superIOConfiguration(board, chip, nrChannels)}
    , chip_{chip}
    , address_{address}
{
	validateConfig();

	for (std::size_t i = 0; i < static_cast<std::size_t>(SensorType::max); ++i) {
		auto it = nrChannels.find(static_cast<SensorType>(i));
		nrChannels_[i] = it != nrChannels.end() ? it->second : 0;
	}
}

wm_sensors::hardware::motherboard::lpc::Chip wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::chip() const
{
	return chip_;
}

std::optional<wm_sensors::u8> wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::readGPIO(u8 /*index*/)
{
	return {};
}

void wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::writeGPIO(u8 /*index*/, wm_sensors::u8 /*value*/) {}

wm_sensors::SensorChip::Config wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::config() const
{
	SensorChip::Config res;
	if (config_.fan.size()) {
		res.sensors[SensorType::fan].channelAttributes.assign(
		    config_.fan.size(), sensorAttributes[utility::to_underlying(SensorType::fan)]);
	}
	if (config_.temperature.size()) {
		res.sensors[SensorType::temp].channelAttributes.assign(
		    config_.temperature.size(), sensorAttributes[utility::to_underlying(SensorType::temp)]);
	}
	if (config_.voltage.size()) {
		res.sensors[SensorType::voltage].channelAttributes.assign(
		    config_.voltage.size(), sensorAttributes[utility::to_underlying(SensorType::voltage)]);
	}
	if (config_.pwm.size()) {
		res.sensors[SensorType::pwm].channelAttributes.assign(
		    config_.pwm.size(), sensorAttributes[utility::to_underlying(SensorType::pwm)]);
	}

	return res;
}

int wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::read(
    SensorType type, u32 attr, std::size_t channel, double& val) const
{
	switch (type) {
		case SensorType::temp:
			if (attr == attributes::temp_input) {
				readSIO(type, config_.temperature[channel].sourceIndex, 1, &val);
				return 0;
			}
			break;
		case SensorType::fan:
			if (attr == attributes::fan_input) {
				readSIO(type, config_.fan[channel].sourceIndex, 1, &val);
				return 0;
			}
			break;
		case SensorType::in:
			if (attr == attributes::in_input) {
				double v;
				const auto& cc = config_.voltage[channel];
				readSIO(type, cc.sourceIndex, 1, &v);
				// Voltage = value + (value - Vf) * Ri / Rf.
				val = v + (v - cc.vf) * cc.ri / cc.rf;
				return 0;
			}
			break;
		case SensorType::pwm:
			if (attr == attributes::pwm_input) {
				readSIO(type, config_.pwm[channel].sourceIndex, 1, &val);
				return 0;
			}
		default: break;
	}
	return -EOPNOTSUPP;
}

int wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	switch (type) {
		case SensorType::temp:
			if (attr == attributes::temp_label) {
				str = config_.temperature[channel].label;
				return 0;
			}
			break;
		case SensorType::fan:
			if (attr == attributes::fan_label) {
				str = config_.fan[channel].label;
				return 0;
			}
			break;
		case SensorType::in:
			if (attr == attributes::in_label) {
				str = config_.voltage[channel].label;
				return 0;
			}
			break;
		case SensorType::pwm:
			if (attr == attributes::pwm_label) {
				str = config_.pwm[channel].label;
				return 0;
			}
		default: break;
	}
	return -EOPNOTSUPP;
}

int wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::write(
    SensorType type, u32 attr, std::size_t channel, double val)
{
	if (type == SensorType::pwm && attr == attributes::pwm_input) {
		writeSIO(type, channel, val);
		return 0;
	}
	return -EOPNOTSUPP;
}

bool wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::beginRead()
{
	return impl::Ring0::instance().acquireMutex(impl::GlobalMutex::ISABus, std::chrono::milliseconds(10));
}

void wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::endRead()
{
	impl::Ring0::instance().releaseMutex(impl::GlobalMutex::ISABus);
}

bool wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::beginWrite()
{
	return beginRead();
}

void wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::endWrite()
{
	return endRead();
}

void wm_sensors::hardware::motherboard::lpc::SuperIOSensorChip::validateConfig(){
	const auto findMaxChannel = [](const auto& configs) {
		return std::max_element(configs.begin(), configs.end(), [](const auto& l, const auto& r) {
			return l.sourceIndex < r.sourceIndex;
		})->sourceIndex;
	};
	
	if (findMaxChannel(config_.fan) > nrChannels_[utility::to_underlying(SensorType::fan)]) {
		throw std::runtime_error("Super I/O chip config invalid for fans");
	}

	if (findMaxChannel(config_.temperature) > nrChannels_[utility::to_underlying(SensorType::temp)]) {
		throw std::runtime_error("Super I/O chip config invalid for temperatures");
	}

	if (findMaxChannel(config_.pwm) > nrChannels_[utility::to_underlying(SensorType::pwm)]) {
		throw std::runtime_error("Super I/O chip config invalid for PWMs");
	}

	if (findMaxChannel(config_.voltage) > nrChannels_[utility::to_underlying(SensorType::in)]) {
		throw std::runtime_error("Super I/O chip config invalid for voltages");
	}
}
