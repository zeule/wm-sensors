// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_EC_ASUS_EC_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_EC_ASUS_EC_HXX

#include "../../../../sensor.hxx"
#include "../../../motherboard/identification.hxx"

#include <memory>

namespace wm_sensors::hardware::motherboard::lpc::ec {
	class AsusEC: wm_sensors::SensorChip {
		using base = wm_sensors::SensorChip;
	public:
		AsusEC(motherboard::Model model);
		~AsusEC();

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

		static bool isAvailable(motherboard::Model model);
	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(AsusEC)

		struct Impl;

		std::unique_ptr<Impl> impl_;
	};
}

#endif
