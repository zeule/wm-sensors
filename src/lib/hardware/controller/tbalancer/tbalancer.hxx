// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CONTROLLER_TBALANCER_HXX
#define WM_SENSORS_LIB_HARDWARE_CONTROLLER_TBALANCER_HXX

#include "../../../sensor.hxx"

#include <memory>
#include <vector>

namespace wm_sensors::hardware::tbalancer {
	class TBalancer final: public SensorChip {
	public:
		static std::vector<std::unique_ptr<SensorChip>> probe();

		SensorChip::Config config() const override;

		int read(SensorType type, u32 attr, int channel, double& val) const override;
		int read(SensorType type, u32 attr, int channel, std::string_view& str) const override;

		~TBalancer();

	private:
		TBalancer(std::size_t portIndex, u8 protocolVersion);

		struct Impl;

		std::unique_ptr<Impl> impl_;
	};
}

#endif
