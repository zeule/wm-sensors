// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_LPC_SUPER_IO_SENSOR_CHIP_HXX
#define WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_LPC_SUPER_IO_SENSOR_CHIP_HXX

#include "../identification.hxx"
#include "../../identification.hxx"
#include "../../../../sensor.hxx"

#include <optional>
#include <map>
#include <string>
#include <vector>


namespace wm_sensors::hardware::motherboard::lpc {

	struct ChannelConfig {
		ChannelConfig(std::string lbl, std::size_t src, bool hide = false);

		std::string label;
		std::size_t sourceIndex;
		bool hidden;
	};

	struct VoltageChannelConfig: public ChannelConfig {
		VoltageChannelConfig(std::string lbl, std::size_t src, float ri, float rf, float vf = 0, bool hide = false);
		VoltageChannelConfig(std::string lbl, std::size_t src, bool hide = false);

		float ri;
		float rf;
		float vf;
	};

	struct ChannelsConfiguration {
		std::vector<VoltageChannelConfig> voltage;
		std::vector<ChannelConfig> temperature;
		std::vector<ChannelConfig> fan;
		std::vector<ChannelConfig> pwm;
		std::wstring mutexName;
	};

	class SuperIOSensorChip: public SensorChip {
		using base = SensorChip;
	public:
		virtual ~SuperIOSensorChip();

		lpc::Chip chip() const;

		virtual void writeGPIO(u8 index, u8 value);
		virtual std::optional<u8> readGPIO(u8 index);

		virtual void readSIO(SensorType type, std::size_t channelMin, std::size_t count, double* values) const = 0;
		virtual void writeSIO(SensorType type, std::size_t channel, double value) = 0;

		SensorChip::Config config() const final override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const final override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const final override;
		int write(SensorType type, u32 attr, std::size_t channel, double val) final override;

	protected:
		SuperIOSensorChip(
		    MotherboardId board, lpc::Chip chip, u16 address, const std::map<SensorType, std::size_t>& nrChannels);

		std::size_t nrChannels(SensorType type) const
		{
			return type < SensorType::max ? nrChannels_[static_cast<std::size_t>(type)] : 0;
		}

		// default implementation acquires the global ISA mutex
		virtual bool beginRead();
		virtual void endRead();

		// default implementation calls the -read counterpart
		virtual bool beginWrite();
		virtual void endWrite();

		void validateConfig();

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(SuperIOSensorChip)

		ChannelsConfiguration config_;
		lpc::Chip chip_;
		u16 address_;
		std::size_t nrChannels_[static_cast<unsigned>(SensorType::max)];
	};
}

#endif
