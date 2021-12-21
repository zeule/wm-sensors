// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_IT87XX_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_IT87XX_HXX

#include "./super_io_sensor_chip.hxx"
#include "../port.hxx"

#include <vector>

namespace wm_sensors::hardware::motherboard::lpc::superio {
	class IT87xx final: public SuperIOSensorChip {
		using base = SuperIOSensorChip;
	public:
		public:
		IT87xx(MotherboardId board, Chip chip, u16 address, u16 gpioAddress, u8 version);

		void readSIO(SensorType type, std::size_t channelMin, std::size_t count, double* values) const override;
		void writeSIO(SensorType type, std::size_t channel, double value) override;

		std::optional<u8> readGPIO(wm_sensors::stdtypes::u8 index) override;
		void writeGPIO(wm_sensors::stdtypes::u8 index, wm_sensors::stdtypes::u8 value) override;

	private:
		static std::map<SensorType, std::size_t> channelCount(Chip chip, u8 version);
		static bool hasExtReg(Chip chip);
		static float voltageGain(Chip chip);
		static u8 gpioPinCount(Chip chip);

		void readVoltages(std::size_t channelMin, std::size_t count, double* values) const;
		void readTemperatures(std::size_t channelMin, std::size_t count, double* values) const;
		void readFans(std::size_t channelMin, std::size_t count, double* values) const;
		void readPWMs(std::size_t channelMin, std::size_t count, double* values) const;

		void saveDefaultFanPwmControl(std::size_t channel);
		void restoreDefaultFanPwmControl(std::size_t channel);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(IT87xx)

		class IT87xxPort: protected SingleBankPort {
			using base = SingleBankPort;
		public:
			IT87xxPort(SingleBankAddress ports, bool validationPosible);

			u8 readByte(u8 registerIndex, bool& valid) const;
			void writeByte(u8 registerIndex, u8 value);

		private:
			bool validationPosible_;
		};

		IT87xxPort port_;
		lpc::Port gpioPort_;
		std::vector<bool> fansDisabled_;
		std::vector<u8> fanPWMControlReg_;
		const bool has16BitFanCounter_;
        const bool hasExtReg_;
        bool initialFanOutputModeEnabled_[3]; // Initial Fan Controller Main Control Register value.
        u8 initialFanPwmControl_[5]; // This will also store the 2nd control register value.
        u8 initialFanPwmControlExt_[5];
        bool restoreDefaultFanPwmControlRequired_[5];
        const float voltageGain_;
		const u8 version_;
		const u8 gpioCount_;
	};
}

#endif
