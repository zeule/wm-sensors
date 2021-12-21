// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CONTROLLER_AEROCOOL_P7_H1_HXX
#define WM_SENSORS_LIB_HARDWARE_CONTROLLER_AEROCOOL_P7_H1_HXX

#include "../../../sensor.hxx"

#include <memory>
#include <optional>

namespace hidapi {
	class device;
}

namespace wm_sensors::hardware::controller::aerocool {

	class P7H1 final: public SensorChip {
		using base = SensorChip;
	public:
		static std::vector<std::unique_ptr<SensorChip>> probe();

		SensorChip::Config config() const override;

		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

		~P7H1();

	private:
		P7H1(hidapi::device&& dev);
		P7H1(const P7H1&) = delete;
		P7H1& operator=(const P7H1&) = delete;

		struct Impl;

		std::unique_ptr<Impl> impl_;
	};

} // namespace wm_sensors::hardware::controller::Aerocool

#endif
