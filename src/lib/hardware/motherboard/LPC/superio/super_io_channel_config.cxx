// SPDX-License-Identifier: LGPL-3.0+
#include "./super_io_channel_config.hxx"

#include <fmt/format.h>

#include <cassert>
#include <utility>

namespace {
	using namespace wm_sensors::hardware::motherboard;
	using lpc::ChannelsConfiguration;
	using lpc::Chip;
	using wm_sensors::SensorType;

	template <class Map>
	typename Map::mapped_type getOrDefault(const Map& m, typename Map::key_type k, typename Map::mapped_type def)
	{
		const auto i = m.find(k);
		return i == m.end() ? def : i->second;
	}

	void addDefaultEntries(
	    ChannelsConfiguration& cfg, SensorType type, const std::map<SensorType, std::size_t>& nrChannels,
	    std::size_t min = 0, bool hide = false)
	{
		const std::size_t count = getOrDefault(nrChannels, type, 0);
		switch (type) {
			case SensorType::voltage:
				for (auto i = min; i < count; i++) {
					cfg.voltage.emplace_back(fmt::format("Voltage #{0}", i + 1), i, hide);
				}
				break;
			case SensorType::temp:
				for (auto i = min; i < count; i++) {
					cfg.temperature.emplace_back(fmt::format("Temperature #{0}", i + 1), i, hide);
				}
				break;
			case SensorType::fan:
				for (auto i = min; i < count; i++) {
					cfg.fan.emplace_back(fmt::format("Fan #{0}", i + 1), i, hide);
				}
				break;
			case SensorType::pwm:
				for (auto i = min; i < count; i++) {
					cfg.pwm.emplace_back(fmt::format("Fan Control #{0}", i + 1), i, hide);
				}
				break;
			default: throw std::logic_error("Unexpected sensor type");
		}
	}

	ChannelsConfiguration
	ASRockConfiguration(MotherboardId /*board*/, Chip /*chip*/, const std::map<SensorType, std::size_t>& /*nrChannels*/)
	{
		ChannelsConfiguration res;
		res.voltage.emplace_back("Vcore", 0);
		res.voltage.emplace_back("+3.3V", 2);
		res.voltage.emplace_back("+12V", 4, 30.f, 10.f);
		res.voltage.emplace_back("+5V", 5, 6.8f, 10.f);
		res.voltage.emplace_back("VBat", 8);
		res.temperature.emplace_back("CPU", 0);
		res.temperature.emplace_back("Motherboard", 1);
		res.fan.emplace_back("CPU Fan", 0);
		res.fan.emplace_back("Chassis Fan #1", 1);

		// this mutex is also used by the official ASRock tool
#if 0
		// TODO
		mutex = new Mutex(false, "ASRockOCMark");

		bool exclusiveAccess = false;
		try {
			exclusiveAccess = mutex.WaitOne(10, false);
		} catch (AbandonedMutexException) {
		} catch (InvalidOperationException) {
		}

		// only read additional fans if we get exclusive access
		if (exclusiveAccess) {
			res.fan.emplace_back("Chassis Fan #2", 2);
			res.fan.emplace_back("Chassis Fan #3", 3);
			res.fan.emplace_back("Power Fan", 4);

			readFan = index = >
			{
				if (index < 2) {
					return superIO.Fans[index];
				}

				// get GPIO 80-87
				byte ? gpio = superIO.ReadGpio(7);
				if (!gpio.HasValue)
					return null;


				// read the last 3 fans based on GPIO 83-85
				int[] masks = {0x05, 0x03, 0x06};
				return ((gpio.Value >> 3) & 0x07) == masks[index - 2] ? superIO.Fans[2] : null;
			};

			int fanIndex = 0;

			postUpdate = () = >
			{
				// get GPIO 80-87
				byte ? gpio = superIO.ReadGpio(7);
				if (!gpio.HasValue)
					return;


				// prepare the GPIO 83-85 for the next update
				int[] masks = {0x05, 0x03, 0x06};
				superIO.WriteGpio(7, (byte)((gpio.Value & 0xC7) | (masks[fanIndex] << 3));
				fanIndex = (fanIndex + 1) % 3;
			};
		}
#endif
		return res;
	}

	ChannelsConfiguration
	ITEConfigurationsA(MotherboardId board, Chip chip, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASUS: {
				switch (board.model) {
					case Model::ROG_CROSSHAIR_III_FORMULA: // IT8720F
					{
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("CPU", 0);

						addDefaultEntries(res, SensorType::fan, nrChannels);
			
						break;
					}
					case Model::M2N_SLI_Deluxe: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 30.f, 10.f);
						res.voltage.emplace_back("+5VSB", 7, 6.8f, 10.f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Chassis Fan #1", 1);
						res.fan.emplace_back("Power Fan", 2);

						break;
					}
					case Model::M4A79XTD_EVO: // IT8720F
					{
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Chassis Fan #1", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);

						break;
					}
					case Model::PRIME_X370_PRO: // IT8665E
					case Model::TUF_X470_PLUS_GAMING: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("SB 2.5V", 1);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("Voltage #4", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("+3.3V", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("Voltage #10", 9, true);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.temperature.emplace_back("PCH", 2);

						addDefaultEntries(res, SensorType::temp, nrChannels, 3);
						
						res.fan.emplace_back("CPU Fan", 0);
						addDefaultEntries(res, SensorType::fan, nrChannels, 1);

						break;
					}
					case Model::ROG_ZENITH_EXTREME: // IT8665E
					{
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("DIMM AB", 1, 10.f, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("SB 1.05V", 4, 10.f, 10.f);
						res.voltage.emplace_back("DIMM CD", 5, 10.f, 10.f);
						res.voltage.emplace_back("1.8V PLL", 6, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.temperature.emplace_back("CPU Socket", 2);
						res.temperature.emplace_back("Temperature #4", 3);
						res.temperature.emplace_back("Temperature #5", 4);
						res.temperature.emplace_back("VRM", 5);

						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Chassis Fan #1", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);
						res.fan.emplace_back("High Amp Fan", 3);
						res.fan.emplace_back("Fan 5", 4);
						res.fan.emplace_back("Fan 6", 5);

						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("Voltage #8", 7, true);
						res.voltage.emplace_back("VBat", 8);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::ASRock: {
				switch (board.model) {
					case Model::P55_Deluxe: // IT8720F
					{
						return ASRockConfiguration(board, chip, nrChannels);
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("Voltage #8", 7, true);
						res.voltage.emplace_back("VBat", 8);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::DFI: {
				switch (board.model) {
					case Model::LP_BI_P45_T2RS_Elite: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("VTT", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 30.f, 10.f);
						res.voltage.emplace_back("NB Core", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("+5VSB", 7, 6.8f, 10.f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("System", 1);
						res.temperature.emplace_back("Chipset", 2);
						res.fan.emplace_back("Fan #1", 0);
						res.fan.emplace_back("Fan #2", 1);
						res.fan.emplace_back("Fan #3", 2);

						break;
					}
					case Model::LP_DK_P55_T3EH9: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("VTT", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 30.f, 10.f);
						res.voltage.emplace_back("CPU PLL", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("+5VSB", 7, 6.8f, 10.f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("Chipset", 0);
						res.temperature.emplace_back("CPU PWM", 1);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("Fan #1", 0);
						res.fan.emplace_back("Fan #2", 1);
						res.fan.emplace_back("Fan #3", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("VTT", 1, true);
						res.voltage.emplace_back("+3.3V", 2, true);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f, 0.f, true);
						res.voltage.emplace_back("+12V", 4, 30.f, 10.f, 0.f, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("DIMM", 6, true);
						res.voltage.emplace_back("+5VSB", 7, 6.8f, 10.f, 0.f, true);
						res.voltage.emplace_back("VBat", 8);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::Gigabyte: {
				switch (board.model) {
					case Model::_965P_S3: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 7, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);

						break;
					}
					case Model::EP45_DS3R: // IT8718F
					case Model::EP45_UD3R:
					case Model::X38_DS5: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 7, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #2", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #1", 3);

						break;
					}
					case Model::EX58_EXTREME: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Northbridge", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #2", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #1", 3);

						break;
					}
					case Model::P35_DS3: // IT8718F
					case Model::P35_DS3L: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 7, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("Power Fan", 3);

						break;
					}
					case Model::P55_UD4: // IT8720F
					case Model::P55A_UD3: // IT8720F
					case Model::P55M_UD4: // IT8720F
					case Model::H55_USB3: // IT8720F
					case Model::EX58_UD3R: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 5, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #2", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #1", 3);

						break;
					}
					case Model::H55N_USB3: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 5, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);

						break;
					}
					case Model::G41M_COMBO: // IT8718F
					case Model::G41MT_S2: // IT8718F
					case Model::G41MT_S2P: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 7, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);

						break;
					}
					case Model::_970A_UD3: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("Power Fan", 4);
						res.pwm.emplace_back("PWM #1", 0);
						res.pwm.emplace_back("PWM #2", 1);
						res.pwm.emplace_back("PWM #3", 2);

						break;
					}
					case Model::MA770T_UD3: // IT8720F
					case Model::MA770T_UD3P: // IT8720F
					case Model::MA790X_UD3P: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("Power Fan", 3);

						break;
					}
					case Model::MA78LM_S2H: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("VRM", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("Power Fan", 3);

						break;
					}
					case Model::MA785GM_US2H: // IT8718F
					case Model::MA785GMT_UD2H: // IT8718F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 4, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);
						res.fan.emplace_back("NB Fan", 2);

						break;
					}
					case Model::X58A_UD3R: // IT8720F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("+3.3V", 2);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f);
						res.voltage.emplace_back("+12V", 5, 24.3f, 8.2f);
						res.voltage.emplace_back("VBat", 8);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Northbridge", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #2", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #1", 3);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1, true);
						res.voltage.emplace_back("+3.3V", 2, true);
						res.voltage.emplace_back("+5V", 3, 6.8f, 10.f, .0f, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("Voltage #8", 7, true);
						res.voltage.emplace_back("VBat", 8);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Vcore", 0);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("Voltage #3", 2, true);
				res.voltage.emplace_back("Voltage #4", 3, true);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("Voltage #8", 7, true);
				res.voltage.emplace_back("VBat", 8);

				addDefaultEntries(res, SensorType::temp, nrChannels);
				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	ITEConfigurationsB(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASUS: {
				switch (board.model) {
					case Model::ROG_STRIX_X470_I: // IT8665E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("SB 2.5V", 1);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("+3.3V", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.temperature.emplace_back("T_Sensor", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM", 4);
						res.temperature.emplace_back("Temperature #6", 5);

						res.fan.emplace_back("CPU Fan", 0);

						// Does not work when in AIO pump mode (shows 0). I don't know how to fix it.
						res.fan.emplace_back("Chassis Fan #1", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);

						// offset: 2, because the first two always show zero
						for (unsigned i = 2; i < getOrDefault(nrChannels, SensorType::pwm, 0); i++)
							res.pwm.emplace_back(fmt::format("Fan Control #{0}", i - 1), i);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("Voltage #8", 7, true);
						res.voltage.emplace_back("VBat", 8);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::ECS: {
				switch (board.model) {
					case Model::A890GXM_A: // IT8721F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("NB Voltage", 2);
						res.voltage.emplace_back("AVCC", 3, 10.f, 10.f);
						// res.voltage.emplace_back("DIMM", 6, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("System", 1);
						res.temperature.emplace_back("Northbridge", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);
						res.fan.emplace_back("Power Fan", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Voltage #1", 0, true);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("AVCC", 3, 10.f, 10.f, .0f, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::Gigabyte: {
				switch (board.model) {
					case Model::H61M_DS2_REV_1_2: // IT8728F
					case Model::H61M_USB3_B3_REV_2_0: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("+12V", 2, 30.9f, 10.f);
						res.voltage.emplace_back("Vcore", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);

						break;
					}
					case Model::H67A_UD3H_B3: // IT8728F
					case Model::H67A_USB3_B3: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("+5V", 1, 15.f, 10.f);
						res.voltage.emplace_back("+12V", 2, 30.9f, 10.f);
						res.voltage.emplace_back("Vcore", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #2", 3);

						break;
					}
					case Model::H81M_HD3: // IT8620E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("iGPU", 4);
						res.voltage.emplace_back("CPU VRIN", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("System", 0);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan", 1);

						break;
					}
					case Model::Z170N_WIFI: // ITE IT8628E
					{
						res.voltage.emplace_back("Vcore", 0, 0.f, 1.f);
						res.voltage.emplace_back("+3.3V", 1, 6.5F, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5F, 1.f);
						// NO DIMM CD channels on this motherboard; gives a very tiny voltage reading
						// res.voltage.emplace_back("DIMM CD", 4, .0f, 1.f);
						res.voltage.emplace_back("iGPU VAXG", 5, .0f, 1.f);
						res.voltage.emplace_back("DIMM AB", 6, .0f, 1.f);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("AVCC3", 9, 54.f, 10.f);

						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("PCH", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM", 4);
						res.temperature.emplace_back("System #2", 5);

						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan", 1);

						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan", 1);

						break;
					}
					case Model::AX370_Gaming_K7: // IT8686E
					case Model::AX370_Gaming_5:
					case Model::AB350_Gaming_3: // IT8686E
					{
						// Note: v3.3, v12, v5, and AVCC3 might be slightly off.
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 0.65f, 1.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("VSOC", 4);
						res.voltage.emplace_back("VDDP", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("AVCC3", 9, 7.53f, 1.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("Chipset", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);

						addDefaultEntries(res, SensorType::fan, nrChannels);

						break;
					}
					case Model::X399_AORUS_Gaming_7: // ITE IT8686E
					{
						res.voltage.emplace_back("Vcore", 0, .0f, 1.f);
						res.voltage.emplace_back("+3.3V", 1, 6.5f, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5F, 1.f);
						res.voltage.emplace_back("DIMM CD", 4, .0f, 1.f);
						res.voltage.emplace_back("Vcore SoC", 5, 0.f, 1.f);
						res.voltage.emplace_back("DIMM AB", 6, 0.f, 1.f);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("AVCC3", 9, 54.f, 10.f);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("Chipset", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM", 4);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::X470_AORUS_GAMING_7_WIFI: // ITE IT8686E
					{
						res.voltage.emplace_back("Vcore", 0, 0.f, 1.f);
						res.voltage.emplace_back("+3.3V", 1, 6.5f, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5F, 1.f);
						res.voltage.emplace_back("Vcore SoC", 4, 0.f, 1.f);
						res.voltage.emplace_back("VDDP", 5, 0.f, 1.f);
						res.voltage.emplace_back("DIMM AB", 6, 0.f, 1.f);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("AVCC3", 9, 54.f, 10.f);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("Chipset", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM", 4);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::B560M_AORUS_ELITE: // IT8689E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 29.4f, 45.3f);
						res.voltage.emplace_back("+12V", 2, 10.f, 2.f);
						res.voltage.emplace_back("+5V", 3, 15.f, 10.f);
						res.voltage.emplace_back("iGPU VAGX", 4);
						res.voltage.emplace_back("VCCSA", 5);
						res.voltage.emplace_back("DRAM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.voltage.emplace_back("AVCC3", 9, 59.9f, 9.8f);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("PCH", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);
						res.temperature.emplace_back("System #2", 5);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("System Fan #3", 3);
						res.fan.emplace_back("CPU OPT Fan", 4);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan #1", 1);
						res.pwm.emplace_back("System Fan #2", 2);
						res.pwm.emplace_back("System Fan #3", 3);
						res.pwm.emplace_back("CPU OPT Fan", 4);

						break;
					}
					case Model::X570_AORUS_MASTER: // IT8688E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 29.4f, 45.3f);
						res.voltage.emplace_back("+12V", 2, 10.f, 2.f);
						res.voltage.emplace_back("+5V", 3, 15.f, 10.f);
						res.voltage.emplace_back("Vcore SoC", 4);
						res.voltage.emplace_back("VDDP", 5);
						res.voltage.emplace_back("DIMM AB", 6);
						res.voltage.emplace_back("3VSB", 7, 1.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 1.f, 10.f);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("EC_TEMP1", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);
						res.temperature.emplace_back("PCH", 5);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("PCH Fan", 3);
						res.fan.emplace_back("CPU OPT Fan", 4);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan #1", 1);
						res.pwm.emplace_back("System Fan #2", 2);
						res.pwm.emplace_back("PCH Fan", 3);
						res.pwm.emplace_back("CPU OPT Fan", 4);

						break;
					}
					case Model::X570_GAMING_X: // IT8688E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 29.4f, 45.3f);
						res.voltage.emplace_back("+12V", 2, 10.f, 2.f);
						res.voltage.emplace_back("+5V", 3, 15.f, 10.f);
						res.voltage.emplace_back("Vcore SoC", 4);
						res.voltage.emplace_back("VDDP", 5);
						res.voltage.emplace_back("DIMM AB", 6);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("System #2", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);
						res.temperature.emplace_back("PCH", 5);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("PCH Fan", 3);
						res.fan.emplace_back("CPU OPT Fan", 4);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan #1", 1);
						res.pwm.emplace_back("System Fan #2", 2);
						res.pwm.emplace_back("PCH Fan", 3);
						res.pwm.emplace_back("CPU OPT Fan", 4);

						break;
					}
					case Model::Z390_M_GAMING: // IT8688E
					case Model::Z390_AORUS_ULTRA:
					case Model::Z390_UD: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 6.49f, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("CPU VCCGT", 4);
						res.voltage.emplace_back("CPU VCCSA", 5);
						res.voltage.emplace_back("VDDQ", 6);
						res.voltage.emplace_back("DDRVTT", 7);
						res.voltage.emplace_back("PCHCore", 8);
						res.voltage.emplace_back("CPU VCCIO", 9);
						res.voltage.emplace_back("DDRVPP", 10);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("PCH", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);
						res.temperature.emplace_back("System #2", 5);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("System Fan #3", 3);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan #1", 1);
						res.pwm.emplace_back("System Fan #2", 2);
						res.pwm.emplace_back("System Fan #3", 3);

						break;
					}
					case Model::Z390_AORUS_PRO: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 1, 6.49f, 10.f);
						res.voltage.emplace_back("+12V", 2, 5.f, 1.f);
						res.voltage.emplace_back("+5V", 3, 1.5f, 1.f);
						res.voltage.emplace_back("CPU VCCGT", 4);
						res.voltage.emplace_back("CPU VCCSA", 5);
						res.voltage.emplace_back("DDR", 6);
						res.voltage.emplace_back("Voltage #7", 7, true);
						res.voltage.emplace_back("3VSB", 8, 1.f, 1.f, -0.312f);
						res.voltage.emplace_back("VBat", 9, 6.f, 1.f, 0.01f);
						res.voltage.emplace_back("AVCC3", 10, 6.f, 1.f, 0.048f);
						res.temperature.emplace_back("System #1", 0);
						res.temperature.emplace_back("PCH", 1);
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("PCIe x16", 3);
						res.temperature.emplace_back("VRM MOS", 4);
						res.temperature.emplace_back("EC_TEMP1/System #2", 5);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("System Fan #3", 3);
						res.fan.emplace_back("CPU Optional Fan", 4);
						res.pwm.emplace_back("CPU Fan", 0);
						res.pwm.emplace_back("System Fan #1", 1);
						res.pwm.emplace_back("System Fan #2", 2);
						res.pwm.emplace_back("System Fan #3", 3);
						res.pwm.emplace_back("CPU Optional Fan", 4);

						break;
					}
					case Model::Z68A_D3H_B3: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("+3.3V", 1, 6.49f, 10.f);
						res.voltage.emplace_back("+12V", 2, 30.9f, 10.f);
						res.voltage.emplace_back("+5V", 3, 7.15f, 10.f);
						res.voltage.emplace_back("Vcore", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #2", 3);

						break;
					}
					case Model::P67A_UD3_B3: // IT8728F
					case Model::P67A_UD3R_B3: // IT8728F
					case Model::P67A_UD4_B3: // IT8728F
					case Model::Z68AP_D3: // IT8728F
					case Model::Z68X_UD3H_B3: // IT8728F
					case Model::Z68XP_UD3R: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("+3.3V", 1, 6.49f, 10.f);
						res.voltage.emplace_back("+12V", 2, 30.9f, 10.f);
						res.voltage.emplace_back("+5V", 3, 7.15f, 10.f);
						res.voltage.emplace_back("Vcore", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #2", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("System Fan #1", 3);

						break;
					}
					case Model::Z68X_UD7_B3: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("+3.3V", 1, 6.49f, 10.f);
						res.voltage.emplace_back("+12V", 2, 30.9f, 10.f);
						res.voltage.emplace_back("+5V", 3, 7.15f, 10.f);
						res.voltage.emplace_back("Vcore", 5);
						res.voltage.emplace_back("DIMM", 6);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("System #3", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Power Fan", 1);
						res.fan.emplace_back("System Fan #1", 2);
						res.fan.emplace_back("System Fan #2", 3);
						res.fan.emplace_back("System Fan #3", 4);

						break;
					}
					case Model::X79_UD3: // IT8728F
					{
						res.voltage.emplace_back("VTT", 0);
						res.voltage.emplace_back("DIMM AB", 1);
						res.voltage.emplace_back("+12V", 2, 10.f, 2.f);
						res.voltage.emplace_back("+5V", 3, 15.f, 10.f);
						res.voltage.emplace_back("VIN4", 4);
						res.voltage.emplace_back("VCore", 5);
						res.voltage.emplace_back("DIMM CD", 6);
						res.voltage.emplace_back("+3V Standby", 7, 1.f, 1.f);
						res.voltage.emplace_back("VBat", 8, 1.f, 1.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Northbridge", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("System Fan #1", 1);
						res.fan.emplace_back("System Fan #2", 2);
						res.fan.emplace_back("System Fan #3", 3);

						break;
					}
					default: {
						res.voltage.emplace_back("Voltage #1", 0, true);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::Shuttle: {
				switch (board.model) {
					case Model::FH67: // IT8772E
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("DIMM", 1);
						res.voltage.emplace_back("PCH VCCIO", 2);
						res.voltage.emplace_back("CPU VCCIO", 3);
						res.voltage.emplace_back("Graphic Voltage", 4);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("System", 0);
						res.temperature.emplace_back("CPU", 1);
						res.fan.emplace_back("Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);

						break;
					}
					default: {
						res.voltage.emplace_back("Voltage #1", 0, true);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Voltage #1", 0, true);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("Voltage #3", 2, true);
				res.voltage.emplace_back("Voltage #4", 3, true);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
				res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

				addDefaultEntries(res, SensorType::temp, nrChannels);
				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	ITEConfigurationsC(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::Gigabyte: {
				switch (board.model) {
					case Model::X570_AORUS_MASTER: // IT879XE
					{
						res.voltage.emplace_back("CPU VDD18", 0);
						res.voltage.emplace_back("DDRVTT AB", 1);
						res.voltage.emplace_back("Chipset Core", 2);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("CPU VDD18", 4);
						res.voltage.emplace_back("PM_CLDO12", 5);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 1.f, 1.f);
						res.voltage.emplace_back("VBat", 8, 1.f, 1.f);
						res.temperature.emplace_back("PCIe x8", 0);
						res.temperature.emplace_back("EC_TEMP2", 1);
						res.temperature.emplace_back("System #2", 2);
						res.fan.emplace_back("System Fan #5 Pump", 0);
						res.fan.emplace_back("System Fan #6 Pump", 1);
						res.fan.emplace_back("System Fan #4", 2);

						break;
					}
					case Model::X470_AORUS_GAMING_7_WIFI: // ITE IT8792
					{
						res.voltage.emplace_back("VIN0", 0, 0.f, 1.f);
						res.voltage.emplace_back("DDR VTT", 1, 0.f, 1.f);
						res.voltage.emplace_back("Chipset Core", 2, 0.f, 1.f);
						res.voltage.emplace_back("VIN3", 3, 0.f, 1.f);
						res.voltage.emplace_back("CPU VDD18", 4, 0.f, 1.f);
						res.voltage.emplace_back("Chipset Core +2.5V", 5, 0.5f, 1.f);
						res.voltage.emplace_back("3VSB", 6, 1.f, 10.f);
						res.voltage.emplace_back("VBat", 7, 0.7f, 1.f);
						res.temperature.emplace_back("PCIe x8", 0);
						res.temperature.emplace_back("System #2", 2);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::Z390_AORUS_PRO: // IT879XE
					{
						res.voltage.emplace_back("VCore", 0);
						res.voltage.emplace_back("DDRVTT AB", 1);
						res.voltage.emplace_back("Chipset Core", 2);
						res.voltage.emplace_back("VIN3", 3, true);
						res.voltage.emplace_back("VCCIO", 4);
						res.voltage.emplace_back("Voltage #7", 5, true);
						res.voltage.emplace_back("DDR VPP", 6);
						res.voltage.emplace_back("3VSB", 7, 1.f, 1.f);
						res.voltage.emplace_back("VBat", 8, 1.f, 1.f);
						res.temperature.emplace_back("PCIe x8", 0);
						res.temperature.emplace_back("EC_TEMP2", 1);
						res.temperature.emplace_back("System #2", 2);
						res.fan.emplace_back("System Fan #5 Pump", 0);
						res.fan.emplace_back("System Fan #6 Pump", 1);
						res.fan.emplace_back("System Fan #4", 2);
						res.pwm.emplace_back("Fan Control #5", 0);
						res.pwm.emplace_back("Fan Control #6", 1);
						res.pwm.emplace_back("Fan Control #4", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Voltage #1", 0, true);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Voltage #1", 0, true);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("Voltage #3", 2, true);
				res.voltage.emplace_back("Voltage #4", 3, true);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 10.f, 10.f, 0.f, true);
				res.voltage.emplace_back("VBat", 8, 10.f, 10.f);

				addDefaultEntries(res, SensorType::temp, nrChannels);
				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	fintekConfiguration(MotherboardId board, Chip chip, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::EVGA: {
				switch (board.model) {
					case Model::X58_SLI_Classified: // F71882
					{
						res.voltage.emplace_back("VCC3V", 0, 150.f, 150.f);
						res.voltage.emplace_back("Vcore", 1, 47.f, 100.f);
						res.voltage.emplace_back("DIMM", 2, 47.f, 100.f);
						res.voltage.emplace_back("CPU VTT", 3, 24.f, 100.f);
						res.voltage.emplace_back("IOH Vcore", 4, 24.f, 100.f);
						res.voltage.emplace_back("+5V", 5, 51.f, 12.f);
						res.voltage.emplace_back("+12V", 6, 56.f, 6.8f);
						res.voltage.emplace_back("3VSB", 7, 150.f, 150.f);
						res.voltage.emplace_back("VBat", 8, 150.f, 150.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("VREG", 1);
						res.temperature.emplace_back("System", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Power Fan", 1);
						res.fan.emplace_back("Chassis Fan", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("VCC3V", 0, 150.f, 150.f);
						res.voltage.emplace_back("Vcore", 1);
						res.voltage.emplace_back("Voltage #3", 2, true);
						res.voltage.emplace_back("Voltage #4", 3, true);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("VSB3V", 7, 150.f, 150.f);
						res.voltage.emplace_back("VBat", 8, 150.f, 150.f);

						addDefaultEntries(res, SensorType::temp, nrChannels);
						addDefaultEntries(res, SensorType::fan, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("VCC3V", 0, 150.f, 150.f);
				res.voltage.emplace_back("Vcore", 1);
				res.voltage.emplace_back("Voltage #3", 2, true);
				res.voltage.emplace_back("Voltage #4", 3, true);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				if (chip != Chip::F71808E)
					res.voltage.emplace_back("Voltage #7", 6, true);

				res.voltage.emplace_back("VSB3V", 7, 150.f, 150.f);
				res.voltage.emplace_back("VBat", 8, 150.f, 150.f);

				addDefaultEntries(res, SensorType::temp, nrChannels);
				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	winbondConfigurationEhf(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& /*nrChannels*/)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASRock: {
				switch (board.model) {
					case Model::AOD790GX_128M: // W83627EHF
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 4, 10.f, 10.f);
						res.voltage.emplace_back("+5V", 5, 20.f, 10.f);
						res.voltage.emplace_back("+12V", 6, 28.f, 5.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("CPU Fan", 0);
						res.fan.emplace_back("Chassis Fan", 1);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #10", 9, true);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("System", 2);
						res.fan.emplace_back("System Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Auxiliary Fan", 2);
						res.fan.emplace_back("CPU Fan #2", 3);
						res.fan.emplace_back("Auxiliary Fan #2", 4);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Vcore", 0);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
				res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
				res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
				res.voltage.emplace_back("Voltage #10", 9, true);
				res.temperature.emplace_back("CPU", 0);
				res.temperature.emplace_back("Auxiliary", 1);
				res.temperature.emplace_back("System", 2);
				res.fan.emplace_back("System Fan", 0);
				res.fan.emplace_back("CPU Fan", 1);
				res.fan.emplace_back("Auxiliary Fan", 2);
				res.fan.emplace_back("CPU Fan #2", 3);
				res.fan.emplace_back("Auxiliary Fan #2", 4);
				res.pwm.emplace_back("System Fan", 0);
				res.pwm.emplace_back("CPU Fan", 1);
				res.pwm.emplace_back("Auxiliary Fan", 2);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	winbondConfigurationHg(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& /*nrChannels*/)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASRock: {
				switch (board.model) {
					case Model::_880GMH_USB3: // W83627DHG-P
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 5, 15.f, 7.5f);
						res.voltage.emplace_back("+12V", 6, 56.f, 10.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("Chassis Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("System", 2);
						res.fan.emplace_back("System Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Auxiliary Fan", 2);
						res.fan.emplace_back("CPU Fan #2", 3);
						res.fan.emplace_back("Auxiliary Fan #2", 4);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
				}

				break;
			}
			case Manufacturer::ASUS: {
				switch (board.model) {
					case Model::P6T: // W83667HG
					case Model::P6X58D_E: // W83667HG
					case Model::RAMPAGE_II_GENE: // W83667HG
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 11.5f, 1.91f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 15.f, 7.5f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("Chassis Fan #2", 3);
						res.fan.emplace_back("Chassis Fan #3", 4);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
					case Model::RAMPAGE_EXTREME: // W83667HG
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 12.f, 2.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 15.f, 7.5f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("Chassis Fan #2", 3);
						res.fan.emplace_back("Chassis Fan #3", 4);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("System", 2);
						res.fan.emplace_back("System Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Auxiliary Fan", 2);
						res.fan.emplace_back("CPU Fan #2", 3);
						res.fan.emplace_back("Auxiliary Fan #2", 4);
						res.pwm.emplace_back("System Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Auxiliary Fan", 2);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Vcore", 0);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
				res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
				res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
				res.temperature.emplace_back("CPU", 0);
				res.temperature.emplace_back("Auxiliary", 1);
				res.temperature.emplace_back("System", 2);
				res.fan.emplace_back("System Fan", 0);
				res.fan.emplace_back("CPU Fan", 1);
				res.fan.emplace_back("Auxiliary Fan", 2);
				res.fan.emplace_back("CPU Fan #2", 3);
				res.fan.emplace_back("Auxiliary Fan #2", 4);
				res.pwm.emplace_back("System Fan", 0);
				res.pwm.emplace_back("CPU Fan", 1);
				res.pwm.emplace_back("Auxiliary Fan", 2);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	nuvotonConfigurationF(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASUS: {
				switch (board.model) {
					case Model::P8P67: // NCT6776F
					case Model::P8P67_EVO: // NCT6776F
					case Model::P8P67_PRO: // NCT6776F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 11.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 12.f, 3.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 2);
						res.temperature.emplace_back("Motherboard", 3);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("Chassis Fan #2", 3);
						res.pwm.emplace_back("Chassis Fan #2", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Chassis Fan #1", 2);

						break;
					}
					case Model::P8P67_M_PRO: // NCT6776F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 11.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 12.f, 3.f);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 3);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);
						res.fan.emplace_back("Power Fan", 3);
						res.fan.emplace_back("Auxiliary Fan", 4);

						break;
					}
					case Model::P8Z68_V_PRO: // NCT6776F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 11.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 12.f, 3.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 2);
						res.temperature.emplace_back("Motherboard", 3);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::P9X79: // NCT6776F
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+12V", 1, 11.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+5V", 4, 12.f, 3.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Motherboard", 3);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Temperature #1", 1);
						res.temperature.emplace_back("Temperature #2", 2);
						res.temperature.emplace_back("Temperature #3", 3);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::ASRock: {
				switch (board.model) {
					case Model::B85M_DGS: {
						res.voltage.emplace_back("Vcore", 0, 1.f, 1.f);
						res.voltage.emplace_back("+12V", 1, 56.f, 10.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("VIN1", 4, true);
						res.voltage.emplace_back("+5V", 5, 12.f, 3.f);
						res.voltage.emplace_back("VIN3", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 2);
						res.temperature.emplace_back("Motherboard", 3);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Power Fan", 2);
						res.fan.emplace_back("Chassis Fan #2", 3);
						res.pwm.emplace_back("Chassis Fan #2", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Chassis Fan #1", 2);
					}

					break;
					case Model::Z77Pro4M: // NCT6776F
					{
						res.voltage.emplace_back("Vcore", 0, 0.f, 1.f);
						res.voltage.emplace_back("+12V", 1, 56.f, 10.f);
						res.voltage.emplace_back("AVCC", 2, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 3, 10.f, 10.f);
						// res.voltage.emplace_back("#Unused #4", 4, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("+5V", 5, 20.f, 10.f);
						// res.voltage.emplace_back("#Unused #6", 6, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Auxiliary", 2);
						res.temperature.emplace_back("Motherboard", 3);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Temperature #1", 1);
						res.temperature.emplace_back("Temperature #2", 2);
						res.temperature.emplace_back("Temperature #3", 3);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Vcore", 0);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
				res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
				res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
				res.temperature.emplace_back("CPU Core", 0);
				res.temperature.emplace_back("Temperature #1", 1);
				res.temperature.emplace_back("Temperature #2", 2);
				res.temperature.emplace_back("Temperature #3", 3);

				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration
	nuvotonConfigurationD(MotherboardId board, Chip /*chip*/, const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		switch (board.manufacturer) {
			case Manufacturer::ASRock: {
				switch (board.model) {
					case Model::A320M_HDV: // NCT6779D
					{
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("Chipset 1.05V", 1, 0.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 3, 10.f, 10.f);
						res.voltage.emplace_back("+12V", 4, 56.f, 10.f);
						res.voltage.emplace_back("VcoreRef", 5, 0.f, 1.f);
						res.voltage.emplace_back("DIMM", 6, 0.f, 1.f);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						// res.voltage.emplace_back("#Unused #9", 9, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #10", 10, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #11", 11, 34.f, 34.f, 0.f, true);
						res.voltage.emplace_back("+5V", 12, 20.f, 10.f);
						// res.voltage.emplace_back("#Unused #13", 13, 10.f, 10.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #14", 14, 0.f, 1.f, 0.f, true);

						// res.temperature.emplace_back("#Unused #0", 0);
						// res.temperature.emplace_back("#Unused #1", 1);
						res.temperature.emplace_back("Motherboard", 2);
						// res.temperature.emplace_back("#Unused #3", 3);
						// res.temperature.emplace_back("#Unused #4", 4);
						res.temperature.emplace_back("Auxiliary", 5);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}

					case Model::AB350_Pro4: // NCT6779D
					case Model::AB350M_Pro4:
					case Model::AB350M:
					case Model::Fatal1ty_AB350_Gaming_K4:
					case Model::AB350M_HDV:
					case Model::B450_Steel_Legend:
					case Model::B450M_Steel_Legend:
					case Model::B450_Pro4:
					case Model::B450M_Pro4: {
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						// res.voltage.emplace_back("#Unused", 1, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("AVCC", 2, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 3, 10.f, 10.f);
						res.voltage.emplace_back("+12V", 4, 28.f, 5.f);
						res.voltage.emplace_back("Vcore Refin", 5, 0.f, 1.f);
						// res.voltage.emplace_back("#Unused #6", 6, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						// res.voltage.emplace_back("#Unused #9", 9, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #10", 10, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("Chipset 1.05V", 11, 0.f, 1.f);
						res.voltage.emplace_back("+5V", 12, 20.f, 10.f);
						// res.voltage.emplace_back("#Unused #13", 13, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("+1.8V", 14, 0.f, 1.f);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.temperature.emplace_back("Auxiliary", 3);
						res.temperature.emplace_back("VRM", 4);
						res.temperature.emplace_back("AUXTIN2", 5);
						// res.temperature.emplace_back("Temperature #6", 6);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::X399_Phantom_Gaming_6: // NCT6779D
					{
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("Chipset 1.05V", 1, 0.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 3, 10.f, 10.f);
						res.voltage.emplace_back("+12V", 4, 56.f, 10.f);
						res.voltage.emplace_back("VDDCR_SOC", 5, 0.f, 1.f);
						res.voltage.emplace_back("DIMM", 6, 0.f, 1.f);
						res.voltage.emplace_back("3VSB", 7, 10.f, 10.f);
						res.voltage.emplace_back("VBat", 8, 10.f, 10.f);
						// res.voltage.emplace_back("#Unused", 9, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused", 10, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused", 11, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("+5V", 12, 20.f, 10.f);
						res.voltage.emplace_back("+1.8V", 13, 10.f, 10.f);
						// res.voltage.emplace_back("unused", 14, 34.f, 34.f, 0.f, true);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Motherboard", 1);
						res.temperature.emplace_back("Auxiliary", 2);
						res.temperature.emplace_back("Chipset", 3);
						res.temperature.emplace_back("Core VRM", 4);
						res.temperature.emplace_back("Core SoC", 5);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::X570_Taichi: {
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);

						res.temperature.emplace_back("Motherboard", 2);
						res.temperature.emplace_back("CPU", 8);
						res.temperature.emplace_back("SB (Chipset)", 9);

						res.fan.emplace_back("Chassis #3", 0);
						res.fan.emplace_back("CPU #1", 1);
						res.fan.emplace_back("CPU #2", 2);
						res.fan.emplace_back("Chassis #1", 3);
						res.fan.emplace_back("Chassis #2", 4);
						res.fan.emplace_back("SB Fan", 5);
						res.fan.emplace_back("Chassis #4", 6);

						res.pwm.emplace_back("Chassis #3", 0);
						res.pwm.emplace_back("CPU #1", 1);
						res.pwm.emplace_back("CPU #2", 2);
						res.pwm.emplace_back("Chassis #1", 3);
						res.pwm.emplace_back("Chassis #2", 4);
						res.pwm.emplace_back("SB Fan", 5);
						res.pwm.emplace_back("Chassis #4", 6);

						break;
					}
					case Model::X570_Phantom_Gaming_ITX: {
						res.voltage.emplace_back("+12V", 0);
						res.voltage.emplace_back("+5V", 1);
						res.voltage.emplace_back("Vcore", 2);
						res.voltage.emplace_back("Voltage #1", 3);
						res.voltage.emplace_back("DIMM", 4);
						res.voltage.emplace_back("CPU I/O", 5);
						res.voltage.emplace_back("CPU SA", 6);
						res.voltage.emplace_back("Voltage #2", 7);
						res.voltage.emplace_back("AVCC3", 8);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("VRef", 10);
						res.voltage.emplace_back("VSB", 11);
						res.voltage.emplace_back("AVSB", 12);
						res.voltage.emplace_back("VBat", 13);

						res.temperature.emplace_back("Motherboard", 0);
						// res.temperature.emplace_back("System", 1); //Unused
						res.temperature.emplace_back("CPU", 2);
						res.temperature.emplace_back("SB (Chipset)", 3);
						res.fan.emplace_back("CPU Fan #1", 0); // CPU_FAN1
						res.fan.emplace_back("Chassis Fan #1", 1); // CHA_FAN1/WP
						res.fan.emplace_back("CPU Fan #2", 2); // CPU_FAN2 (WP)
						res.fan.emplace_back("Chipset Fan", 3);

						res.pwm.emplace_back("CPU Fan #1", 0);
						res.pwm.emplace_back("Chassis Fan", 1);
						res.pwm.emplace_back("CPU Fan #2", 2);
						res.pwm.emplace_back("Chipset Fan", 3);
						break;
					}

					default: {
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Temperature #1", 1);
						res.temperature.emplace_back("Temperature #2", 2);
						res.temperature.emplace_back("Temperature #3", 3);
						res.temperature.emplace_back("Temperature #4", 4);
						res.temperature.emplace_back("Temperature #5", 5);
						res.temperature.emplace_back("Temperature #6", 6);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::ASUS: {
				switch (board.model) {
					case Model::P8Z77_V: // NCT6779D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);
						res.fan.emplace_back("Chassis Fan #3", 3);
						res.pwm.emplace_back("Chassis Fan #1", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Chassis Fan #2", 2);
						res.pwm.emplace_back("Chassis Fan #3", 3);

						break;
					}
					case Model::ROG_MAXIMUS_X_APEX: // NCT6793D
					{
						res.voltage.emplace_back("Vcore", 0, 2.f, 2.f);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVSB", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("CPU GFX", 6, 2.f, 2.f);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("DIMM", 10, 1.f, 1.f);
						res.voltage.emplace_back("VCCSA", 11);
						res.voltage.emplace_back("PCH Core", 12);
						res.voltage.emplace_back("CPU PLLs", 13);
						res.voltage.emplace_back("CPU VCCIO/IMC", 14);
						res.temperature.emplace_back("CPU (PECI)", 0);
						res.temperature.emplace_back("T2", 1);
						res.temperature.emplace_back("T1", 2);
						res.temperature.emplace_back("CPU", 3);
						res.temperature.emplace_back("PCH", 4);
						res.temperature.emplace_back("Temperature #4", 5);
						res.temperature.emplace_back("Temperature #5", 6);
						res.fan.emplace_back("Chassis Fan #1", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("Chassis Fan #2", 2);
						res.fan.emplace_back("Chassis Fan #3", 3);
						res.fan.emplace_back("AIO Pump", 4);
						res.pwm.emplace_back("Chassis Fan #1", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("Chassis Fan #2", 2);
						res.pwm.emplace_back("Chassis Fan #3", 3);
						res.pwm.emplace_back("AIO Pump", 4);

						break;
					}
					case Model::Z170_A: // NCT6793D
					{
						res.voltage.emplace_back("Vcore", 0, 2.f, 2.f);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVSB", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						res.voltage.emplace_back("Voltage #6", 5, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("CPU GFX", 6, 2.f, 2.f);
						res.voltage.emplace_back("3VSB_ATX", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("DIMM", 10, 1.f, 1.f);
						res.voltage.emplace_back("VCCSA", 11);
						res.voltage.emplace_back("PCH Core", 12);
						res.voltage.emplace_back("CPU PLLs", 13);
						res.voltage.emplace_back("CPU VCCIO/IMC", 14);
						res.temperature.emplace_back("CPU (PECI)", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.temperature.emplace_back("CPU", 3);
						res.temperature.emplace_back("PCH", 4);
						res.temperature.emplace_back("Temperature #4", 5);
						res.temperature.emplace_back("Temperature #5", 6);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::TUF_GAMING_B550M_PLUS_WIFI: // NCT6798D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("PECI 0", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("System", 2);
						res.temperature.emplace_back("AUX 0", 3);
						res.temperature.emplace_back("AUX 1", 4);
						res.temperature.emplace_back("AUX 2", 5);
						res.temperature.emplace_back("AUX 3", 6);
						res.temperature.emplace_back("AUX 4", 7);
						res.temperature.emplace_back("SMBus 0", 8);
						res.temperature.emplace_back("SMBus 1", 9);
						res.temperature.emplace_back("PECI 1", 10);
						res.temperature.emplace_back("PCH Chip CPU Max", 11);
						res.temperature.emplace_back("PCH Chip", 12);
						res.temperature.emplace_back("PCH CPU", 13);
						res.temperature.emplace_back("PCH MCH", 14);
						res.temperature.emplace_back("Agent 0 DIMM 0", 15);
						res.temperature.emplace_back("Agent 0 DIMM 1", 16);
						res.temperature.emplace_back("Agent 1 DIMM 0", 17);
						res.temperature.emplace_back("Agent 1 DIMM 1", 18);
						res.temperature.emplace_back("Device 0", 19);
						res.temperature.emplace_back("Device 1", 20);
						res.temperature.emplace_back("PECI 0 Calibrated", 21);
						res.temperature.emplace_back("PECI 1 Calibrated", 22);
						res.temperature.emplace_back("Virtual", 23);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					case Model::ROG_CROSSHAIR_VIII_HERO: // NCT6798D
					case Model::ROG_CROSSHAIR_VIII_HERO_WIFI: // NCT6798D
					case Model::ROG_CROSSHAIR_VIII_DARK_HERO: // NCT6798D
					case Model::ROG_CROSSHAIR_VIII_FORMULA: // NCT6798D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("CPU SoC", 6);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("DRAM", 13);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("PECI 0", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.temperature.emplace_back("AUX 0", 3);
						res.temperature.emplace_back("AUX 1", 4);
						res.temperature.emplace_back("AUX 2", 5);
						res.temperature.emplace_back("AUX 3", 6);
						res.temperature.emplace_back("AUX 4", 7);
						res.temperature.emplace_back("SMBus 0", 8);
						res.temperature.emplace_back("SMBus 1", 9);
						res.temperature.emplace_back("PECI 1", 10);
						res.temperature.emplace_back("PCH Chip CPU Max", 11);
						res.temperature.emplace_back("PCH Chip", 12);
						res.temperature.emplace_back("PCH CPU", 13);
						res.temperature.emplace_back("PCH MCH", 14);
						res.temperature.emplace_back("Agent 0 DIMM 0", 15);
						res.temperature.emplace_back("Agent 0 DIMM 1", 16);
						res.temperature.emplace_back("Agent 1 DIMM 0", 17);
						res.temperature.emplace_back("Agent 1 DIMM 1", 18);
						res.temperature.emplace_back("Device 0", 19);
						res.temperature.emplace_back("Device 1", 20);
						res.temperature.emplace_back("PECI 0 Calibrated", 21);
						res.temperature.emplace_back("PECI 1 Calibrated", 22);
						res.temperature.emplace_back("Virtual", 23);

						std::vector<std::string_view> fanControlNames = {
						    "Chassis Fan 1", "CPU Fan", "Chassis Fan 2", "Chassis Fan 3",
						    "High Amp Fan",  "W_PUMP+", "AIO Pump"};
						assert(fanControlNames.size() == getOrDefault(nrChannels, SensorType::fan, 0));
						assert(
						    getOrDefault(nrChannels, SensorType::fan, 0) ==
						    getOrDefault(nrChannels, SensorType::pwm, 0));

						for (unsigned i = 0; i < fanControlNames.size(); i++)
							res.fan.emplace_back(std::string(fanControlNames[i]), i);

						for (unsigned i = 0; i < fanControlNames.size(); i++)
							res.pwm.emplace_back(std::string(fanControlNames[i]), i);


						break;
					}
					case Model::ROG_STRIX_B550_I_GAMING: // NCT6798D
					{
						res.voltage.emplace_back("Vcore", 0, 10.f, 10.f);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f); // Probably not updating properly
						res.voltage.emplace_back("AVCC", 2, 10.f, 10.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f); // Probably not updating properly
						// res.voltage.emplace_back("#Unused #5", 5, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #6", 6, 0.f, 1.f, 0.f, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						// res.voltage.emplace_back("#Unused #9", 9, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #10", 10, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #11", 11, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #12", 12, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #13", 13, 0.f, 1.f, 0.f, true);
						// res.voltage.emplace_back("#Unused #14", 14, 0.f, 1.f, 0.f, true);

						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("Motherboard", 2);
						// res.temperature.emplace_back("AUX 0", 3); //No software from Asus reports this temperature
						// ~82C res.temperature.emplace_back("#Unused 4", 4); res.temperature.emplace_back("#Unused 5",
						// 5); res.temperature.emplace_back("#Unused 6", 6); res.temperature.emplace_back("#Unused 7",
						// 7); res.temperature.emplace_back("#Unused 8", 8); res.temperature.emplace_back("#Unused 9",
						// 9); res.temperature.emplace_back("#Unused 10", 10);
						res.temperature.emplace_back("PCH Chip CPU Max", 11);
						res.temperature.emplace_back("PCH Chip", 12);
						res.temperature.emplace_back("PCH CPU", 13);
						res.temperature.emplace_back("PCH MCH", 14);
						res.temperature.emplace_back("Agent 0 DIMM 0", 15);
						// res.temperature.emplace_back("Agent 0 DIMM 1", 16);
						res.temperature.emplace_back("Agent 1 DIMM 0", 17);
						// res.temperature.emplace_back("Agent 1 DIMM 1", 18);
						res.temperature.emplace_back("Device 0", 19);
						res.temperature.emplace_back("Device 1", 20);
						res.temperature.emplace_back("PECI 0 Calibrated", 21);
						res.temperature.emplace_back("PECI 1 Calibrated", 22);
						res.temperature.emplace_back("Virtual", 23);

						for (unsigned i = 0; i < getOrDefault(nrChannels, SensorType::fan, 0); i++) {
							switch (i) {
								case 0: res.fan.emplace_back("Chassis Fan", 0); break;
								case 1: res.fan.emplace_back("CPU Fan", 1); break;
								case 4: res.fan.emplace_back("AIO Pump", 4); break;
							}
						}

						for (unsigned i = 0; i < getOrDefault(nrChannels, SensorType::pwm, 0); i++) {
							switch (i) {
								case 0: res.pwm.emplace_back("Chassis Fan Control", 0); break;
								case 1: res.pwm.emplace_back("CPU Fan Control", 1); break;
								case 4: res.pwm.emplace_back("AIO Pump Control", 4); break;
							}
						}

						break;
					}
					case Model::ROG_STRIX_B550_F_GAMING_WIFI: // NCT6798D-R
					{
						res.voltage.emplace_back("Vcore", 0, 2.f, 2.f);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU Core", 0);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Temperature #1", 1);
						res.temperature.emplace_back("Temperature #2", 2);
						res.temperature.emplace_back("Temperature #3", 3);
						res.temperature.emplace_back("Temperature #4", 4);
						res.temperature.emplace_back("Temperature #5", 5);
						res.temperature.emplace_back("Temperature #6", 6);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			case Manufacturer::MSI: {
				switch (board.model) {
					case Model::B360M_PRO_VDH: // NCT6797D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						// res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("CPU I/O", 6);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("CPU SA", 10);
						// res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("NB/SoC", 12);
						res.voltage.emplace_back("DIMM", 13, 1.f, 1.f);
						// res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.temperature.emplace_back("Temperature #1", 5);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("System Fan #1", 2);
						res.fan.emplace_back("System Fan #2", 3);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("System Fan #1", 2);
						res.pwm.emplace_back("System Fan #2", 3);

						break;
					}
					case Model::B450A_PRO: // NCT6797D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						// res.voltage.emplace_back("Voltage #6", 5, false);
						// res.voltage.emplace_back("CPU I/O", 6);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("CPU SA", 10);
						// res.voltage.emplace_back("Voltage #12", 11, false);
						res.voltage.emplace_back("NB/SoC", 12);
						res.voltage.emplace_back("DIMM", 13, 1.f, 1.f);
						// res.voltage.emplace_back("Voltage #15", 14, false);
						// res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("CPU", 1);
						res.temperature.emplace_back("System", 2);
						res.temperature.emplace_back("VRM MOS", 3);
						res.temperature.emplace_back("PCH", 5);
						res.temperature.emplace_back("SMBus 0", 8);
						res.fan.emplace_back("Pump Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("System Fan #1", 2);
						res.fan.emplace_back("System Fan #2", 3);
						res.fan.emplace_back("System Fan #3", 4);
						res.fan.emplace_back("System Fan #4", 5);
						res.pwm.emplace_back("Pump Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("System Fan #1", 2);
						res.pwm.emplace_back("System Fan #2", 3);
						res.pwm.emplace_back("System Fan #3", 4);
						res.pwm.emplace_back("System Fan #4", 5);

						break;
					}
					case Model::Z270_PC_MATE: // NCT6795D
					{
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("+5V", 1, 4.f, 1.f);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("+12V", 4, 11.f, 1.f);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("CPU I/O", 6);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("CPU SA", 10);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("PCH", 12);
						res.voltage.emplace_back("DIMM", 13, 1.f, 1.f);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU", 0);
						res.temperature.emplace_back("Auxiliary", 1);
						res.temperature.emplace_back("Motherboard", 2);
						res.fan.emplace_back("Pump Fan", 0);
						res.fan.emplace_back("CPU Fan", 1);
						res.fan.emplace_back("System Fan #1", 2);
						res.fan.emplace_back("System Fan #2", 3);
						res.fan.emplace_back("System Fan #3", 4);
						res.fan.emplace_back("System Fan #4", 5);
						res.pwm.emplace_back("Pump Fan", 0);
						res.pwm.emplace_back("CPU Fan", 1);
						res.pwm.emplace_back("System Fan #1", 2);
						res.pwm.emplace_back("System Fan #2", 3);
						res.pwm.emplace_back("System Fan #3", 4);
						res.pwm.emplace_back("System Fan #4", 5);

						break;
					}
					default: {
						res.voltage.emplace_back("Vcore", 0);
						res.voltage.emplace_back("Voltage #2", 1, true);
						res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
						res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
						res.voltage.emplace_back("Voltage #5", 4, true);
						res.voltage.emplace_back("Voltage #6", 5, true);
						res.voltage.emplace_back("Voltage #7", 6, true);
						res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
						res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
						res.voltage.emplace_back("VTT", 9);
						res.voltage.emplace_back("Voltage #11", 10, true);
						res.voltage.emplace_back("Voltage #12", 11, true);
						res.voltage.emplace_back("Voltage #13", 12, true);
						res.voltage.emplace_back("Voltage #14", 13, true);
						res.voltage.emplace_back("Voltage #15", 14, true);
						res.temperature.emplace_back("CPU Core", 0);
						res.temperature.emplace_back("Temperature #1", 1);
						res.temperature.emplace_back("Temperature #2", 2);
						res.temperature.emplace_back("Temperature #3", 3);
						res.temperature.emplace_back("Temperature #4", 4);
						res.temperature.emplace_back("Temperature #5", 5);
						res.temperature.emplace_back("Temperature #6", 6);

						addDefaultEntries(res, SensorType::fan, nrChannels);
						addDefaultEntries(res, SensorType::pwm, nrChannels);

						break;
					}
				}

				break;
			}
			default: {
				res.voltage.emplace_back("Vcore", 0);
				res.voltage.emplace_back("Voltage #2", 1, true);
				res.voltage.emplace_back("AVCC", 2, 34.f, 34.f);
				res.voltage.emplace_back("+3.3V", 3, 34.f, 34.f);
				res.voltage.emplace_back("Voltage #5", 4, true);
				res.voltage.emplace_back("Voltage #6", 5, true);
				res.voltage.emplace_back("Voltage #7", 6, true);
				res.voltage.emplace_back("3VSB", 7, 34.f, 34.f);
				res.voltage.emplace_back("VBat", 8, 34.f, 34.f);
				res.voltage.emplace_back("VTT", 9);
				res.voltage.emplace_back("Voltage #11", 10, true);
				res.voltage.emplace_back("Voltage #12", 11, true);
				res.voltage.emplace_back("Voltage #13", 12, true);
				res.voltage.emplace_back("Voltage #14", 13, true);
				res.voltage.emplace_back("Voltage #15", 14, true);
				res.temperature.emplace_back("CPU Core", 0);
				res.temperature.emplace_back("Temperature #1", 1);
				res.temperature.emplace_back("Temperature #2", 2);
				res.temperature.emplace_back("Temperature #3", 3);
				res.temperature.emplace_back("Temperature #4", 4);
				res.temperature.emplace_back("Temperature #5", 5);
				res.temperature.emplace_back("Temperature #6", 6);

				addDefaultEntries(res, SensorType::fan, nrChannels);
				addDefaultEntries(res, SensorType::pwm, nrChannels);

				break;
			}
		}
		return res;
	}

	ChannelsConfiguration generateDefaultConfiguration(const std::map<SensorType, std::size_t>& nrChannels)
	{
		ChannelsConfiguration res;
		addDefaultEntries(res, SensorType::voltage, nrChannels);
		addDefaultEntries(res, SensorType::temp, nrChannels);
		addDefaultEntries(res, SensorType::fan, nrChannels);
		addDefaultEntries(res, SensorType::pwm, nrChannels);
		return res;
	}
} // namespace

wm_sensors::hardware::motherboard::lpc::ChannelsConfiguration
wm_sensors::hardware::motherboard::lpc::superIOConfiguration(
    MotherboardId board, Chip chip, const std::map<SensorType, std::size_t>& nrChannels)
{
	switch (chip) {
		case Chip::IT8705F:
		case Chip::IT8712F:
		case Chip::IT8716F:
		case Chip::IT8718F:
		case Chip::IT8720F:
		case Chip::IT8726F: {
			return ITEConfigurationsA(board, chip, nrChannels);
		}
		case Chip::IT8620E:
		case Chip::IT8628E:
		case Chip::IT8631E:
		case Chip::IT8655E:
		case Chip::IT8665E:
		case Chip::IT8686E:
		case Chip::IT8688E:
		case Chip::IT8689E:
		case Chip::IT8721F:
		case Chip::IT8728F:
		case Chip::IT8771E:
		case Chip::IT8772E: {
			return ITEConfigurationsB(board, chip, nrChannels);
		}
		case Chip::IT879XE: {
			return ITEConfigurationsC(board, chip, nrChannels);
		}
		case Chip::F71858: {
			ChannelsConfiguration res;
			res.voltage.emplace_back("VCC3V", 0, 150.f, 150.f);
			res.voltage.emplace_back("VSB3V", 1, 150.f, 150.f);
			res.voltage.emplace_back("Battery", 2, 150.f, 150.f);

			addDefaultEntries(res, SensorType::voltage, nrChannels, 3);
			addDefaultEntries(res, SensorType::fan, nrChannels);

			return res;
		}
		case Chip::F71808E:
		case Chip::F71862:
		case Chip::F71869:
		case Chip::F71869A:
		case Chip::F71882:
		case Chip::F71889AD:
		case Chip::F71889ED:
		case Chip::F71889F: {
			return fintekConfiguration(board, chip, nrChannels);
		}
		case Chip::W83627EHF: {
			return winbondConfigurationEhf(board, chip, nrChannels);
		}
		case Chip::W83627DHG:
		case Chip::W83627DHGP:
		case Chip::W83667HG:
		case Chip::W83667HGB: {
			return winbondConfigurationHg(board, chip, nrChannels);
		}
		case Chip::W83627HF: {
			return {
			    {
			        {"Vcore", 0},
			        {"Voltage #2", 1, true},
			        {"Voltage #3", 2, true},
			        {"AVCC", 3, 34.f, 51.f},
			        {"Voltage #5", 4, true},
			        {"+5VSB", 5, 34.f, 51.f},
			        {"VBat", 6},
			    },
			    {
			        {"CPU", 0},
			        {"Auxiliary", 1},
			        {"System", 2},
			    },
			    {
			        {"System Fan", 0},
			        {"CPU Fan", 1},
			        {"Auxiliary Fan", 2},
			    },
			    {
			        {"Fan 1", 0},
			        {"Fan 2", 1},
			    }};
		}
		case Chip::W83627THF:
		case Chip::W83687THF:
			return {
			    {
			        {"Vcore", 0},
			        {"Voltage #2", 1, true},
			        {"Voltage #3", 2, true},
			        {"AVCC", 3, 34.f, 51.f},
			        {"Voltage #5", 4, true},
			        {"+5VSB", 5, 34.f, 51.f},
			        {"VBat", 6},
			    },
			    {
			        {"CPU", 0},
			        {"Auxiliary", 1},
			        {"System", 2},
			        {"System Fan", 0},
			        {"CPU Fan", 1},
			        {"Auxiliary Fan", 2},
			    },
			    {
			        {"System Fan", 0},
			        {"CPU Fan", 1},
			        {"Auxiliary Fan", 2},
			    }};
		case Chip::NCT6771F:
		case Chip::NCT6776F: {
			return nuvotonConfigurationF(board, chip, nrChannels);
		}
		case Chip::NCT610XD: {
			ChannelsConfiguration res{
			    {
			        {"Vcore", 0},
			        {"Voltage #0", 1, true},
			        {"AVCC", 2, 34.f, 34.f},
			        {"+3.3V", 3, 34.f, 34.f},
			        {"Voltage #1", 4, true},
			        {"Voltage #2", 5, true},
			        {"Reserved", 6, true},
			        {"3VSB", 7, 34.f, 34.f},
			        {"VBat", 8, 34.f, 34.f},
			        {"Voltage #10", 9, true},
			    },
			    {
			        {"System", 1},
			        {"CPU Core", 2},
			        {"Auxiliary", 3},
			    }};

			addDefaultEntries(res, SensorType::fan, nrChannels);
			addDefaultEntries(res, SensorType::pwm, nrChannels);

			return res;
		}
		case Chip::NCT6779D:
		case Chip::NCT6791D:
		case Chip::NCT6792D:
		case Chip::NCT6792DA:
		case Chip::NCT6793D:
		case Chip::NCT6795D:
		case Chip::NCT6796D:
		case Chip::NCT6796DR:
		case Chip::NCT6797D:
		case Chip::NCT6798D:
		case Chip::NCT6683D: {
			return nuvotonConfigurationD(board, chip, nrChannels);
		}
		case Chip::NCT6687D:
			return {
			    {
			        {"+12V", 0},
			        {"+5V", 1},
			        {"Vcore", 2},
			        {"Voltage #1", 3},
			        {"DIMM", 4},
			        {"CPU I/O", 5},
			        {"CPU SA", 6},
			        {"Voltage #2", 7},
			        {"AVCC3", 8},
			        {"VTT", 9},
			        {"VRef", 10},
			        {"VSB", 11},
			        {"AVSB", 12},
			        {"VBat", 13},
			    },
			    {
			        {"CPU", 0},
			        {"System", 1},
			        {"VRM MOS", 2},
			        {"PCH", 3},
			        {"CPU Socket", 4},
			        {"PCIe x1", 5},
			        {"M2_1", 6},
			    },
			    {
			        {"CPU Fan", 0},
			        {"Pump Fan", 1},
			        {"System Fan #1", 2},
			        {"System Fan #2", 3},
			        {"System Fan #3", 4},
			        {"System Fan #4", 5},
			        {"System Fan #5", 6},
			        {"System Fan #6", 7},
			    },
			    {
			        {"CPU Fan", 0},
			        {"Pump Fan", 1},
			        {"System Fan #1", 2},
			        {"System Fan #2", 3},
			        {"System Fan #3", 4},
			        {"System Fan #4", 5},
			        {"System Fan #5", 6},
			        {"System Fan #6", 7},
			    }};
		default: return generateDefaultConfiguration(nrChannels);
	}
}
