// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_HARDWARE_MOTHERPOARD_LPC_LPCIO_HXX
#define WM_SENSORS_LIB_HARDWARE_MOTHERPOARD_LPC_LPCIO_HXX

#include "../identification.hxx"

#include <memory>
#include <vector>

namespace wm_sensors::hardware::motherboard::lpc {
	class SingleBankPort;
	class SuperIOSensorChip;

	class LpcIo {
		// private readonly StringBuilder _report = new StringBuilder();
		// private readonly List<ISuperIO> _superIOs = new List<ISuperIO>();

	public:
		LpcIo(MotherboardId board);
		~LpcIo();

		// public ISuperIO[] SuperIO => _superIOs.ToArray();

		std::vector<std::unique_ptr<SuperIOSensorChip>>& chips() 
		{
			return superioChips_;
		}

	private:
		void detect(MotherboardId board);
		bool detectIT87(MotherboardId board, const SingleBankPort& port);
		bool detectSmsc(MotherboardId board, const SingleBankPort& port);
		bool detectWinbondFintek(MotherboardId board, const SingleBankPort& port);

		void reportUnknownChip(const SingleBankPort& port, std::string_view type, int chip);

		std::vector<std::unique_ptr<SuperIOSensorChip>> superioChips_;
	};
} // namespace wm_sensors::hardware::motherboard::lpc

#endif
