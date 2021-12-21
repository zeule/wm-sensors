// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_USBHID_CHIP_HXX
#define WM_SENSORS_LIB_IMPL_USBHID_CHIP_HXX

#include "../../sensor.hxx"
#include "../../utility/hidapi++/hidapi.hxx"

#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <thread>
#include <vector>

namespace wm_sensors::impl {
	std::vector<std::unique_ptr<SensorChip>> enumerate(
	    std::function<std::unique_ptr<SensorChip>(const hidapi::device_info& di)> creator, u16 vendorId);


	class HidChipImpl {
	public:
		HidChipImpl(hidapi::device&& dev);

		hidapi::device& device() {
			return device_;
		}

	private:
		hidapi::device device_;
	};

	class HidChipImplAutoRead: public HidChipImpl {
	public:
		virtual ~HidChipImplAutoRead();
		void acknowledgeDataAccess();
	protected:
		HidChipImplAutoRead(hidapi::device&& dev, std::chrono::milliseconds readInterval);

		virtual bool readData() = 0;

	private:
		HidChipImplAutoRead(const HidChipImplAutoRead&) = delete;
		HidChipImplAutoRead& operator=(const HidChipImplAutoRead&) = delete;

		void run();
		void stop();

		void readThread();

		std::thread readThread_;
		std::chrono::milliseconds readInterval_;
		std::chrono::steady_clock::time_point lastRead_;
		bool running_;
	};
}

#endif
