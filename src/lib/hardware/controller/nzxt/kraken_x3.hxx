// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CONTROLLER_NZXT_KRAKEN_X3_HXX
#define WM_SENSORS_LIB_HARDWARE_CONTROLLER_NZXT_KRAKEN_X3_HXX

#include "../../../sensor.hxx"

#include <memory>
#include <optional>

namespace hidapi {
	class device;
}

namespace wm_sensors::hardware::controller::nzxt {
	class KrakenX3 final: public SensorChip {
		using base = SensorChip;
	public:
		static std::vector<std::unique_ptr<SensorChip>> probe();

		SensorChip::Config config() const override;

		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

		~KrakenX3();

	private:
		KrakenX3(hidapi::device&& dev);

		KrakenX3(const KrakenX3&) = delete;
		KrakenX3& operator=(const KrakenX3&) = delete;

		struct Impl;

		std::unique_ptr<Impl> impl_;
	};
}

#endif
