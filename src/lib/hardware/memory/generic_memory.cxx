// SPDX-License-Identifier: LGPL-3.0+

#include "./generic_memory.hxx"

#include "../../utility/string.hxx"
#include "../../utility/utility.hxx"

#include <spdlog/spdlog.h>

#include <Windows.h>

namespace {
	const std::chrono::seconds updateInterval{1};

	const char* fractionChannels[] = {"Physical", "Page file"};
	const char* dataChannels[] = {"Physical total", "Physical available", "Page file total", "Page file available"};
} // namespace

wm_sensors::hardware::memory::GenericMemory::GenericMemory()
    : base({"generic", "mem", BusType::Virtual})
{
}

wm_sensors::SensorChip::Config wm_sensors::hardware::memory::GenericMemory::config() const
{
	u32 attrs = attributes::generic_input | attributes::generic_label;

	return {{
	    {SensorType::fraction,
	     {{
	         attrs, // physical load
	         attrs, // page file load
	     }}},
	    {SensorType::data,
	     {{
	         attrs, // total phys.
	         attrs, // avail. phys.
	         attrs, // total page file
	         attrs, // avail. page file
	     }}},
	}};
}

int wm_sensors::hardware::memory::GenericMemory::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	switch (type) {
		case SensorType::fraction:
			update();
			switch (channel) {
				case 0: val = status_.load; return 0;
				case 1: val = 1.0 - static_cast<double>(status_.availPageFile) / static_cast<double>(status_.totalPageFile); return 0;
				default: return -EOPNOTSUPP;
			}
		case SensorType::data:
			update();
			switch (channel) {
				case 0: val = static_cast<double>(status_.totalPhys); return 0;
				case 1: val = static_cast<double>(status_.availPhys); return 0;
				case 2: val = static_cast<double>(status_.totalPageFile); return 0;
				case 3: val = static_cast<double>(status_.availPageFile); return 0;
				default: return -EOPNOTSUPP;
			}
		default: return base::read(type, attr, channel, val);
	}
}

int wm_sensors::hardware::memory::GenericMemory::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	switch (type) {
		case SensorType::fraction:
			if (channel < utility::array_size(fractionChannels)) {
				str = fractionChannels[channel];
				return 0;
			}
			break;
		case SensorType::data:
			if (channel < utility::array_size(dataChannels)) {
				str = dataChannels[channel];
				return 0;
			}
			break;
		default: break;
	}
	return base::read(type, attr, channel, str);
}

void wm_sensors::hardware::memory::GenericMemory::update() const
{
	const auto now = std::chrono::steady_clock::now();
	if (now > lastUpdate_ + updateInterval) {
		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof(ms);
		if (::GlobalMemoryStatusEx(&ms)) {
			lastUpdate_ = now;
			status_.load = static_cast<float>(ms.dwMemoryLoad) / 100.f;
			status_.totalPhys = ms.ullTotalPhys;
			status_.availPhys = ms.ullAvailPhys;
			status_.totalPageFile = ms.ullTotalPageFile;
			status_.availPageFile = ms.ullAvailPageFile;
		} else {
			spdlog::error("GlobalMemoryStatusEx() failed: {}", windowsLastErrorMessage());
		}
	}
}
