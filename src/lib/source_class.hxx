// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_SOURCE_CLASS_HXX
#define WM_SENSORS_LIB_SOURCE_CLASS_HXX

#include <string>

#include "wm-sensors_export.h"

namespace wm_sensors {
	enum class BusType: short
	{
		Any = -1,
		I2C,
		ISA,
		PCI,
		SPI,
		Virtual,
		ACPI,
		HID,
		MDIO,
		SCSI,
	};
/*
	enum class HardwareType
	{
		CPU,
		GPU,
		Motherboard,
		Controller,
		Storage,
		PSU
	};
	*/
	using HardwareType = std::string_view;

	// common hardware types
	namespace hardwareTypes {
		inline const HardwareType cpu{"cpu"};
		inline const HardwareType gpu{"gpu"};
		inline const HardwareType motherboard{"motherboard"};
		inline const HardwareType storage{"storage"};
		inline const HardwareType psu{"psu"};
	}


} // namespace wm_sensors

#endif
