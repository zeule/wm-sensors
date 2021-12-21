// SPDX-License-Identifier: LGPL-3.0+

// Copyright (C) LibreHardwareMonitor contributors

#include "./intel_cpu.hxx"

#include "../../impl/ring0.hxx"

#include <limits>
#include <thread>

using wm_sensors::hardware::impl::Ring0;

namespace {
	using namespace wm_sensors::stdtypes;

	const u32 IA32_PACKAGE_THERM_STATUS = 0x1B1;
	const u32 IA32_PERF_STATUS = 0x0198;
	const u32 IA32_TEMPERATURE_TARGET = 0x01A2;
	const u32 IA32_THERM_STATUS_MSR = 0x019C;

	const u32 MSR_DRAM_ENERGY_STATUS = 0x619;
	const u32 MSR_PKG_ENERY_STATUS = 0x611;
	const u32 MSR_PLATFORM_INFO = 0xCE;
	const u32 MSR_PP0_ENERY_STATUS = 0x639;
	const u32 MSR_PP1_ENERY_STATUS = 0x641;

	const u32 MSR_RAPL_POWER_UNIT = 0x606;

	const u32 energyStatusMsrs[] = {
	    MSR_PKG_ENERY_STATUS, MSR_PP0_ENERY_STATUS, MSR_PP1_ENERY_STATUS, MSR_DRAM_ENERGY_STATUS};

	const std::chrono::seconds updateFreq{1};
} // namespace

wm_sensors::hardware::cpu::IntelCPU::IntelCPU(unsigned processorIndex, std::vector<std::vector<CPUIDData>>&& cpuId)
    : GenericCPU(processorIndex, std::move(cpuId))
    , baseChannels_{base::config().nrChannels()}
    , lastUpdate_{std::chrono::steady_clock::now() - 2 * updateFreq}
{
	// set tjMax
	std::vector<float> tjMax;
	switch (family()) {
		case 0x06: {
			switch (model()) {
				case 0x0F: // Intel Core 2 (65nm)
					microArchitecture_ = MicroArchitecture::Core;
					switch (stepping()) {
						case 0x06: // B2
							switch (coreCount()) {
								case 2: tjMax = std::vector<float>(coreCount(), 80 + 10); break;
								case 4: tjMax = std::vector<float>(coreCount(), 90 + 10); break;
								default: tjMax = std::vector<float>(coreCount(), 85 + 10); break;
							}
							break;
						case 0x0B: // G0
							tjMax = std::vector<float>(coreCount(), 90 + 10);
							break;
						case 0x0D: // M0
							tjMax = std::vector<float>(coreCount(), 85 + 10);
							break;
						default: tjMax = std::vector<float>(coreCount(), 85 + 10); break;
					}
					break;
				case 0x17: // Intel Core 2 (45nm)
					microArchitecture_ = MicroArchitecture::Core;
					tjMax = std::vector<float>(coreCount(), 100);
					break;
				case 0x1C: // Intel Atom (45nm)
					microArchitecture_ = MicroArchitecture::Atom;
					switch (stepping()) {
						case 0x02: // C0
							tjMax = std::vector<float>(coreCount(), 90);
							break;
						case 0x0A: // A0, B0
							tjMax = std::vector<float>(coreCount(), 100);
							break;
						default: tjMax = std::vector<float>(coreCount(), 90); break;
					}
					break;
				case 0x1A: // Intel Core i7 LGA1366 (45nm)
				case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
				case 0x1F: // Intel Core i5, i7
				case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
				case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
				case 0x2E: // Intel Xeon Processor 7500 series (45nm)
				case 0x2F: // Intel Xeon Processor (32nm)
					microArchitecture_ = MicroArchitecture::Nehalem;
					tjMax = tjsFromMSR();
					break;
				case 0x2A: // Intel Core i5, i7 2xxx LGA1155 (32nm)
				case 0x2D: // Next Generation Intel Xeon, i7 3xxx LGA2011 (32nm)
					microArchitecture_ = MicroArchitecture::SandyBridge;
					tjMax = tjsFromMSR();
					break;
				case 0x3A: // Intel Core i5, i7 3xxx LGA1155 (22nm)
				case 0x3E: // Intel Core i7 4xxx LGA2011 (22nm)
					microArchitecture_ = MicroArchitecture::IvyBridge;
					tjMax = tjsFromMSR();
					break;
				case 0x3C: // Intel Core i5, i7 4xxx LGA1150 (22nm)
				case 0x3F: // Intel Xeon E5-2600/1600 v3, Core i7-59xx
				// LGA2011-v3, Haswell-E (22nm)
				case 0x45: // Intel Core i5, i7 4xxxU (22nm)
				case 0x46:
					microArchitecture_ = MicroArchitecture::Haswell;
					tjMax = tjsFromMSR();
					break;
				case 0x3D: // Intel Core M-5xxx (14nm)
				case 0x47: // Intel i5, i7 5xxx, Xeon E3-1200 v4 (14nm)
				case 0x4F: // Intel Xeon E5-26xx v4
				case 0x56: // Intel Xeon D-15xx
					microArchitecture_ = MicroArchitecture::Broadwell;
					tjMax = tjsFromMSR();
					break;
				case 0x36: // Intel Atom S1xxx, D2xxx, N2xxx (32nm)
					microArchitecture_ = MicroArchitecture::Atom;
					tjMax = tjsFromMSR();
					break;
				case 0x37: // Intel Atom E3xxx, Z3xxx (22nm)
				case 0x4A:
				case 0x4D: // Intel Atom C2xxx (22nm)
				case 0x5A:
				case 0x5D:
					microArchitecture_ = MicroArchitecture::Silvermont;
					tjMax = tjsFromMSR();
					break;
				case 0x4E:
				case 0x5E: // Intel Core i5, i7 6xxxx LGA1151 (14nm)
				case 0x55: // Intel Core X i7, i9 7xxx LGA2066 (14nm)
					microArchitecture_ = MicroArchitecture::Skylake;
					tjMax = tjsFromMSR();
					break;
				case 0x4C: // Intel Airmont (Cherry Trail, Braswell)
					microArchitecture_ = MicroArchitecture::Airmont;
					tjMax = tjsFromMSR();
					break;
				case 0x8E: // Intel Core i5, i7 7xxxx (14nm) (Kaby Lake) and 8xxxx (14nm++) (Coffee Lake)
				case 0x9E:
					microArchitecture_ = MicroArchitecture::KabyLake;
					tjMax = tjsFromMSR();
					break;
				case 0x5C: // Goldmont (Apollo Lake)
				case 0x5F: // (Denverton)
					microArchitecture_ = MicroArchitecture::Goldmont;
					tjMax = tjsFromMSR();
					break;
				case 0x7A: // Goldmont plus (Gemini Lake)
					microArchitecture_ = MicroArchitecture::GoldmontPlus;
					tjMax = tjsFromMSR();
					break;
				case 0x66: // Intel Core i3 8xxx (10nm) (Cannon Lake)
					microArchitecture_ = MicroArchitecture::CannonLake;
					tjMax = tjsFromMSR();
					break;
				case 0x7D: // Intel Core i3, i5, i7 10xxx (10nm) (Ice Lake)
				case 0x7E:
				case 0x6A: // Ice Lake server
				case 0x6C:
					microArchitecture_ = MicroArchitecture::IceLake;
					tjMax = tjsFromMSR();
					break;
				case 0xA5:
				case 0xA6: // Intel Core i3, i5, i7 10xxxU (14nm)
					microArchitecture_ = MicroArchitecture::CometLake;
					tjMax = tjsFromMSR();
					break;
				case 0x86: // Tremont (10nm) (Elkhart Lake, Skyhawk Lake)
					microArchitecture_ = MicroArchitecture::Tremont;
					tjMax = tjsFromMSR();
					break;
				case 0x8C: // Tiger Lake (10nm)
				case 0x8D:
					microArchitecture_ = MicroArchitecture::TigerLake;
					tjMax = tjsFromMSR();
					break;
				case 0x97: // Alder Lake (7nm)
					microArchitecture_ = MicroArchitecture::AlderLake;
					tjMax = tjsFromMSR();
					break;
				case 0x9C: // Jasper Lake (10nm)
					microArchitecture_ = MicroArchitecture::JasperLake;
					tjMax = tjsFromMSR();
					break;
				case 0xA7: // Intel Core i5, i6, i7 11xxx (14nm) (Rocket Lake)
					microArchitecture_ = MicroArchitecture::RocketLake;
					tjMax = tjsFromMSR();
					break;
				default:
					microArchitecture_ = MicroArchitecture::Unknown;
					tjMax = std::vector<float>(coreCount(), 100);
					break;
			}
		} break;
		case 0x0F: {
			switch (model()) {
				case 0x00: // Pentium 4 (180nm)
				case 0x01: // Pentium 4 (130nm)
				case 0x02: // Pentium 4 (130nm)
				case 0x03: // Pentium 4, Celeron D (90nm)
				case 0x04: // Pentium 4, Pentium D, Celeron D (90nm)
				case 0x06: // Pentium 4, Pentium D, Celeron D (65nm)
					microArchitecture_ = MicroArchitecture::NetBurst;
					tjMax = std::vector<float>(coreCount(), 100);
					break;
				default:
					microArchitecture_ = MicroArchitecture::Unknown;
					tjMax = std::vector<float>(coreCount(), 100);
					break;
			}
		} break;
		default:
			microArchitecture_ = MicroArchitecture::Unknown;
			tjMax = std::vector<float>(coreCount(), 100);
			break;
	}
	// set timeStampCounterMultiplier
	switch (microArchitecture_) {
		case MicroArchitecture::Atom:
		case MicroArchitecture::Core:
		case MicroArchitecture::NetBurst: {
			u32 eax, edx;
			if (Ring0::instance().readMSR(IA32_PERF_STATUS, eax, edx)) {
				timeStampCounterMultiplier_ = ((edx >> 8) & 0x1f) + 0.5 * ((edx >> 14) & 1);
			}

			break;
		}
		case MicroArchitecture::Airmont:
		case MicroArchitecture::AlderLake:
		case MicroArchitecture::Broadwell:
		case MicroArchitecture::CannonLake:
		case MicroArchitecture::CometLake:
		case MicroArchitecture::Goldmont:
		case MicroArchitecture::GoldmontPlus:
		case MicroArchitecture::Haswell:
		case MicroArchitecture::IceLake:
		case MicroArchitecture::IvyBridge:
		case MicroArchitecture::JasperLake:
		case MicroArchitecture::KabyLake:
		case MicroArchitecture::Nehalem:
		case MicroArchitecture::RocketLake:
		case MicroArchitecture::SandyBridge:
		case MicroArchitecture::Silvermont:
		case MicroArchitecture::Skylake:
		case MicroArchitecture::TigerLake:
		case MicroArchitecture::Tremont: {
			u32 eax, edx;
			if (Ring0::instance().readMSR(MSR_PLATFORM_INFO, eax, edx)) {
				timeStampCounterMultiplier_ = (eax >> 8) & 0xff;
			}
		} break;
		default: timeStampCounterMultiplier_ = 0; break;
	}

	// check if processor supports a digital thermal sensor at core level
	if ((cpu0IdData().safeData(6, 0, 0) & 1) != 0 && microArchitecture_ != MicroArchitecture::Unknown) {
		coreTemperatures_.reserve(coreCount());
		for (std::size_t i = 0; i < coreCount(); i++) {
			coreTemperatures_.push_back({tjMax[i], 1., 0.});
			temperatureLabels_.push_back(coreString(i));
		}
		for (std::size_t i = 0; i < coreCount(); i++) {
			temperatureLabels_.push_back(coreString(i) + " distance to TjMax");
		}
	}

	// check if processor supports a digital thermal sensor at package level
	if ((cpu0IdData().safeData(6, 0, 0) & 0x40) != 0 && microArchitecture_ != MicroArchitecture::Unknown) {
		packageTemperature_ = {tjMax[0], 1., 0.};
		temperatureLabels_.emplace_back("CPU Package");
	}

#if 0
	// dist to tjmax sensor
	if (cpu0IdData().data().size() > 6 && (cpu0IdData().data(6, 0) & 1) != 0 &&
	    microArchitecture_ != MicroArchitecture::Unknown) {
		_distToTjMaxTemperatures = new Sensor[_coreCount];
		for (int i = 0; i < _distToTjMaxTemperatures.Length; i++) {
			_distToTjMaxTemperatures[i] =
			    new Sensor(CoreString(i) + " Distance to TjMax", coreSensorId, SensorType.Temperature, this, settings);
			ActivateSensor(_distToTjMaxTemperatures[i]);
			coreSensorId++;
		}
	} else
		_distToTjMaxTemperatures = new Sensor[0];
#endif

	// core temp avg and max value
	// is only available when the cpu has more than 1 core
	if ((cpu0IdData().safeData(6, 0, 0) & 0x40) != 0 && microArchitecture_ != MicroArchitecture::Unknown &&
	    coreCount() > 1) {
		coreMaxTemperature_ = 0.;
		temperatureLabels_.emplace_back("Core Max");
		coreAvgTemperature_ = 0.;
		temperatureLabels_.emplace_back("Core Average");
	}

	if (hasTimeStampCounter() && microArchitecture_ != MicroArchitecture::Unknown) {
		frequencyLabels_.emplace_back("Bus Speed");
		busClock_ = 0.;
		coreClocks_.resize(coreCount(), 0.f);
		for (std::size_t i = 0; i < coreCount(); i++) {
			frequencyLabels_.emplace_back(coreString(i));
		}
	}

	if (microArchitecture_ == MicroArchitecture::Airmont || microArchitecture_ == MicroArchitecture::AlderLake ||
	    microArchitecture_ == MicroArchitecture::Broadwell || microArchitecture_ == MicroArchitecture::CannonLake ||
	    microArchitecture_ == MicroArchitecture::CometLake || microArchitecture_ == MicroArchitecture::Goldmont ||
	    microArchitecture_ == MicroArchitecture::GoldmontPlus || microArchitecture_ == MicroArchitecture::Haswell ||
	    microArchitecture_ == MicroArchitecture::IceLake || microArchitecture_ == MicroArchitecture::IvyBridge ||
	    microArchitecture_ == MicroArchitecture::JasperLake || microArchitecture_ == MicroArchitecture::KabyLake ||
	    microArchitecture_ == MicroArchitecture::RocketLake || microArchitecture_ == MicroArchitecture::SandyBridge ||
	    microArchitecture_ == MicroArchitecture::Silvermont || microArchitecture_ == MicroArchitecture::Skylake ||
	    microArchitecture_ == MicroArchitecture::TigerLake || microArchitecture_ == MicroArchitecture::Tremont) {
		u32 eax, edx;
		if (Ring0::instance().readMSR(MSR_RAPL_POWER_UNIT, eax, edx)) {
			switch (microArchitecture_) {
				case MicroArchitecture::Silvermont:
				case MicroArchitecture::Airmont:
					energyUnitMultiplier_ = 1.0e-6f * static_cast<float>(1 << ((eax >> 8) & 0x1F));
					break;
				default:
					energyUnitMultiplier_ = 1.0f / static_cast<float>(1 << ((eax >> 8) & 0x1F));
					break;
			}
		}

		if (energyUnitMultiplier_ != 0) {
			powerSensors_.resize(utility::array_size(energyStatusMsrs), std::numeric_limits<float>::quiet_NaN());
			lastEnergyTime_.resize(utility::array_size(energyStatusMsrs));
			lastEnergyConsumed_.resize(utility::array_size(energyStatusMsrs));

			const char* powerSensorLabels[] = {"CPU Package", "CPU Cores", "CPU Graphics", "CPU Memory"};

			for (std::size_t i = 0; i < utility::array_size(energyStatusMsrs); i++) {
				if (!Ring0::instance().readMSR(energyStatusMsrs[i], eax, edx)) {
					continue;
				}

				powerLabels_.emplace_back(powerSensorLabels[i]);
				lastEnergyTime_[i] = std::chrono::steady_clock::now();
				lastEnergyConsumed_[i] = eax;
				powerSensors_[i] = 0.;
			}
		}
	}

	update();
}

wm_sensors::SensorChip::Config wm_sensors::hardware::cpu::IntelCPU::config() const
{
	Config res = base::config();
	res.appendChannels(SensorType::temp, temperatureLabels_.size(), attributes::temp_input | attributes::temp_label);
	res.appendChannels(
	    SensorType::frequency, frequencyLabels_.size(), attributes::frequency_input | attributes::frequency_label);
	res.appendChannels(SensorType::power, powerLabels_.size(), attributes::power_input | attributes::power_label);

	return res;
}

int wm_sensors::hardware::cpu::IntelCPU::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	std::size_t myChannel;
	
	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		if (std::chrono::steady_clock::now() > lastUpdate_ + updateFreq) {
			this->update();
		}
		const auto optionallyRead = [&myChannel, &val](const std::optional<double>& o) -> bool {
			if (o.has_value()) {
				if (myChannel == 0) {
					val = o.value();
					return true;
				} else {
					--myChannel;
					return false;
				}
			}
			return false;
		};

		switch (type) {
			case SensorType::temp:
				// order: coreTemperatures_(value),temperatureLabels_(delta), packageTemperature_, coreMaxTemperature_, coreAvgTemperature_
				if (myChannel < coreTemperatures_.size()) {
					val = coreTemperatures_[myChannel].value;
					return 0;
				} else {
					myChannel -= coreTemperatures_.size();
				}
				if (myChannel < coreTemperatures_.size()) {
					val = coreTemperatures_[myChannel].deltaT;
					return 0;
				} else {
					myChannel -= coreTemperatures_.size();
				}
				if (packageTemperature_.has_value()) {
					if (myChannel == 0 ) {
						val = packageTemperature_.value().value;
						return 0;
					} else {
						--myChannel;
					}
				}
				if (optionallyRead(coreMaxTemperature_)) {
					return 0;
				}
				if (optionallyRead(coreAvgTemperature_)) {
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::frequency:
				// order: busClock_, coreClocks_
				if (optionallyRead(busClock_)) {
					return 0;
				}
				if (myChannel < coreClocks_.size()) {
					val = coreClocks_[myChannel] * 1e6;
					return 0;
				} else {
					myChannel -= coreClocks_.size();
				}
				return -EOPNOTSUPP;
			case SensorType::power:
				// order: 
				for (std::size_t i = 0; i < powerSensors_.size(); ++i) {
					if (!std::isnan(powerSensors_[i])) {
						if (myChannel == 0) {
							val = powerSensors_[i];
							return 0;
						} else {
							--myChannel;
						}
					}
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, val);
}

int wm_sensors::hardware::cpu::IntelCPU::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	std::size_t myChannel;

	if (Config::isInRange(baseChannels_, type, channel, &myChannel)) {
		switch (type) {
			case SensorType::temp:
				if (myChannel < temperatureLabels_.size()) {
					str = temperatureLabels_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::frequency:
				if (myChannel < frequencyLabels_.size()) {
					str = frequencyLabels_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			case SensorType::power:
				if (myChannel < powerLabels_.size()) {
					str = powerLabels_[myChannel];
					return 0;
				}
				return -EOPNOTSUPP;
			default: return -EOPNOTSUPP;
		}
	}
	return base::read(type, attr, channel, str);
}

void wm_sensors::hardware::cpu::IntelCPU::update() const
{
	double coreMax = std::numeric_limits<float>::min();
	double coreAvg = 0.f;

	for (std::size_t i = 0; i < coreTemperatures_.size(); i++) {
		// if reading is valid
		u32 eax, edx;

		if (Ring0::instance().readMSR(IA32_THERM_STATUS_MSR, eax, edx, cpuIdData()[i][0].affinity()) &&
		    (eax & 0x80000000) != 0) {
			// get the dist from tjMax from bits 22:16
			double deltaT = static_cast<float>((eax & 0x007F0000) >> 16);
			coreTemperatures_[i].value = coreTemperatures_[i].tjMax - coreTemperatures_[i].slope * deltaT;

			coreAvg += coreTemperatures_[i].value;
			if (coreMax < coreTemperatures_[i].value) {
				coreMax = coreTemperatures_[i].value;
			}
		} else {
			coreTemperatures_[i].value = std::numeric_limits<decltype(CoreTempData::value)>::quiet_NaN();
		}
	}

	// calculate average cpu temperature over all cores
	if (coreMaxTemperature_.has_value() &&
	    coreMax > std::numeric_limits<decltype(coreMaxTemperature_)::value_type>::min()) {
		coreMaxTemperature_= coreMax;
		coreAvg /= static_cast<double>(coreTemperatures_.size());
		coreAvgTemperature_ = coreAvg;
	}

	if (packageTemperature_.has_value()) {
		// if reading is valid
		u32 eax, edx;
		if (Ring0::instance().readMSR(IA32_PACKAGE_THERM_STATUS, eax, edx, cpu0IdData().affinity()) &&
		    (eax & 0x80000000) != 0) {
			// get the dist from tjMax from bits 22:16
			double deltaT = static_cast<double>((eax & 0x007F0000) >> 16);
			packageTemperature_.value().update(deltaT);
		} else {
			packageTemperature_.reset();
		}
	}

	if (hasTimeStampCounter() && timeStampCounterMultiplier_ > 0) {
		double newBusClock = 0;
		for (std::size_t i = 0; i < coreClocks_.size(); i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			u32 eax, edx;
			if (Ring0::instance().readMSR(IA32_PERF_STATUS, eax, edx, cpu0IdData().affinity())) {
				newBusClock = timeStampCounterFrequency() / timeStampCounterMultiplier_;
				switch (microArchitecture_) {
					case MicroArchitecture::Nehalem: {
						u32 multiplier = eax & 0xff;
						coreClocks_[i] = multiplier * newBusClock;
						break;
					}
					case MicroArchitecture::Airmont:
					case MicroArchitecture::AlderLake:
					case MicroArchitecture::Broadwell:
					case MicroArchitecture::CannonLake:
					case MicroArchitecture::CometLake:
					case MicroArchitecture::Goldmont:
					case MicroArchitecture::GoldmontPlus:
					case MicroArchitecture::Haswell:
					case MicroArchitecture::IceLake:
					case MicroArchitecture::IvyBridge:
					case MicroArchitecture::JasperLake:
					case MicroArchitecture::KabyLake:
					case MicroArchitecture::RocketLake:
					case MicroArchitecture::SandyBridge:
					case MicroArchitecture::Silvermont:
					case MicroArchitecture::Skylake:
					case MicroArchitecture::TigerLake:
					case MicroArchitecture::Tremont: {
						u32 multiplier = (eax >> 8) & 0xff;
						coreClocks_[i] = multiplier * newBusClock;
						break;
					}
					default: {
						double multiplier = ((eax >> 8) & 0x1f) + 0.5 * ((eax >> 14) & 1);
						coreClocks_[i] = multiplier * newBusClock;
						break;
					}
				}
			} else {
				// if IA32_PERF_STATUS is not available, assume TSC frequency
				coreClocks_[i] = timeStampCounterFrequency();
			}
		}

		if (newBusClock > 0) {
			busClock_ = newBusClock; // TODO activate
		}
	}

	for (std::size_t i = 0; i < powerSensors_.size(); ++i) {
		if (std::isnan(powerSensors_[i])) {
			continue;
		}

		u32 eax, edx;
		if (!Ring0::instance().readMSR(energyStatusMsrs[i], eax, edx)) {
			continue;
		}


		auto  time = std::chrono::steady_clock::now();
		u32   energyConsumed = eax;
		float deltaTime =
		    static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(time - lastEnergyTime_[i]).count()) * 1e-6f;
		if (deltaTime < 0.01) {
			continue;
		}


		powerSensors_[i] = energyUnitMultiplier_ * static_cast<float>(energyConsumed - lastEnergyConsumed_[i]) / deltaTime;
		lastEnergyTime_[i] = time;
		lastEnergyConsumed_[i] = energyConsumed;
	}
}

std::vector<float> wm_sensors::hardware::cpu::IntelCPU::tjsFromMSR()
{
	std::vector<float> result(coreCount());
	for (std::size_t i = 0; i < result.size(); i++) {
		u32 eax, edx;
		if (Ring0::instance().readMSR(IA32_TEMPERATURE_TARGET, eax, edx, cpuIdData()[i][0].affinity())) {
			result[i] = static_cast<float>((eax >> 16) & 0xFF);
		} else {
			result[i] = 100;
		}
	}

	return result;
}

void wm_sensors::hardware::cpu::IntelCPU::CoreTempData::update(double newDeltaT)
{
	deltaT = newDeltaT;
	value = tjMax - slope * newDeltaT;
}
