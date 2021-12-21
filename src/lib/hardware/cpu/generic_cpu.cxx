// SPDX-License-Identifier: LGPL-3.0+

#include "./generic_cpu.hxx"

#include "../../impl/group_affinity.hxx"
#include "../../utility/string.hxx"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <Windows.h>

#include <intrin.h>
#include <powerbase.h>

#include <cmath>
#include <limits>

#if 0
static Identifier CreateIdentifier(Vendor vendor, int processorIndex)
{
	string s;
	switch (vendor) {
		case Vendor.AMD: s = "amdcpu"; break;
		case Vendor.Intel: s = "intelcpu"; break;
		default: s = "genericcpu"; break;
	}

	return new Identifier(s, processorIndex.ToString(CultureInfo.InvariantCulture));
}

#endif

namespace {
	const auto frequencyUpdatePeriod = std::chrono::seconds(1);
	struct PROCESSOR_POWER_INFORMATION {
		ULONG Number;
		ULONG MaxMhz;
		ULONG CurrentMhz;
		ULONG MhzLimit;
		ULONG MaxIdleState;
		ULONG CurrentIdleState;
	};
	const LONG STATUS_ACCESS_DENIED = 0xC0000022;
	const LONG STATUS_BUFFER_TOO_SMALL = 0xC0000023;
} // namespace

wm_sensors::hardware::cpu::GenericCPU::GenericCPU(unsigned processorIndex, CpuIdDataArray&& cpuId)
    : base{{cpuId[0][0].name(), "cpu", BusType::ISA}}
    , cpuIdData_{std::move(cpuId)}
    , coreCount_{cpuIdData_.size()}
    , logicalCoreCount_{coreCount_ * cpuIdData_.front().size()}
    , vendor_{cpuIdData_[0][0].vendor()}
    , family_{cpuIdData_[0][0].family()}
    , model_{cpuIdData_[0][0].model()}
    , packageType_{cpuIdData_[0][0].pkgType()}
    , stepping_{cpuIdData_[0][0].stepping()}
    , index_{processorIndex}
    , cpuLoad_{cpuIdData_}
    , hasModelSpecificRegisters_{cpuIdData_[0][0].data().size() > 1 && (cpuIdData_[0][0].data()[1][3] & 0x20) != 0}
    // check if processor has MSRs
    , hasTimeStampCounter_{cpuIdData_[0][0].data().size() > 1 && (cpuIdData_[0][0].data()[1][3] & 0x10) != 0}
    // check if processor has a TSC
    , isInvariantTimeStampCounter_{cpuIdData_[0][0].data().size() > 7 && (cpuIdData_[0][0].data()[7][3] & 0x100) != 0}
    , coreFrequencies_(coreCount_, 0)
    // check if processor supports an invariant TSC
    , lastTime_{0}
{
	coreLabels_.reserve(coreCount_);
	for (std::size_t i = 0; i < coreCount_; ++i) {
		coreLabels_.push_back(coreString(i));
	}

	if (cpuLoad_.available()) {
		coreLoads_.resize(coreCount_ + 1);
	}

	if (hasTimeStampCounter_) {
		impl::ThreadGroupAffinityGuard guard{cpuIdData_[0][0].affinity()};
		estimateTimeStampCounterFrequency(
		    estimatedTimeStampCounterFrequency_, estimatedTimeStampCounterFrequencyError_);
	} else {
		estimatedTimeStampCounterFrequency_ = 0;
	}

	timeStampCounterFrequency_ = estimatedTimeStampCounterFrequency_;
}

void wm_sensors::hardware::cpu::GenericCPU::update() const
{
	if (hasTimeStampCounter_ && isInvariantTimeStampCounter_) {
		LARGE_INTEGER freq, firstTime, time;
		u64 timeStampCount;
		{
			// make sure always the same thread is used
			impl::ThreadGroupAffinityGuard guard{cpuIdData_[0][0].affinity()};
			// read time before and after getting the TSC to estimate the error
			::QueryPerformanceCounter(&firstTime);
			timeStampCount = __rdtsc();
			::QueryPerformanceCounter(&time);
		}
		::QueryPerformanceFrequency(&freq);
		double delta = static_cast<double>(time.QuadPart - lastTime_) / static_cast<double>(freq.QuadPart);
		double error = static_cast<double>(time.QuadPart - firstTime.QuadPart) / static_cast<double>(freq.QuadPart);

		// only use data if they are measured accurate enough (max 0.1ms delay)
		if (error < 0.0001) {
			// ignore the first reading because there are no initial values
			// ignore readings with too large or too small time window
			if (lastTime_ != 0 && delta > 0.5 && delta < 2) {
				// update the TSC frequency with the new value
				timeStampCounterFrequency_ = static_cast<double>(timeStampCount - lastTimeStampCount_) / (1e6 * delta);
			}

			lastTimeStampCount_ = timeStampCount;
			lastTime_ = time.QuadPart;
		}
	}

	if (cpuLoad_.available()) {
		cpuLoad_.update();
		coreLoads_[0] = cpuLoad_.totalLoad();
		for (std::size_t i = 1; i < coreLoads_.size(); i++) {
			coreLoads_[i] = cpuLoad_.coreLoad(i - 1);
		}
	}
}

wm_sensors::SensorChip::Config wm_sensors::hardware::cpu::GenericCPU::config() const
{
	Config res;
	if (!coreLoads_.empty()) {
		res.sensors[SensorType::load].channelAttributes.resize(
		    coreLoads_.size(), attributes::load_input | attributes::load_label);
	}

	res.sensors[SensorType::frequency].channelAttributes.resize(
	    coreFrequencies_.size(), attributes::frequency_input | attributes::frequency_label);
	return res;
}

int wm_sensors::hardware::cpu::GenericCPU::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	switch (type) {
		case SensorType::frequency:
			switch (attr) {
				case attributes::frequency_input:
					if (channel < coreFrequencies_.size()) {
						maybeUpdateFrequencies();
						val = coreFrequencies_[channel] * 1e6;
						return 0;
					}
				default: break;
			}
		case SensorType::load:
			switch (attr) {
				case attributes::load_input:
					if (channel < coreLoads_.size()) {
						updateLoads();
						val = coreLoads_[channel];
						return 0;
					}
					break;
				default: break;
			}
		default: break;
	}
	return base::read(type, attr, channel, val);
}

int wm_sensors::hardware::cpu::GenericCPU::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	switch (type) {
		case SensorType::frequency:
			switch (attr) {
				case attributes::frequency_label:
					if (channel < coreFrequencies_.size()) {
						str = coreLabels_[channel];
						return 0;
					}
				default: break;
			}
		case SensorType::load:
			switch (attr) {
				case attributes::load_label:
					if (channel < coreLoads_.size()) {
						str = channel == 0 ? std::string_view("CPU Total") : std::string_view(coreLabels_[channel - 1]);
						return 0;
					}
					break;
				default: break;
			}
		default: break;
	}
	return base::read(type, attr, channel, str);
}

std::string wm_sensors::hardware::cpu::GenericCPU::coreString(std::size_t i) const
{
	return coreCount_ == 1 ? std::string{"CPU Core"} : fmt::format("CPU Core #{0}", i);
}

void wm_sensors::hardware::cpu::GenericCPU::estimateTimeStampCounterFrequency(double& frequency, double& error)
{
	// preload the function
	double f, e;
	estimateTimeStampCounterFrequency(0, f, e);
	estimateTimeStampCounterFrequency(0, f, e);

	// estimate the frequency
	error = std::numeric_limits<double>::max();
	frequency = 0;
	for (int i = 0; i < 5; i++) {
		estimateTimeStampCounterFrequency(0.025, f, e);
		if (e < error) {
			error = e;
			frequency = f;
		}

		if (error < 1e-4)
			break;
	}
}

void wm_sensors::hardware::cpu::GenericCPU::estimateTimeStampCounterFrequency(
    double timeWindow, double& frequency, double& error)
{
	LARGE_INTEGER freq, counter;
	::QueryPerformanceFrequency(&freq);
	s64 ticks = static_cast<s64>(timeWindow * static_cast<double>(freq.QuadPart));

	::QueryPerformanceCounter(&counter);
	s64 timeBegin = counter.QuadPart + static_cast<s64>(std::ceil(0.001 * static_cast<double>(ticks)));
	s64 timeEnd = timeBegin + ticks;

	while (counter.QuadPart < timeBegin) {
		::QueryPerformanceCounter(&counter);
	}

	u64 countBegin = __rdtsc();
	LARGE_INTEGER afterBegin;
	::QueryPerformanceCounter(&afterBegin);

	while (counter.QuadPart < timeEnd) {
		::QueryPerformanceCounter(&counter);
	}

	u64 countEnd = __rdtsc();
	LARGE_INTEGER afterEnd;
	::QueryPerformanceCounter(&afterEnd);

	double delta = static_cast<double>(timeEnd - timeBegin);
	frequency = 1e-6 * (static_cast<double>(countEnd - countBegin) * static_cast<double>(freq.QuadPart)) / delta;

	double beginError = static_cast<double>(afterBegin.QuadPart - timeBegin) / delta;
	double endError = static_cast<double>(afterEnd.QuadPart - timeEnd) / delta;
	error = beginError + endError;
}

void wm_sensors::hardware::cpu::GenericCPU::updateLoads() const
{
	LARGE_INTEGER freq, time;
	::QueryPerformanceFrequency(&freq);
	::QueryPerformanceCounter(&time);
	if (time.QuadPart - lastTime_ > freq.QuadPart) {
		this->update();
	}
}

void wm_sensors::hardware::cpu::GenericCPU::maybeUpdateFrequencies() const
{
	const auto now = std::chrono::steady_clock::now();
	if (now - lastUpdateFreq_ > frequencyUpdatePeriod) {
		std::vector<PROCESSOR_POWER_INFORMATION> buf(logicalCoreCount_);
		auto er = ::CallNtPowerInformation(
		    ProcessorInformation, nullptr, 0, buf.data(), static_cast<ULONG>(buf.size() * sizeof(PROCESSOR_POWER_INFORMATION)));
		if (er == 0) {
			for (std::size_t i = 0; i < coreCount_; ++i) {
				coreFrequencies_[i] = buf[i * logicalCoreCount_ / coreCount_].CurrentMhz;
			}
			lastUpdateFreq_ = now;
		} else if (er == STATUS_BUFFER_TOO_SMALL) {
			spdlog::error("CallNtPowerInformation() error: buffer too small");
		} else if (er == STATUS_ACCESS_DENIED) {
			spdlog::error("CallNtPowerInformation() error: access denied");
		}
	}
}
