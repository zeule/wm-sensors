// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPUPROBE_HXX
#define WM_SENSORS_LIB_HARDWARE_CPUPROBE_HXX

#include "../../impl/chip_registrator.hxx"

namespace wm_sensors::hardware::cpu {
	class CPUProbe: public wm_sensors::impl::ChipProbe {
		// Inherited via ChipProbe
		virtual bool probe(SensorChipTreeNode& sensorsTree) override;
	};
}

#endif
