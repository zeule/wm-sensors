// SPDX-License-Identifier: LGPL-3.0+

#include "./usbhid_chip.hxx"

#include <utility>

std::vector<std::unique_ptr<wm_sensors::SensorChip>>
wm_sensors::impl::enumerate(
	std::function<std::unique_ptr<SensorChip> (const hidapi::device_info&)> creator, u16 vendorId)
{
	std::vector<std::unique_ptr<wm_sensors::SensorChip>> res;
	for (const auto& di: hidapi::hid::instance().enumerate(vendorId)) {
		auto dev = creator(di);
		if (dev) {
			res.push_back(std::move(dev));
		}
	}
	return res;
}

wm_sensors::impl::HidChipImpl::HidChipImpl(hidapi::device&& dev)
	: device_{std::move(dev)}
{
}

wm_sensors::impl::HidChipImplAutoRead::HidChipImplAutoRead(
    hidapi::device&& dev, std::chrono::milliseconds readInterval)
	: HidChipImpl{std::move(dev)}
	, readInterval_{readInterval}
	, running_{false}
{
}

wm_sensors::impl::HidChipImplAutoRead::~HidChipImplAutoRead()
{
	stop();
	if (readThread_.joinable()) {
		readThread_.join();
	}
}

void wm_sensors::impl::HidChipImplAutoRead::run()
{
	readThread_ = std::thread([this](){
		this->readThread();
	});
}

void wm_sensors::impl::HidChipImplAutoRead::stop()
{
	running_ = false;
}

void wm_sensors::impl::HidChipImplAutoRead::acknowledgeDataAccess()
{
	if (!running_) {
		run();
	}
	lastRead_ = std::chrono::steady_clock::now();
}

void wm_sensors::impl::HidChipImplAutoRead::readThread()
{
	std::chrono::steady_clock::time_point lastDataAcquired;
	do {
		if (readData()) {
			lastDataAcquired = std::chrono::steady_clock::now();
		}
		std::this_thread::sleep_for(readInterval_);
	} while (running_);
}
