// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_NCT67XX_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_SUPERIO_NCT67XX_HXX

#include "../port.hxx"
#include "./super_io_sensor_chip.hxx"

#include <vector>

namespace wm_sensors::hardware::motherboard::lpc::superio {
	class Nct67xx final: public SuperIOSensorChip {
		using base = SuperIOSensorChip;

	public:
		Nct67xx(MotherboardId board, Chip chip, u8 revision, u16 address, const SingleBankPort& port);

		void readSIO(SensorType type, std::size_t channelMin, std::size_t count, double* values) const override;
		void writeSIO(SensorType type, std::size_t channel, double value) override;

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Nct67xx)

		static std::map<SensorType, std::size_t> channelCount(Chip chip, u8 version);
		static AddressWithBank portAddress(Chip chip, u16 address);

		void readVoltages(std::size_t channelMin, std::size_t count, double* values) const;
		void readTemperatures(std::size_t channelMin, std::size_t count, double* values) const;
		void readFans(std::size_t channelMin, std::size_t count, double* values) const;
		void readPWMs(std::size_t channelMin, std::size_t count, double* values) const;
		void writePWM(std::size_t channel, double value);

		void saveDefaultFanPwmControl(std::size_t channel);
		void restoreDefaultFanPwmControl(std::size_t channel);

		// one-time setup functions
		bool isNuvotonVendor();
		void setupChipParameters(Chip chip);

		bool disableIOSpaceLock();

		bool beginRead() override;

		enum class Source : u8
		{
			None = 0,
			SYSTIN = 1,
			CPUTIN = 2,
			AUXTIN = 3,

			Nct6771F_PECI_0 = 5,

			Nct6776F_PECI_0 = 12,

			Nct67Xxd_AUXTIN0 = 3,
			Nct67Xxd_AUXTIN1 = 4,
			Nct67Xxd_AUXTIN2 = 5,
			Nct67Xxd_AUXTIN3 = 6,
			Nct67Xxd_AUXTIN4 = 7,
			Nct67Xxd_SMBUSMASTER0 = 8,
			Nct67Xxd_SMBUSMASTER1 = 9,
			Nct67Xxd_PECI_0 = 16,
			Nct67Xxd_PECI_1 = 17,
			Nct67Xxd_PCH_CHIP_CPU_MAX_TEMP = 18,
			Nct67Xxd_PCH_CHIP_TEMP = 19,
			Nct67Xxd_PCH_CPU_TEMP = 20,
			Nct67Xxd_PCH_MCH_TEMP = 21,
			Nct67Xxd_AGENT0_DIMM0 = 22,
			Nct67Xxd_AGENT0_DIMM1 = 23,
			Nct67Xxd_AGENT1_DIMM0 = 24,
			Nct67Xxd_AGENT1_DIMM1 = 25,
			Nct67Xxd_BYTE_TEMP0 = 26,
			Nct67Xxd_BYTE_TEMP1 = 27,
			Nct67Xxd_PECI_0_CAL = 28,
			Nct67Xxd_PECI_1_CAL = 29,
			Nct67Xxd_VIRTUAL_TEMP = 31,

			Nct610X_PECI_0 = 12
		};

		struct TempSrcDef {
			TempSrcDef(
			    Source source,
			    u16 reg,
			    u16 halfReg = 0,
			    int halfBit = -1,
			    u16 sourceReg = 0,
			    std::optional<u16> altReg = {});

			std::optional<u16> alternateReg;
			int halfBit;
			u16 reg;
			u16 halfReg;
			u16 sourceReg;
			Source source;
		};

		PortWithBanks port_;

		struct FanControl {
			u16 reg;
		};
		std::vector<u16> fanCountRegister_; // TODO join in a single struct
		std::vector<u16> fanRpmRegister_;
		std::vector<u8> initialFanControlMode_;
		std::vector<u8> initialFanPwmCommand_;
		std::vector<bool> restoreDefaultFanControlRequired_;

		bool isNuvotonVendor_;
		SingleBankPort lpcPort_;
		int maxFanCount_;
		int minFanCount_;
		int minFanRpm_;

		u8 revision_;
		std::vector<TempSrcDef> temperaturesSource_;
		u16 vBatMonitorControlRegister_;
		std::vector<u16> voltageRegisters_;
		u16 voltageVBatRegister_;
		std::vector<u16> fanControlModeReg_;
		std::vector<u16> fanPWMCommandReg_;
		std::vector<u16> fanPWMOutReg_;
		std::vector<u16> fanPWMRequestReg_;
	};
} // namespace wm_sensors::hardware::motherboard::lpc::superio

#endif
