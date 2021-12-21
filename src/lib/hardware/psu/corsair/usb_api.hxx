// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_HARDWARE_PSU_CORSAIR_USB_API_HXX
#define WM_SENSORS_HARDWARE_PSU_CORSAIR_USB_API_HXX

#include "../../../utility/hidapi++/hidapi.hxx"
#include "../../../utility/macro.hxx"
#include "../../../wm_sensor_types.hxx"

#include <array>
#include <bitset>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace wm_sensors::hardware::psu::corsair {
	class CorsairUSBDevice {
	public:
		CorsairUSBDevice(hidapi::device&& dev);

		using Reply = std::array<std::uint8_t, 16>;

		struct FirmwareInfo {
			std::string Vendor;
			std::string Product;
		};

		FirmwareInfo fwInfo();

		enum class Command : u8
		{
			SELECT_RAIL = 0x00,      /* expects length 2 */
			RAIL_VOLTS_HCRIT = 0x40, /* the rest of the commands expect length 3 */
			RAIL_VOLTS_LCRIT = 0x44,
			RAIL_AMPS_HCRIT = 0x46,
			TEMP_HCRIT = 0x4F,
			IN_VOLTS = 0x88,
			IN_AMPS = 0x89,
			RAIL_VOLTS = 0x8B,
			RAIL_AMPS = 0x8C,
			TEMP0 = 0x8D,
			TEMP1 = 0x8E,
			FAN_RPM = 0x90,
			RAIL_WATTS = 0x96,
			VEND_STR = 0x99,
			PROD_STR = 0x9A,
			TOTAL_UPTIME = 0xD1,
			UPTIME = 0xD2,
			OCPMODE = 0xD8,
			TOTAL_WATTS = 0xEE,
			INIT = 0xFE
		};

		std::optional<double> value(Command cmd, u8 rail);

		static const std::size_t railCount = 3; // 3v3 + 5v + 12v
		static const std::size_t tempCount = 2;
		static const std::size_t fanCount = 1;

		struct Criticals {
			std::bitset<tempCount> tempSupport;
			std::bitset<railCount> voltageMinSupport;
			std::bitset<railCount> voltageMaxSupport;
			std::bitset<railCount> currentMaxSupport;

			std::array<double, tempCount> tempMax;

			std::array<double, railCount> voltageMin;
			std::array<double, railCount> voltageMax;

			std::array<double, railCount> currentMax;
		};

		Criticals criticals();

		enum OptionalCommands
		{
			InputCurrent = 0
		};

		unsigned optionalCommands();

		class CommunicationError: public std::runtime_error {
		public:
			using std::runtime_error::runtime_error;

			CommunicationError() = delete;
		};

	private:
		bool sendCommand(u8 length, Command cmd, u8 arg, Reply* reply = nullptr);
		bool request(Command cmd, u8 rail, Reply& reply);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(CorsairUSBDevice)

		hidapi::device device_;
		std::mutex mutex_;
	};
} // namespace wm_sensors::hardware::psu::corsair

#endif
