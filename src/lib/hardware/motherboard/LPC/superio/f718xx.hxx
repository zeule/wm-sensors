// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_F718XX_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_F718XX_HXX

#include "./super_io_sensor_chip.hxx"
#include "../port.hxx"

#include <array>

namespace wm_sensors::hardware::motherboard::lpc::superio {
	class F718xx final: public SuperIOSensorChip {
		using base = SuperIOSensorChip;
	public:
		F718xx(MotherboardId board, Chip chip, u16 address);

		void readSIO(SensorType type, std::size_t channelMin, std::size_t count, double* values) const override;
		void writeSIO(SensorType type, std::size_t channel, double value) override;

	private:
		void readVoltages(std::size_t channelMin, std::size_t count, double* values) const;
		void readTemperatures(std::size_t channelMin, std::size_t count, double* values) const;
		void readFans(std::size_t channelMin, std::size_t count, double* values) const;
		void readPWMs(std::size_t channelMin, std::size_t count, double* values) const;

		void restoreDefaultFanPwmControl(std::size_t channel);
		void saveDefaultFanPwmControl(std::size_t channel);

		static std::map<SensorType, std::size_t> channelCount(Chip chip);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(F718xx)

		SingleBankPort port_;
		std::array<u8, 4> initialFanPWMValues_;
		bool restoreDefaultFanPWMValuesRequired_[4];
	};
}

#endif
