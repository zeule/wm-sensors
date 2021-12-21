// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_W836XX_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_W836XX_HXX

#include "../port.hxx"
#include "./super_io_sensor_chip.hxx"

#include <vector>

namespace wm_sensors::hardware::motherboard::lpc::superio {
	class W836xx final: public SuperIOSensorChip {
		using base = SuperIOSensorChip;

	public:
		W836xx(MotherboardId board, Chip chip, u8 revision, u16 address);

		void readSIO(SensorType type, std::size_t channelMin, std::size_t count, double* values) const override;
		void writeSIO(SensorType type, std::size_t channel, double value) override;

	private:
		static std::map<SensorType, std::size_t> channelCount(Chip chip, u8 revision);

		void readVoltages(std::size_t channelMin, std::size_t count, double* values) const;
		void readTemperatures(std::size_t channelMin, std::size_t count, double* values) const;
		void readFans(std::size_t channelMin, std::size_t count, double* values) const;
		void readPWMs(std::size_t channelMin, std::size_t count, double* values) const;
		void writePWM(std::size_t channel, double value);

		void saveDefaultFanPwmControl(std::size_t channel);
		void restoreDefaultFanPwmControl(std::size_t channel);

		bool isWinbondVendor();

		DELETE_COPY_CTOR_AND_ASSIGNMENT(W836xx)

		PortWithBanks port_;
		std::vector<bool> peciTemperature_;
		std::vector<u8> voltageBank_;
		std::vector<u8> voltageRegister_;

		// Added to control fans.
		std::vector<u8> fanPwmRegister_;
		std::vector<u8> fanPrimaryControlModeRegister_;
		std::vector<u8> fanPrimaryControlValue_;
		std::vector<u8> fanSecondaryControlModeRegister_;
		std::vector<u8> fanSecondaryControlValue_;
		std::vector<u8> fanTertiaryControlModeRegister_;
		std::vector<u8> fanTertiaryControlValue_;

		std::vector<u8> initialFanControlValue_;
		std::vector<u8> initialFanSecondaryControlValue_;
		std::vector<u8> initialFanTertiaryControlValue_;

		std::vector<bool> restoreDefaultFanPwmControlRequired_;

		float voltageGain_ = 0.008f;
		u8 revision_;
	};
} // namespace wm_sensors::hardware::motherboard::lpc::superio

#endif
