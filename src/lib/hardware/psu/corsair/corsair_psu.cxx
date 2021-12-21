// SPDX-License-Identifier: LGPL-3.0+
// Copyright (C) 2020 Wilken Gottwalt <wilken.gottwalt@posteo.net>

#include "./corsair_psu.hxx"

#include "./usb_api.hxx"

#include "../../../wm_sensor_types.hxx"
#include "../../impl/usbhid_chip.hxx"

#include <algorithm>
#include <map>

namespace {
	using namespace wm_sensors::stdtypes;

	const u16 vendorId = 0x1b1c;
	const std::map<u16, std::string_view> productIds = {
	    {0x1c03_u16, "HX550i"},  {0x1c04_u16, "HX650i"},  {0x1c05_u16, "HX750i"},  {0x1c06_u16, "HX850i"},
	    {0x1c07_u16, "HX1000i"}, {0x1c08_u16, "HX1200i"}, {0x1c09_u16, "RM550i"},  {0x1c0a_u16, "RM650i"},
	    {0x1c0b_u16, "RM750i"},  {0x1c0c_u16, "RM850i"},  {0x1c0d_u16, "RM1000i"},
	    // 0x1c11, // AX1600i
	};


	int checkedReturn(std::optional<double> v, double& res)
	{
		res = v.value_or(std::numeric_limits<double>::quiet_NaN());
		return v.has_value() ? 0 : -EOPNOTSUPP;
	}

} // namespace

std::vector<std::unique_ptr<wm_sensors::SensorChip>> wm_sensors::hardware::psu::Corsair::probe()
{
	return impl::enumerate(
	    [](const auto& di) {
		    return std::unique_ptr<SensorChip>(
		        productIds.contains(di.product_id) ? new Corsair(hidapi::hid::instance().open(di)) : nullptr);
	    },
	    vendorId);
}

struct wm_sensors::hardware::psu::Corsair::Impl {
	Impl(hidapi::device&& dev);
	DELETE_COPY_CTOR_AND_ASSIGNMENT(Impl)

	corsair::CorsairUSBDevice device;
	corsair::CorsairUSBDevice::Criticals criticals;
	unsigned optionalCommands;
};

wm_sensors::hardware::psu::Corsair::Impl::Impl(hidapi::device&& dev)
    : device{std::move(dev)}
    , criticals{device.criticals()}
    , optionalCommands{device.optionalCommands()}
{
}

wm_sensors::hardware::psu::Corsair::~Corsair() = default;

wm_sensors::hardware::psu::Corsair::Corsair(hidapi::device&& dev)
    : base{{"Corsair " + std::string(productIds.at(dev.info().product_id)), "psu", BusType::HID}}
    , impl_{std::make_unique<Impl>(std::move(dev))}
{
}

wm_sensors::SensorChip::Config wm_sensors::hardware::psu::Corsair::config() const
{
	using namespace attributes;

	return {
	    {{SensorType::temp, {{temp_input | temp_label | temp_crit, temp_input | temp_label | temp_crit}}},
	     {SensorType::in,
	      {{in_input | in_label, in_input | in_label | in_crit | in_lcrit, in_input | in_label | in_crit | in_lcrit,
	        in_input | in_label | in_crit | in_lcrit}}},
	     {SensorType::curr,
	      {{curr_input | curr_label, curr_input | curr_label | curr_crit, curr_input | curr_label | curr_crit,
	        curr_input | curr_label | curr_crit}}},
	     {SensorType::power,
	      {{power_input | power_label, power_input | power_label, power_input | power_label,
	        power_input | power_label}}},
	     {SensorType::fan, {{fan_input | fan_label}}},
	     {SensorType::duration, {{duration_input | duration_label, duration_input | duration_label}}}}};
}

double wm_sensors::hardware::psu::Corsair::calcInCurr() const
{
	std::optional<double> watts = impl_->device.value(corsair::CorsairUSBDevice::Command::TOTAL_WATTS, 0);
	std::optional<double> volts = impl_->device.value(corsair::CorsairUSBDevice::Command::IN_VOLTS, 0);

	/* most of these PSUs have an average efficiency of 92%, so put it in there */
	return watts.value_or(std::numeric_limits<float>::quiet_NaN()) /
	       volts.value_or(std::numeric_limits<float>::quiet_NaN()) / 0.92;
}

int wm_sensors::hardware::psu::Corsair::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	using labels_array = std::vector<std::string_view>;
	static const labels_array labelsTemp{"VRM", "Case"};
	static const labels_array labelsIn{"Input", "+12V", "+5V", "+3.3V"};
	static const labels_array labelsCurr{"Input", "+12V", "+5V", "+3.3V"};
	static const labels_array labelsPower{"Total", "+12V", "+5V", "+3.3V"};
	static const labels_array labelsFan{"PSU"};
	static const labels_array labelsDuration{"Uptime", "Total Uptime"};

	using namespace attributes;

	if (type == SensorType::temp && attr == temp_label && channel < labelsTemp.size()) {
		str = labelsTemp[channel];
		return 0;
	} else if (type == SensorType::fan && attr == fan_label && channel < labelsFan.size()) {
		str = labelsFan[channel];
		return 0;
	} else if (type == SensorType::power && attr == power_label && channel < labelsPower.size()) {
		str = labelsPower[channel];
		return 0;
	} else if (type == SensorType::in && attr == in_label && channel < labelsIn.size()) {
		str = labelsIn[channel];
		return 0;
	} else if (type == SensorType::curr && attr == curr_label && channel < labelsCurr.size()) {
		str = labelsCurr[channel];
		return 0;
	} else if (type == SensorType::duration && attr == curr_label && channel < labelsDuration.size()) {
		str = labelsDuration[channel];
		return 0;
	}
	return -EOPNOTSUPP;
}

wm_sensors::SensorChip::VisibilityFlags
wm_sensors::hardware::psu::Corsair::isVisible(SensorType /*type*/, u32 attr, std::size_t channel) const
{
	VisibilityFlags res = SensorVisibility::Readable;

	switch (attr) {
		case attributes::temp_input:
		case attributes::temp_label:
		case attributes::temp_crit:
			if (channel > 0 && !(impl_->criticals.tempSupport.test(channel - 1))) {
				res.set(SensorVisibility::Readable, false);
			}
			break;
		default: break;
	}

	return res;
}

int wm_sensors::hardware::psu::Corsair::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	switch (type) {
		case SensorType::temp: return checkedReturn(readTemp(attr, channel), val);
		case SensorType::fan: return checkedReturn(readFan(attr, channel), val);
		case SensorType::power: return checkedReturn(readPower(attr, channel), val);
		case SensorType::in: return checkedReturn(readVoltage(attr, channel), val);
		case SensorType::curr: return checkedReturn(readCurrent(attr, channel), val);
		case SensorType::duration: return checkedReturn(readDuration(attr, channel), val);
		default: return -EOPNOTSUPP;
	}
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readFan(u32 attr, std::size_t /*channel*/) const
{
	if (attr == attributes::fan_input) {
		return impl_->device.value(corsair::CorsairUSBDevice::Command::FAN_RPM, 0);
	}
	return {};
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readTemp(u32 attr, std::size_t channel) const
{
	using Command = corsair::CorsairUSBDevice::Command;

	switch (attr) {
		case attributes::temp_input:
			return impl_->device.value(channel ? Command::TEMP1 : Command::TEMP0, static_cast<u8>(channel));
		case attributes::temp_crit: return impl_->criticals.tempMax[channel];
		default: return {};
	}
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readVoltage(u32 attr, std::size_t channel) const
{
	using Command = corsair::CorsairUSBDevice::Command;

	switch (attr) {
		case attributes::in_input:
			switch (channel) {
				case 0: return impl_->device.value(Command::IN_VOLTS, 0);
				case 1:
				case 2:
				case 3: return impl_->device.value(Command::RAIL_VOLTS, static_cast<u8>(channel - 1));
				default: break;
			}
			break;
		case attributes::in_crit: return impl_->criticals.voltageMax[channel - 1];
		case attributes::in_lcrit: return impl_->criticals.voltageMin[channel - 1];
	}

	return {};
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readCurrent(u32 attr, std::size_t channel) const
{
	using OptionalCommands = corsair::CorsairUSBDevice::OptionalCommands;
	using Command = corsair::CorsairUSBDevice::Command;

	switch (attr) {
		case attributes::curr_input:
			switch (channel) {
				case 0:
					if (impl_->optionalCommands & OptionalCommands::InputCurrent)
						return impl_->device.value(Command::IN_AMPS, 0);
					else
						return calcInCurr();
					break;
				case 1:
				case 2:
				case 3: return impl_->device.value(Command::RAIL_AMPS, static_cast<u8>(channel - 1));
				default: break;
			}
			break;
		case attributes::curr_crit: return impl_->criticals.currentMax[channel - 1]; break;
		default: break;
	}

	return {};
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readPower(u32 attr, std::size_t channel) const
{
	using Command = corsair::CorsairUSBDevice::Command;
	if (attr == attributes::power_input) {
		switch (channel) {
			case 0: return impl_->device.value(Command::TOTAL_WATTS, 0);
			case 1:
			case 2:
			case 3: return impl_->device.value(Command::RAIL_WATTS, static_cast<u8>(channel - 1));
			default: break;
		}
	}

	return {};
}

std::optional<double> wm_sensors::hardware::psu::Corsair::readDuration(u32 attr, std::size_t channel) const
{
	using Command = corsair::CorsairUSBDevice::Command;
	if (attr == attributes::duration_input) {
		return impl_->device.value(channel ? Command::TOTAL_UPTIME : Command::UPTIME, 0);
	}
	return {};
}
