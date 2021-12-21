// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_LPC_SUPER_IO_CHANNEL_CONFIG_HXX
#define WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_LPC_SUPER_IO_CHANNEL_CONFIG_HXX

#include "../../../../sensor.hxx"
#include "../identification.hxx"
#include "./super_io_sensor_chip.hxx"

#include <map>

namespace wm_sensors::hardware::motherboard::lpc {
	ChannelsConfiguration
	superIOConfiguration(MotherboardId board, Chip chip, const std::map<SensorType, std::size_t>& nrChannels);
}

#endif
