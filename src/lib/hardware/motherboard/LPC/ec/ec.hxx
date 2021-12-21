// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_LPC_EC_EC_HXX
#define WM_SENSORS_LIB_HARDWARE_LPC_EC_EC_HXX

#include "../../../../utility/macro.hxx"
#include "../../../../wm_sensor_types.hxx"

#include <memory>
#include <stdexcept>

namespace wm_sensors::hardware::motherboard::lpc::ec {
	class EC {
	public:
		static EC& first();

		enum class Command : u8
		{
			Read = 0x80,         // RD_EC
			Write = 0x81,        // WR_EC
			BurstEnable = 0x82,  // BE_EC
			BurstDisable = 0x83, // BD_EC
			Query = 0x84         // QR_EC
		};

		u8 read(u8 addr);
		void write(u8 addr, u8 val);
		void transaction(Command command, const u8* wdata, unsigned wdata_len, u8* rdata, unsigned rdata_len);

		EC(u16 commandPort, u16 dataPort);
		~EC();

		// exceptions

		class TransactionFailed: public std::runtime_error {
		public:
			TransactionFailed();
		};

	private:
		bool waitRead();
		bool waitWrite();

		DELETE_COPY_CTOR_AND_ASSIGNMENT(EC)

		struct State;
		std::unique_ptr<State> state_;
	};

} // namespace wm_sensors::hardware::motherboard::lpc::ec

#endif
