// SPDX-License-Identifier: LGPL-3.0+

// Copyright (C) LibreHardwareMonitor contributors

#include "./amd0f_cpu.hxx"

#include "../../impl/ring0.hxx"

#include <limits>
#include <thread>

using namespace wm_sensors::stdtypes;

namespace {
	const u32 FIDVID_STATUS = 0xC0010042;
	const u16 MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1103;
	const u8 MISCELLANEOUS_CONTROL_FUNCTION = 3;
	const unsigned THERMTRIP_STATUS_REGISTER = 0xE4;

	const std::chrono::seconds updateInterval{1};
} // namespace

wm_sensors::hardware::cpu::Amd0FCpu::Amd0FCpu(unsigned processorIndex, std::vector<std::vector<CPUIDData>>&& cpuId)
    : base{processorIndex, std::move(cpuId)}
    , baseChannels_{base::config().nrChannels()}
    , busClock_{std::numeric_limits<decltype(busClock_)>::quiet_NaN()}
    , lastUpdate_{std::chrono::steady_clock::now() - 2 * updateInterval}
{
	temperatureOffset_ = -49.0f;

	// AM2+ 65nm +21 offset
	u32 model = cpu0IdData().model();
	if (model >= 0x69 && model != 0xc1 && model != 0x6c && model != 0x7c) {
		temperatureOffset_ += 21;
	}

	if (model < 40) {
		// AMD Athlon 64 Processors
		thermSenseCoreSelCPU0_ = 0x0;
		thermSenseCoreSelCPU1_ = 0x4;
	} else {
		// AMD NPT Family 0Fh Revision F, G have the core selection swapped
		thermSenseCoreSelCPU0_ = 0x4;
		thermSenseCoreSelCPU1_ = 0x0;
	}

	// check if processor supports a digital thermal sensor
	if ((cpu0IdData().safeExtData(7, 3, 0) & 1) != 0) {
		coreTemperatures_.resize(coreCount());
	}

	miscellaneousControlAddress_ = PCIAddress(MISCELLANEOUS_CONTROL_FUNCTION, MISCELLANEOUS_CONTROL_DEVICE_ID);
	if (hasTimeStampCounter()) {
		coreClocks_.resize(coreCount());
	}
}

wm_sensors::SensorChip::Config wm_sensors::hardware::cpu::Amd0FCpu::config() const
{
	Config res = base::config();
	res.appendChannels(
	    SensorType::frequency, 1 + coreClocks_.size(), attributes::frequency_input | attributes::frequency_label);
	res.appendChannels(SensorType::temp, coreTemperatures_.size(), attributes::temp_input | attributes::temp_label);
	return res;
}

int wm_sensors::hardware::cpu::Amd0FCpu::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	std::size_t myChannel;

	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		if (std::chrono::steady_clock::now() > lastUpdate_ + updateInterval) {
			this->update();
		}
		switch (type) {
			case SensorType::temp:
				if (myChannel < coreTemperatures_.size()) {
					val = coreTemperatures_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::frequency:
				if (myChannel == 0) {
					val = busClock_ * 1e6;
					return 0;
				} else {
					--myChannel;
				}
				if (myChannel < coreClocks_.size()) {
					val = coreClocks_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, val);
}

int wm_sensors::hardware::cpu::Amd0FCpu::read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	std::size_t myChannel;

	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		switch (type) {
			case SensorType::temp:
				if (myChannel < coreTemperatures_.size()) {
					str = coreLabels()[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::frequency:
				if (myChannel == 0) {
					str = "Bus Clock";
					return 0;
				} else {
					--myChannel;
				}
				if (myChannel < coreClocks_.size()) {
					str = coreLabels()[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, str);
}

void wm_sensors::hardware::cpu::Amd0FCpu::update() const
{
	if (miscellaneousControlAddress_ != impl::Ring0::INVALID_PCI_ADDRESS) {
		impl::GlobalMutexTryLock lock{impl::GlobalMutex::ISABus, std::chrono::milliseconds(10)};

		if (lock.succeded()) {
			for (std::size_t i = 0; i < coreTemperatures_.size(); i++) {
				if (impl::Ring0::instance().writePciConfig(
				        miscellaneousControlAddress_, THERMTRIP_STATUS_REGISTER,
				        i > 0 ? thermSenseCoreSelCPU1_ : thermSenseCoreSelCPU0_)) {
					u32 value;
					if (impl::Ring0::instance().readPciConfig(
					        miscellaneousControlAddress_, THERMTRIP_STATUS_REGISTER, value)) {
						coreTemperatures_[i] =
						    static_cast<double>((value >> 16) & 0xFF) + temperatureOffset_ /* _coreTemperatures[i].Parameters[0].Value */;
					} else {
						coreTemperatures_[i] = std::numeric_limits<double>::quiet_NaN();
					}
				}
			}
		}
	}

	if (hasTimeStampCounter()) {
		double newBusClock = 0;

		for (std::size_t i = 0; i < coreClocks_.size(); i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			u32 eax, edx;
			if (impl::Ring0::instance().readMSR(FIDVID_STATUS, eax, edx, cpuId()[i][0].affinity())) {
				// CurrFID can be found in eax bits 0-5, MaxFID in 16-21
				// 8-13 hold StartFID, we don't use that here.
				double curMp = 0.5 * ((eax & 0x3F) + 8);
				double maxMp = 0.5 * ((eax >> 16 & 0x3F) + 8);
				coreClocks_[i] = (float)(curMp * timeStampCounterFrequency() / maxMp);
				newBusClock = (timeStampCounterFrequency() / maxMp);
			} else {
				// Fail-safe value - if the code above fails, we'll use this instead
				coreClocks_[i] = timeStampCounterFrequency();
			}
		}

		busClock_ = newBusClock > 0 ? newBusClock : std::numeric_limits<double>::quiet_NaN();
	}

	lastUpdate_ = std::chrono::steady_clock::now();
}
