// SPDX-License-Identifier: LGPL-3.0+

#include "./kraken_x3.hxx"

#include "../../../utility/hidapi++/hidapi.hxx"
#include "../../../utility/macro.hxx"
#include "../../../utility/unaligned.hxx"
#include "../../impl/usbhid_chip.hxx"

#include <array>

std::vector<std::unique_ptr<wm_sensors::SensorChip>> wm_sensors::hardware::controller::nzxt::KrakenX3::probe()
{
	return impl::enumerate(
	    [](const auto& di) {
		    return std::unique_ptr<SensorChip>(
		        di.product_id == 0x2007 ? new KrakenX3(hidapi::hid::instance().open(di)) : nullptr);
	    },
	    0x1e71);
}

struct wm_sensors::hardware::controller::nzxt::KrakenX3::Impl: public impl::HidChipImplAutoRead {
	Impl(hidapi::device&& dev)
	    : HidChipImplAutoRead{std::move(dev), std::chrono::milliseconds(500)}
	{
	}

	DELETE_COPY_CTOR_AND_ASSIGNMENT(Impl)

	bool readData() override;

	u16 pumpRPM_;
	float temperature_;
};

bool wm_sensors::hardware::controller::nzxt::KrakenX3::Impl::readData()
{
	std::array<u8, 64> data;
	if (device().read(data.data(), data.size()) == data.size() && data[0] == 0x75 && data[1] == 0x02) {
		temperature_ = data[15] + data[16] / 10.0f;
		pumpRPM_ = utility::get_unaligned_le<u16>(&data[17]); // (data[18] << 8) | data[17];
		return true;
	}
	return false;
}


wm_sensors::hardware::controller::nzxt::KrakenX3::KrakenX3(hidapi::device&& dev)
    : base{{"Kraken X3", "controller", BusType::HID}}
    , impl_{std::make_unique<Impl>(std::move(dev))}
{
}

wm_sensors::hardware::controller::nzxt::KrakenX3::~KrakenX3() = default;

wm_sensors::SensorChip::Config wm_sensors::hardware::controller::nzxt::KrakenX3::config() const
{
	return {{
	    {SensorType::temp, {{attributes::temp_input | attributes::temp_label}}},
	    {SensorType::fan, {{attributes::fan_input | attributes::fan_label}}},
	}};
}

int wm_sensors::hardware::controller::nzxt::KrakenX3::read(
    SensorType type, u32 attr, std::size_t channel, double& val) const
{
	switch (type) {
		case SensorType::temp:
			if (attr == attributes::temp_input && channel == 0) {
				val = impl_->temperature_;
				impl_->acknowledgeDataAccess();
				return 0;
			}
			break;
		case SensorType::fan:
			if (attr == attributes::fan_input && channel == 0) {
				val = impl_->pumpRPM_;
				impl_->acknowledgeDataAccess();
				return 0;
			}
			break;
		default: break;
	}
	return -EOPNOTSUPP;
}

int wm_sensors::hardware::controller::nzxt::KrakenX3::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	static std::array<std::string_view, 2> labels = {"Water", "Pump"};

	switch (type) {
		case SensorType::temp:
			if (attr == attributes::temp_label && channel == 0) {
				str = labels[0];
				return 0;
			}
			break;
		case SensorType::fan:
			if (attr == attributes::fan_label && channel == 0) {
				str = labels[1];
				return 0;
			}
			break;
		default: break;
	}
	return -EOPNOTSUPP;
}
