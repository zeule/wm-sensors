// SPDX-License-Identifier: LGPL-3.0+

#include "./p7_h1.hxx"

#include "../../impl/usbhid_chip.hxx"
#include "../../../utility/hidapi++/hidapi.hxx"

#include <array>
#include <chrono>

using namespace std::chrono_literals;

namespace {
	constexpr const std::size_t fanCount = 5;
	const auto updateTimeout = 1s;
} // namespace

std::vector<std::unique_ptr<wm_sensors::SensorChip>> wm_sensors::hardware::controller::aerocool::P7H1::probe()
{
	return impl::enumerate([](const auto& di){
		int hubno = di.product_id - 0x1000;
		if ((di.path.find("mi_02") != std::string::npos) && (hubno >= 1) && (hubno <= 8)) {
			return std::unique_ptr<SensorChip>(new P7H1(hidapi::hid::instance().open(di)));
		}
		return std::unique_ptr<SensorChip>();
	}, 0x2e97);
}

struct wm_sensors::hardware::controller::aerocool::P7H1::Impl {
	Impl(hidapi::device&& dev)
	    : device{std::move(dev)}
	    , lastUpdate{std::chrono::steady_clock::now() - 2 * updateTimeout}
	{
	}

	void read();


	std::array<u16, fanCount> readings;
	hidapi::device device;
	std::chrono::steady_clock::time_point lastUpdate;
};

void wm_sensors::hardware::controller::aerocool::P7H1::Impl::read()
{
	const unsigned char reportId = 0;
	std::array<unsigned char, fanCount * 3 + 1> buf;
	buf[0] = reportId;
	if (device.read(buf.data(), buf.size()) == buf.size() && buf[0] == reportId) {
		for (std::size_t i = 0; i < readings.size(); ++i) {
			readings[i] = static_cast<u16>((buf[i * 3 + 2] << 8) + buf[i * 3 + 3]); // TODO unuligned_get
		}
		lastUpdate = std::chrono::steady_clock::now();
	}
}


wm_sensors::hardware::controller::aerocool::P7H1::P7H1(hidapi::device&& dev)
    : base{{"P7H1", "controller", BusType::HID}}
    , impl_{std::make_unique<Impl>(std::move(dev))}
{
}

wm_sensors::hardware::controller::aerocool::P7H1::~P7H1() = default;

wm_sensors::SensorChip::Config wm_sensors::hardware::controller::aerocool::P7H1::config() const
{
	using namespace attributes;

	return {{
	    {SensorType::fan, {std::vector<u32>(fanCount, fan_input | fan_label)}},
	}};
}

int wm_sensors::hardware::controller::aerocool::P7H1::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	if (type == SensorType::fan && attr == attributes::fan_input) {
		if (std::chrono::steady_clock::now() > impl_->lastUpdate + updateTimeout) {
			impl_->read();
		}

		if (channel < fanCount) {
			val = impl_->readings[channel];
			return 0;
		}
	}

	return -EOPNOTSUPP;
}

int wm_sensors::hardware::controller::aerocool::P7H1::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	static std::array<std::string_view, fanCount> labels = {
		"Fan #1", "Fan #2", "Fan #3", "Fan #4"
	};

	if (type == SensorType::fan && attr == attributes::fan_label && channel < fanCount) {
		str = labels[channel];
		return 0;
	}

	return -EOPNOTSUPP;
}
