// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_SENSOR_PATH_HXX
#define WM_SENSORS_LIB_SENSOR_PATH_HXX

#include "./source_class.hxx"

#include "wm-sensors_export.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace wm_sensors {

	struct Identifier {
		std::string name;
		HardwareType type; // builds up sensor path (/motherboard/lpc/sio0/temp/0) for sensor example
		BusType bus;
	};
}

#endif
