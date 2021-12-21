// SPDX-License-Identifier: LGPL-3.0+

#include "./amd10_cpu.hxx"

#include "../../impl/ring0.hxx"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <thread>

namespace {
	using namespace wm_sensors::stdtypes;
	const u32 CLOCK_POWER_TIMING_CONTROL_0_REGISTER = 0xD4;
	const u32 COFVID_STATUS = 0xC0010071;
	const u32 CSTATES_IO_PORT = 0xCD6;
	const u32 SMU_REPORTED_TEMP_CTRL_OFFSET = 0xD8200CA4;
	const u32 HWCR = 0xC0010015;
	const u8 MISCELLANEOUS_CONTROL_FUNCTION = 3;
	const u32 P_STATE_0 = 0xC0010064;
	const u32 PERF_CTL_0 = 0xC0010000;
	const u32 PERF_CTR_0 = 0xC0010004;
	const u32 REPORTED_TEMPERATURE_CONTROL_REGISTER = 0xA4;

	const u16 FAMILY_10H_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1203;
	const u16 FAMILY_11H_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1303;
	const u16 FAMILY_12H_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1703;
	const u16 FAMILY_14H_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1703;
	const u16 FAMILY_15H_MODEL_00_MISC_CONTROL_DEVICE_ID = 0x1603;
	const u16 FAMILY_15H_MODEL_10_MISC_CONTROL_DEVICE_ID = 0x1403;
	const u16 FAMILY_15H_MODEL_30_MISC_CONTROL_DEVICE_ID = 0x141D;
	const u16 FAMILY_15H_MODEL_60_MISC_CONTROL_DEVICE_ID = 0x1573;
	const u16 FAMILY_15H_MODEL_70_MISC_CONTROL_DEVICE_ID = 0x15B3;
	const u16 FAMILY_16H_MODEL_00_MISC_CONTROL_DEVICE_ID = 0x1533;
	const u16 FAMILY_16H_MODEL_30_MISC_CONTROL_DEVICE_ID = 0x1583;

	const std::chrono::seconds updateInterval{1};
} // namespace

wm_sensors::hardware::cpu::Amd10Cpu::Amd10Cpu(unsigned processorIndex, CpuIdDataArray&& cpuId)
    : base{processorIndex, std::move(cpuId)}
    , baseChannels_{base::config().nrChannels()}
    , cStatesIoOffset_{0}
{
	
	u16 miscellaneousControlDeviceId;
	
	isSvi2_ = (family() == 0x15 && model() >= 0x10) || family() == 0x16;

	switch (family()) {
		case 0x10: {
			miscellaneousControlDeviceId = FAMILY_10H_MISCELLANEOUS_CONTROL_DEVICE_ID;
			break;
		}
		case 0x11: {
			miscellaneousControlDeviceId = FAMILY_11H_MISCELLANEOUS_CONTROL_DEVICE_ID;
			break;
		}
		case 0x12: {
			miscellaneousControlDeviceId = FAMILY_12H_MISCELLANEOUS_CONTROL_DEVICE_ID;
			break;
		}
		case 0x14: {
			miscellaneousControlDeviceId = FAMILY_14H_MISCELLANEOUS_CONTROL_DEVICE_ID;
			break;
		}
		case 0x15: {
			switch (model() & 0xF0) {
				case 0x00: {
					miscellaneousControlDeviceId = FAMILY_15H_MODEL_00_MISC_CONTROL_DEVICE_ID;
					break;
				}
				case 0x10: {
					miscellaneousControlDeviceId = FAMILY_15H_MODEL_10_MISC_CONTROL_DEVICE_ID;
					break;
				}
				case 0x30: {
					miscellaneousControlDeviceId = FAMILY_15H_MODEL_30_MISC_CONTROL_DEVICE_ID;
					break;
				}
				case 0x70: {
					miscellaneousControlDeviceId = FAMILY_15H_MODEL_70_MISC_CONTROL_DEVICE_ID;
					hasSmuTemperatureRegister_ = true;
					break;
				}
				case 0x60: {
					miscellaneousControlDeviceId = FAMILY_15H_MODEL_60_MISC_CONTROL_DEVICE_ID;
					hasSmuTemperatureRegister_ = true;
					break;
				}
				default: {
					miscellaneousControlDeviceId = 0;
					break;
				}
			}
			break;
		}
		case 0x16: {
			switch (model() & 0xF0) {
				case 0x00: {
					miscellaneousControlDeviceId = FAMILY_16H_MODEL_00_MISC_CONTROL_DEVICE_ID;
					break;
				}
				case 0x30: {
					miscellaneousControlDeviceId = FAMILY_16H_MODEL_30_MISC_CONTROL_DEVICE_ID;
					break;
				}
				default: {
					miscellaneousControlDeviceId = 0;
					break;
				}
			}
			break;
		}
		default: {
			miscellaneousControlDeviceId = 0;
			break;
		}
	}

	// get the pci address for the Miscellaneous Control registers
	miscellaneousControlAddress_ = PCIAddress(MISCELLANEOUS_CONTROL_FUNCTION, miscellaneousControlDeviceId);
	if (hasTimeStampCounter()) {
		coreClock_.resize(coreCount());
	}

	impl::Ring0& ring0 = impl::Ring0::instance();

	bool corePerformanceBoostSupport = (cpu0IdData().safeExtData(7, 3, 0) & (1 << 9)) > 0;

	{
		// set affinity to the first thread for all frequency estimations
		wm_sensors::impl::ThreadGroupAffinityGuard lock{cpu0IdData().affinity()};

		// disable core performance boost
		impl::Ring0::MSRValue hwcr;
		ring0.readMSR(HWCR, hwcr);
		if (corePerformanceBoostSupport)
			ring0.writeMSR(HWCR, hwcr.reg.eax | (1 << 25), hwcr.reg.edx);

		impl::Ring0::MSRValue ctl, ctr;
		ring0.readMSR(PERF_CTL_0, ctl);
		ring0.readMSR(PERF_CTR_0, ctr);

		timeStampCounterMultiplier_ = estimateTimeStampCounterMultiplier();

		// restore the performance counter registers
		ring0.writeMSR(PERF_CTL_0, ctl);
		ring0.writeMSR(PERF_CTR_0, ctr);

		// restore core performance boost
		if (corePerformanceBoostSupport) {
			ring0.writeMSR(HWCR, hwcr);
		}
	}

	u32 addr = ring0.PCIAddress(0, 20, 0);
	u32 dev, rev;
	if (ring0.readPciConfig(addr, 0, dev)) {
		ring0.readPciConfig(addr, 8, rev);

		if (dev == 0x43851002)
			cStatesIoOffset_ = static_cast<u8>((rev & 0xFF) < 0x40 ? 0xB3 : 0x9C);
		else if (dev == 0x780B1022 || dev == 0x790B1022)
			cStatesIoOffset_ = 0x9C;
	}

	if (cStatesIoOffset_ != 0) {
		cStatesResidency_.resize(2);
	}
}

double wm_sensors::hardware::cpu::Amd10Cpu::estimateTimeStampCounterMultiplier(double timeWindow)
{
	auto& ring0 = impl::Ring0::instance();
	// select event "076h CPU Clocks not Halted" and enable the counter
	ring0.writeMSR(
	    PERF_CTL_0,
	    (1 << 22) |     // enable performance counter
	        (1 << 17) | // count events in user mode
	        (1 << 16) | // count events in operating-system mode
	        0x76,
	    0x00000000);

	// set the counter to 0
	ring0.writeMSR(PERF_CTR_0, 0, 0);

	LARGE_INTEGER freq, counter;
	::QueryPerformanceFrequency(&freq);
	s64 ticks = static_cast<s64>(timeWindow * static_cast<double>(freq.QuadPart));

	::QueryPerformanceCounter(&counter);
	s64 timeBegin = counter.QuadPart + static_cast<s64>(std::ceil(0.001 * static_cast<double>(ticks)));

	s64 timeEnd = timeBegin + ticks;
	while (counter.QuadPart < timeBegin) {
		::QueryPerformanceCounter(&counter);
	}

	impl::Ring0::MSRValue begin;
	ring0.readMSR(PERF_CTR_0, begin);

	while (counter.QuadPart < timeEnd) {
		::QueryPerformanceCounter(&counter);
	}

	impl::Ring0::MSRValue end, cofvid;
	ring0.readMSR(PERF_CTR_0, end);
	ring0.readMSR(COFVID_STATUS, cofvid);
	double coreMultiplier = this->coreMultiplier(cofvid.reg.eax);

	u64 countBegin = begin.value;
	u64 countEnd = end.value;

	double coreFrequency = 1e-6 * (static_cast<double>(countEnd - countBegin) * static_cast<double>(freq.QuadPart)) /
		static_cast<double>(timeEnd - timeBegin);
	double busFrequency = coreFrequency / coreMultiplier;
	return 0.25 * std::round(4 * timeStampCounterFrequency() / busFrequency);
}

wm_sensors::SensorChip::Config wm_sensors::hardware::cpu::Amd10Cpu::config() const
{
	Config res = base::config();
	res.appendChannels(SensorType::temp, 1, attributes::temp_input | attributes::temp_label);
	res.appendChannels(SensorType::in, 2, attributes::in_input | attributes::in_label);
	res.appendChannels(
	    SensorType::frequency, 1 + coreClock_.size(), attributes::frequency_input | attributes::frequency_label);
	res.appendChannels(
	    SensorType::fraction, cStatesResidency_.size(), attributes::generic_input | attributes::generic_label);
	return res;
}

int wm_sensors::hardware::cpu::Amd10Cpu::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	std::size_t myChannel;

	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		if (std::chrono::steady_clock::now() > lastUpdate_ + updateInterval) {
			this->update();
		}
		switch (type) {
			case SensorType::temp:
				if (myChannel == 0) {
					val = coreTemperature_;
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::in:
				switch (myChannel) {
					case 0: val = coreVoltage_; return 0;
					case 1: val = northbridgeVoltage_; return 0;
					default: return -EOPNOTSUPP;
				}
			case SensorType::frequency:
				if (myChannel == 0) {
					val = busClock_;
					return 0;
				} else {
					--myChannel;
				}
				if (myChannel < coreClock_.size()) {
					val = coreClock_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::fraction:
				if (myChannel < cStatesResidency_.size()) {
					val = cStatesResidency_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, val);
}

int wm_sensors::hardware::cpu::Amd10Cpu::read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	std::size_t myChannel;

	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		switch (type) {
			case SensorType::temp:
				if (myChannel == 0) {
					str = "CPU Cores";
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::in:
				switch (myChannel) {
					case 0: str = "CPU Cores"; return 0;
					case 1: str = "Northbridge"; return 0;
					default: return -EOPNOTSUPP;
				}
			case SensorType::frequency:
				if (myChannel == 0) {
					str = "Bus Speed";
					return 0;
				} else {
					--myChannel;
				}
				if (myChannel < coreClock_.size()) {
					str = coreLabels()[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::fraction:
				if (myChannel < cStatesResidency_.size()) {
					switch (myChannel) {
						case 0: str = "CPU Package C2"; return 0;
						case 1: str = "CPU Package C3"; return 0;
					}
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, str);
}

double wm_sensors::hardware::cpu::Amd10Cpu::estimateTimeStampCounterMultiplier()
{
	// preload the function
	estimateTimeStampCounterMultiplier(0);
	estimateTimeStampCounterMultiplier(0);

	// estimate the multiplier
	std::vector<double> estimate{3};
	for (std::size_t i = 0; i < estimate.size(); i++) {
		estimate[i] = estimateTimeStampCounterMultiplier(0.025);
	}
	auto m = estimate.begin() + static_cast<std::ptrdiff_t>(estimate.size() / 2);
	std::nth_element(estimate.begin(), m, estimate.end());

	return estimate[estimate.size() / 2];
}

double wm_sensors::hardware::cpu::Amd10Cpu::coreMultiplier(u32 cofVidEax) const
{
	switch (family()) {
		case 0x10:
		case 0x11:
		case 0x15:
		case 0x16: {
			// 8:6 CpuDid: current core divisor ID
			// 5:0 CpuFid: current core frequency ID
			u32 cpuDid = (cofVidEax >> 6) & 7;
			u32 cpuFid = cofVidEax & 0x1F;
			return 0.5 * (cpuFid + 0x10) / (1 << (int)cpuDid);
		}
		case 0x12: {
			// 8:4 CpuFid: current CPU core frequency ID
			// 3:0 CpuDid: current CPU core divisor ID
			u32 cpuFid = (cofVidEax >> 4) & 0x1F;
			u32 cpuDid = cofVidEax & 0xF;
			double divisor;
			switch (cpuDid) {
				case 0: divisor = 1; break;
				case 1: divisor = 1.5; break;
				case 2: divisor = 2; break;
				case 3: divisor = 3; break;
				case 4: divisor = 4; break;
				case 5: divisor = 6; break;
				case 6: divisor = 8; break;
				case 7: divisor = 12; break;
				case 8: divisor = 16; break;
				default: divisor = 1; break;
			}

			return (cpuFid + 0x10) / divisor;
		}
		case 0x14: {
			// 8:4: current CPU core divisor ID most significant digit
			// 3:0: current CPU core divisor ID least significant digit
			u32 divisorIdMsd = (cofVidEax >> 4) & 0x1F;
			u32 divisorIdLsd = cofVidEax & 0xF;
			u32 value;
			impl::Ring0::instance().readPciConfig(
			    miscellaneousControlAddress_, CLOCK_POWER_TIMING_CONTROL_0_REGISTER, value);
			u32 frequencyId = value & 0x1F;
			return (frequencyId + 0x10) / (divisorIdMsd + (divisorIdLsd * 0.25) + 1);
		}
		default: return 1;
	}
}

void wm_sensors::hardware::cpu::Amd10Cpu::update() const {
	auto& ring0 = impl::Ring0::instance();

	if (miscellaneousControlAddress_ != impl::Ring0::INVALID_PCI_ADDRESS) {
		u32 value;
		bool isValueValid =
		    hasSmuTemperatureRegister_ ?
		        readSMURegister(SMU_REPORTED_TEMP_CTRL_OFFSET, value) :
                ring0.readPciConfig(miscellaneousControlAddress_, REPORTED_TEMPERATURE_CONTROL_REGISTER, value);

			if (isValueValid) {
			if ((family() == 0x15 || family() == 0x16) && (value & 0x30000) == 0x3000) {
				    if (family() == 0x15 && (model() & 0xF0) == 0x00) {
						coreTemperature_ =
					        static_cast<double>((value >> 21) & 0x7FC) / 8.0 /* + _coreTemperature.Parameters[0].Value */ - 49;
					} else {
						coreTemperature_ =
					        static_cast<double>((value >> 21) & 0x7FF) / 8.0 /* + _coreTemperature.Parameters[0].Value */ - 49;
					}
				} else {
				coreTemperature_ = static_cast<double>((value >> 21) & 0x7FF) / 8.0 /* + _coreTemperature.Parameters[0].Value */;
				}
			} else {
			    coreTemperature_ = std::numeric_limits<decltype(coreTemperature_)>::quiet_NaN();
			}
	} else {
		coreTemperature_ = std::numeric_limits<decltype(coreTemperature_)>::quiet_NaN();
	}

	if (hasTimeStampCounter()) {
		double newBusClock = 0;
		float maxCoreVoltage = 0, maxNbVoltage = 0;

		for (std::size_t i = 0; i < coreClock_.size(); i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			impl::Ring0::MSRValue cofVid;
			if (ring0.readMSR(COFVID_STATUS, cofVid, cpuIdData()[i][0].affinity())) {
				double multiplier = coreMultiplier(cofVid.reg.eax);

				coreClock_[i] = (float)(multiplier * timeStampCounterFrequency() / timeStampCounterMultiplier_);
				newBusClock = timeStampCounterFrequency() / timeStampCounterMultiplier_;
			} else {
				coreClock_[i] = timeStampCounterFrequency();
			}

			auto SVI2Volt = [](u32 vid) -> float {
				return vid < 0b1111'1000 ? 1.5500f - 0.00625f * static_cast<float>(vid) : 0;
			};
			auto SVI1Volt = [](u32 vid) -> float {
				return vid < 0x7C ? 1.550f - 0.0125f * static_cast<float>(vid) : 0;
			};

			float newCoreVoltage, newNbVoltage;
			u32 coreVid60 = (cofVid.reg.eax >> 9) & 0x7F;
			if (isSvi2_) {
				newCoreVoltage = SVI2Volt(cofVid.reg.eax >> 13 & 0x80 | coreVid60);
				newNbVoltage = SVI2Volt(cofVid.reg.eax >> 24);
			} else {
				newCoreVoltage = SVI1Volt(coreVid60);
				newNbVoltage = SVI1Volt(cofVid.reg.eax >> 25);
			}

			if (newCoreVoltage > maxCoreVoltage)
				maxCoreVoltage = newCoreVoltage;

			if (newNbVoltage > maxNbVoltage)
				maxNbVoltage = newNbVoltage;
		}

		coreVoltage_ = maxCoreVoltage;
		northbridgeVoltage_ = maxNbVoltage;

		if (newBusClock > 0) {
			busClock_ = newBusClock;
		}
	}

	for (std::size_t i = 0; i < cStatesResidency_.size(); i++) {
		ring0.writeIOPOrt(CSTATES_IO_PORT, static_cast<u8>(cStatesIoOffset_ + i));
		cStatesResidency_[i] = static_cast<double>(ring0.readIOPort(CSTATES_IO_PORT + 1)) / 256.;
	}
}

bool wm_sensors::hardware::cpu::Amd10Cpu::readSMURegister(u32 address, u32& value)
{
	impl::GlobalMutexTryLock lock{impl::GlobalMutex::PCIBus, std::chrono::milliseconds{10}};
	if (lock.failed() || !impl::Ring0::instance().writePciConfig(0, 0xB8, address)) {
		value = 0;
		return false;
	}

	return impl::Ring0::instance().readPciConfig(0, 0xBC, value);
}
