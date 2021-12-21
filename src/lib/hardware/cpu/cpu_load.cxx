// SPDX-License-Identifier: LGPL-3.0+

#include "./cpu_load.hxx"

#include <Windows.h>
#include <winternl.h>

#include <cmath>
#include <stdexcept>

namespace {

	class WinAPI {
	public:
		static WinAPI& instance()
		{
			static WinAPI instance;
			return instance;
		}

		decltype(&NtQuerySystemInformation)
		ntQuerySystemInformation() const
		{
			return NtQuerySystemInformation_;
		}

	private:
		WinAPI()
		{
			HMODULE h = ::LoadLibrary(L"Ntdll.dll");
			if (!h) {
				throw std::logic_error("Could not load Ntdll.dl");
			}
#pragma warning (push)
#pragma warning (disable: 4191)
			NtQuerySystemInformation_ =
			    reinterpret_cast<decltype(&NtQuerySystemInformation)>(::GetProcAddress(h, "NtQuerySystemInformation"));
#pragma warning (pop)
		}
		decltype(&NtQuerySystemInformation) NtQuerySystemInformation_;
	};
}

wm_sensors::hardware::cpu::CpuLoad::CpuLoad(const std::vector<std::vector<CPUIDData>>& cpuid)
    : cpuid_{cpuid}
    , coreLoads_(cpuid.size(), 0.f)
    , totalLoad_{0.f}
{
	try {
		getTimes(idleTimes_, totalTimes_);
	} catch (...) {
		isAvailable_ = false;
	}

	isAvailable_ = !idleTimes_.empty();
}

void wm_sensors::hardware::cpu::CpuLoad::update()
{
	if (idleTimes_.empty())
		return;

	std::vector<s64> newIdleTimes, newTotalTimes;
	if (!getTimes(newIdleTimes, newTotalTimes))
		return;

	if (newIdleTimes.empty())
		return;

	for (std::size_t i = 0; i < std::min(newTotalTimes.size(), totalTimes_.size()); i++) {
		if (newTotalTimes[i] - totalTimes_[i] < 100000)
			return;
	}

	float total = 0;
	int count = 0;
	for (std::size_t i = 0; i < cpuid_.size(); i++) {
		float value = 0;
		for (std::size_t j = 0; j < cpuid_[i].size(); j++) {
			auto index = cpuid_[i][j].thread();
			if (index < newIdleTimes.size() && index < totalTimes_.size()) {
				float idle =
				    static_cast<float>(newIdleTimes[index] - idleTimes_[index]) / static_cast<float>(newTotalTimes[index] - totalTimes_[index]);
				value += idle;
				total += idle;
				count++;
			}
		}

		value = 1.0f - value / static_cast<float>(cpuid_[i].size());
		value = value < 0 ? 0 : value;
		coreLoads_[i] = value; // * 100;
	}

	if (count > 0) {
		total = 1.0f - total / static_cast<float>(count);
		total = total < 0 ? 0 : total;
	} else {
		total = 0;
	}

	totalLoad_ = total; // *100;
	std::swap(totalTimes_, newTotalTimes);
	std::swap(idleTimes_, newIdleTimes);
}

bool wm_sensors::hardware::cpu::CpuLoad::getTimes(std::vector<s64>& idle, std::vector<s64>& total)
{
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION information[64];
	DWORD outLength;
	auto r = WinAPI::instance().ntQuerySystemInformation()(
	    SystemProcessorPerformanceInformation, information, sizeof(information), &outLength);
	
	if (r != 0) {
		idle.clear();
		total.clear();
		return false;
	}

	idle.resize(outLength/sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
	total.resize(outLength / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

	for (std::size_t i = 0; i < idle.size(); i++) {
		idle[i] = information[i].IdleTime.QuadPart;
		total[i] = information[i].KernelTime.QuadPart + information[i].UserTime.QuadPart;
	}

	return true;
}
