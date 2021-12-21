// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_HARDWARE_PSU_CORSAIR_PSU_HXX
#define WM_SENSORS_HARDWARE_PSU_CORSAIR_PSU_HXX

#include "../../../sensor.hxx"

#include <memory>
#include <optional>

namespace hidapi {
	class device;
}

namespace wm_sensors::hardware::psu {

	class Corsair final: public SensorChip {
		using base = SensorChip;

	public:
		static std::vector<std::unique_ptr<SensorChip>> probe();

		SensorChip::Config config() const override;

		VisibilityFlags isVisible(SensorType type, u32 attr, std::size_t channel) const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

		~Corsair();

	private:
		Corsair(hidapi::device&& dev);
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Corsair)


		std::optional<double> readTemp(u32 attr, std::size_t channel) const;
		std::optional<double> readFan(u32 attr, std::size_t channel) const;
		std::optional<double> readPower(u32 attr, std::size_t channel) const;
		std::optional<double> readCurrent(u32 attr, std::size_t channel) const;
		std::optional<double> readVoltage(u32 attr, std::size_t channel) const;
		std::optional<double> readDuration(u32 attr, std::size_t channel) const;

		double calcInCurr() const;

		struct Impl;

		std::unique_ptr<Impl> impl_;
	};
} // namespace wm_sensors::hardware::psu

#endif
