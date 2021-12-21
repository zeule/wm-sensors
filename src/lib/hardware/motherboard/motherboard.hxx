// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_HXX
#define WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_HXX

#include "../../impl/chip_registrator.hxx"
#include "./identification.hxx"

namespace wm_sensors::hardware {
	class SMBios;
}

namespace wm_sensors::hardware::motherboard {

	class Motherboard: public impl::ChipProbe {
		using base = impl::ChipProbe;

	private:
		static std::string boardName(const SMBios& smbios);

		// Inherited via ChipProbe
		bool probe(SensorChipTreeNode& sensorsTree) override;
	};
} // namespace wm_sensors::hardware::motherboard

#endif
