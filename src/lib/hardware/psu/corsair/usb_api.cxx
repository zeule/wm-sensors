// SPDX-License-Identifier: LGPL-3.0+
// Copyright (C) 2020 Wilken Gottwalt <wilken.gottwalt@posteo.net>

#include "./usb_api.hxx"

#include "../../../utility/utility.hxx"
#include "../../../utility/unaligned.hxx"

#include <cmath>
#include <cstring>

namespace {
	/* some values are SMBus LINEAR11 data which need a conversion */
#if 0
        int linear11ToInt(ushort val, int scale)
        {
            int exp = ((short)val) >> 11;
            int mant = (((short)(val & 0x7ff)) << 5) >> 5;
            int result = mant * scale;

            return (exp >= 0) ? (result << exp) : (result >> -exp);
        }
#endif
	double linear11ToFloat(wm_sensors::u16 val)
	{
		int exp = static_cast<wm_sensors::s16>(val) >> 11;
		int mant = ((static_cast<wm_sensors::s16>(val & 0x7ff)) << 5) >> 5;
		return mant * std::pow(2., exp);
	}
} // namespace

wm_sensors::hardware::psu::corsair::CorsairUSBDevice::CorsairUSBDevice(hidapi::device&& dev) : device_{std::move(dev)}
{
	/*
	 * PSU_CMD_INIT uses swapped length/command and expects 2 parameter bytes, this command
	 * actually generates a reply, but we don't need it
	 */
	sendCommand(static_cast<u8>(Command::INIT), static_cast<Command>(3), 0);
}

wm_sensors::hardware::psu::corsair::CorsairUSBDevice::FirmwareInfo
wm_sensors::hardware::psu::corsair::CorsairUSBDevice::fwInfo()
{
	Reply vendorArr, productArr;
	if (!sendCommand(3, Command::VEND_STR, 0, &vendorArr)) throw CommunicationError("Can't read vendor string");

	if (!sendCommand(3, Command::PROD_STR, 0, &productArr)) throw CommunicationError("Can't read product");

	return {reinterpret_cast<const char*>(vendorArr.data()), reinterpret_cast<const char*>(productArr.data())};
}

bool wm_sensors::hardware::psu::corsair::CorsairUSBDevice::sendCommand(u8 length, Command cmd, u8 arg, Reply* reply)
{
	/*
	 * Corsair protocol for PSUs
	 *
	 * message size = 64 bytes (request and response, little endian)
	 * request:
	 *	[length][command][param0][param1][paramX]...
	 * reply:
	 *	[echo of length][echo of command][data0][data1][dataX]...
	 *
	 *	- commands are byte sized opcodes
	 *	- length is the sum of all bytes of the commands/params
	 *	- the micro-controller of most of these PSUs support concatenation in the request and reply,
	 *	  but it is better to not rely on this (it is also hard to parse)
	 *	- the driver uses raw events to be accessible from userspace (though this is not really
	 *	  supported, it is just there for convenience, may be removed in the future)
	 *	- a reply always start with the length and command in the same order the request used it
	 *	- length of the reply data is specific to the command used
	 *	- some of the commands work on a rail and can be switched to a specific rail (0 = 12v,
	 *	  1 = 5v, 2 = 3.3v)
	 *	- the format of the init command 0xFE is swapped length/command bytes
	 *	- parameter bytes amount and values are specific to the command (rail setting is the only
	 *	  for now that uses non-zero values)
	 *	- there are much more commands, especially for configuring the device, but they are not
	 *	  supported because a wrong command/length can lockup the micro-controller
	 *	- the driver supports debugfs for values not fitting into the hwmon class
	 *	- not every device class (HXi, RMi or AXi) supports all commands
	 *	- it is a pure sensors reading driver (will not support configuring)
	 */

	const int cmdBufferSize = 64;
	// 	const int replySize = 16;

	u8 cmdBuffer[cmdBufferSize];
	::memset(cmdBuffer, 0, sizeof(cmdBuffer));
	cmdBuffer[0] = 0; // report id
	cmdBuffer[1] = length;
	cmdBuffer[2] = utility::to_underlying(cmd);
	cmdBuffer[3] = arg;

	device_.write(cmdBuffer, cmdBufferSize);
	device_.read(cmdBuffer, cmdBufferSize);

	if (reply) { _memccpy(reply->data(), cmdBuffer + 2, 1, reply->size()); }

	return cmdBuffer[1] == length && cmdBuffer[2] == utility::to_underlying(cmd);
}

bool wm_sensors::hardware::psu::corsair::CorsairUSBDevice::request(Command cmd, u8 rail, Reply& reply)
{
	std::lock_guard<std::mutex> lock{mutex_};

	switch (cmd) {
		case Command::RAIL_VOLTS_HCRIT:
		case Command::RAIL_VOLTS_LCRIT:
		case Command::RAIL_AMPS_HCRIT:
		case Command::RAIL_VOLTS:
		case Command::RAIL_AMPS:
		case Command::RAIL_WATTS:
			if (!sendCommand(2, Command::SELECT_RAIL, rail)) { return false; }
			break;
		default: break;
	}

	return sendCommand(3, cmd, 0, &reply);
}

std::optional<double> wm_sensors::hardware::psu::corsair::CorsairUSBDevice::value(Command cmd, u8 rail)
{
	Reply data;
	if (!request(cmd, rail, data)) { return {}; }

	/*
	 * the biggest value here comes from the uptime command and to exceed MAXINT total uptime
	 * needs to be about 68 years, the rest are u16 values and the biggest value coming out of
	 * the LINEAR11 conversion are the watts values which are about 1200 for the strongest psu
	 * supported (HX1200i)
	 */
	int tmp = utility::get_unaligned_le<s32>(data.data()); // static_cast<int>(data[3] << 24) + (data[2] << 16) + (data[1] << 8) +
	                                           // data[0];
	switch (cmd) {
		case Command::RAIL_VOLTS_HCRIT:
		case Command::RAIL_VOLTS_LCRIT:
		case Command::RAIL_AMPS_HCRIT:
		case Command::TEMP_HCRIT:
		case Command::IN_VOLTS:
		case Command::IN_AMPS:
		case Command::RAIL_VOLTS:
		case Command::RAIL_AMPS:
		case Command::TEMP0:
		case Command::TEMP1:
		case Command::FAN_RPM:
		case Command::RAIL_WATTS:
		case Command::TOTAL_WATTS: return linear11ToFloat(static_cast<u16>(tmp)); // Linear11ToInt((ushort)tmp, 1000000);
		case Command::TOTAL_UPTIME:
		case Command::UPTIME:
		case Command::OCPMODE: return tmp;
		default: return {};
	}
}

wm_sensors::hardware::psu::corsair::CorsairUSBDevice::Criticals
wm_sensors::hardware::psu::corsair::CorsairUSBDevice::criticals()
{
	Criticals res;
	std::optional<double> tmp;
	std::size_t rail;

	for (rail = 0; rail < tempCount; ++rail) {
		tmp = value(Command::TEMP_HCRIT, static_cast<u8>(rail));
		res.tempMax[rail] = tmp.value_or(0.f);
		if (tmp.has_value()) { res.tempSupport.set(rail); }
	}

	for (rail = 0; rail < railCount; ++rail) {
		tmp = value(Command::RAIL_VOLTS_HCRIT, static_cast<u8>(rail));
		res.voltageMax[rail] = tmp.value_or(0.f);
		if (tmp.has_value()) { res.voltageMaxSupport.set(rail); }

		tmp = value(Command::RAIL_VOLTS_LCRIT, static_cast<u8>(rail));
		res.voltageMin[rail] = tmp.value_or(0.f);
		if (tmp.has_value()) { res.voltageMinSupport.set(rail); }

		tmp = value(Command::RAIL_AMPS_HCRIT, static_cast<u8>(rail));
		res.currentMax[rail] = tmp.value_or(0.f);
		if (tmp.has_value()) { res.currentMaxSupport.set(rail); }
	}

	return res;
}

unsigned wm_sensors::hardware::psu::corsair::CorsairUSBDevice::optionalCommands()
{
	unsigned res = 0;
	if (value(Command::IN_AMPS, 0).has_value()) {
		res |= 1L << OptionalCommands::InputCurrent;
	}

	return res;
}
