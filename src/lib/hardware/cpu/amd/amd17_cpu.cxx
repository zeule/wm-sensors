// SPDX-License-Identifier: LGPL-3.0+

// Copyright (C) LibreHardwareMonitor contributors

#include "./amd17_cpu.hxx"

#include "../../../utility/utility.hxx"
#include "../../impl/group_affinity.hxx"
#include "../../impl/ring0.hxx"
#include "../../impl/sensor_collection.hxx"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <system_error>

#pragma warning(disable : 4355) // 'this' : used in base member initializer list
#pragma warning(disable : 4702)

namespace {
	using namespace wm_sensors::stdtypes;
	using wm_sensors::SensorType;
	using wm_sensors::hardware::cpu::Amd17Cpu;
	using wm_sensors::hardware::cpu::CPUIDData;
	using wm_sensors::hardware::impl::Ring0;
	using wm_sensors::impl::Sensor;
	using SensorHandle = wm_sensors::impl::SensorCollection<Sensor>::Handle;


	const u32 COFVID_STATUS = 0xC0010071;
	const u32 F17H_M01H_SVI = 0x0005A000;
	const u32 F17H_M01H_THM_TCON_CUR_TMP = 0x00059800;

	// const u32 F17H_M70H_CCD1_TEMP = 0x00059954;

	const u32 F17H_TEMP_OFFSET_FLAG = 0x80000;

	const u32 HWCR = 0xC0010015;
	const u32 MSR_CORE_ENERGY_STAT = 0xC001029A;
	const u32 MSR_HARDWARE_PSTATE_STATUS = 0xC0010293;
	const u32 MSR_PKG_ENERGY_STAT = 0xC001029B;
	const u32 MSR_PSTATE_0 = 0xC0010064;
	const u32 MSR_PWR_UNIT = 0xC0010299;
	const u32 PERF_CTL_0 = 0xC0010000;

	const u32 PERF_CTR_0 = 0xC0010004;

	const auto updateTimeout = std::chrono::seconds(1);

	struct tctl_offset {
		tctl_offset(u8 pModel, const char* pId, float pOffset)
		    : id{pId}
		    , offset{pOffset}
		    , model{pModel}
		{
		}

		char const* id;
		float offset;
		u8 model;
	};

	// Offset table: https://github.com/torvalds/linux/blob/master/drivers/hwmon/k10temp.c#L78
	const struct tctl_offset tctl_offset_table[] = {
	    {0x17, "AMD Ryzen 5 1600X", -20.f},         {0x17, "AMD Ryzen 7 1700X", -20.f},
	    {0x17, "AMD Ryzen 7 1800X", -20.f},         {0x17, "AMD Ryzen 7 2700X", -10.f},
	    {0x17, "AMD Ryzen Threadripper 19", -27.f}, /* 19{00,20,50}X */
	    {0x17, "AMD Ryzen Threadripper 29", -27.f}, /* 29{20,50,70,90}[W]X */
	};

	/* Common for Zen CPU families (Family 17h and 18h and 19h) */
	const u32 ZEN_REPORTED_TEMP_CTRL_BASE = 0x00059800;
	constexpr u32 ZEN_CCD_TEMP(u32 offset, unsigned x)
	{
		return ZEN_REPORTED_TEMP_CTRL_BASE + offset + x * 4;
	}

	const auto ZEN_CCD_TEMP_VALID = wm_sensors::utility::bit<unsigned>(11);

	struct CCDInfo {
		unsigned maxCount;
		u32 offset;
	};

	CCDInfo ccdInfo(u32 family, u32 model)
	{
		if (family == 0x17 || family == 0x18) {
			switch (model) {
				case 0x1:  /* Zen */
				case 0x8:  /* Zen+ */
				case 0x11: /* Zen APU */
				case 0x18: /* Zen+ APU */ return {4, 0x154};
				case 0x31: /* Zen2 Threadripper */
				case 0x60: /* Renoir */
				case 0x68: /* Lucienne */
				case 0x71: /* Zen2 */ return {8, 0x154};
			}
		} else if (family == 0x19) {
			if (model <= 1) { // Zen3 SP3/TR
				return {8, 0x154};
			} else if (model <= 0x09) {
				return {0, 0};
			} else if (model <= 0x1f) {
				return {12, 0x300};
			} else if (model <= 0x20) {
				return {0, 0};
			} else if (model <= 0x21) { // Zen3 Ryzen Desktop
				return {8, 0x154};
			} else if (model <= 0x3f) {
				return {0, 0};
			} else if (model <= 0x4f) { // Yellow Carp
				return {8, 0x300};
			} else if (model <= 0x5f) { // Green Sardine
				return {8, 0x300};
			} else if (model <= 0x9f) {
				return {0, 0};
			} else if (model <= 0xaf) {
				return {12, 0x300};
			} else {
				return {0, 0};
			}
		}
		return {0, 0};
	}

	class AmdSmn {
	public:
		AmdSmn(u16 node)
		    : pciAddress_{nodeToPciAddress(node)}
		{
		}

		u32 read(u32 address) const
		{
			u32 val;
			std::error_code res = readWrite(address, val, false);
			if (res) {
				throw std::system_error(res);
			}
			return val;
		}

		void write(u32 address, u32 value) const
		{
			std::error_code res = readWrite(address, value, true);
			if (res) {
				throw std::system_error(res);
			}
		}

	private:
		static u32 nodeToPciAddress(u16 node);
		std::error_code readWrite(u32 address, u32& value, bool write) const;

		u32 pciAddress_;
		static std::mutex smnMutex_;
	};

	std::mutex AmdSmn::smnMutex_;

	u32 AmdSmn::nodeToPciAddress(u16 node)
	{
		// TODO
		if (node != 0) {
			throw std::logic_error("Not implemented");
		}
		return 0;
	}

	std::error_code AmdSmn::readWrite(u32 address, u32& value, bool write) const
	{
		const u32 FAMILY_17H_PCI_CONTROL_REGISTER = 0x60;

		auto& ring0 = wm_sensors::hardware::impl::Ring0::instance();

		std::lock_guard<std::mutex> lock(smnMutex_);

		if (!ring0.writePciConfig(pciAddress_, FAMILY_17H_PCI_CONTROL_REGISTER, address)) {
			spdlog::warn("Error programming SMN address {0:x}.", address);
			return std::make_error_code(std::errc::io_error);
		}

		bool r =
		    (write ? ring0.writePciConfig(pciAddress_, FAMILY_17H_PCI_CONTROL_REGISTER + 4, value) :
                     ring0.readPciConfig(pciAddress_, FAMILY_17H_PCI_CONTROL_REGISTER + 4, value));
		if (!r) {
			spdlog::warn("Error {0} SMN address {1:x}.", (write ? "writing to" : "reading from"), address);
			return std::make_error_code(std::errc::io_error);
		}

		return {};
	}

	u32 availableCCDMask(const AmdSmn& smn, const CCDInfo& info)
	{
		u32 res = 0;
		for (unsigned i = 0; i < info.maxCount; i++) {
			u32 regval = smn.read(ZEN_CCD_TEMP(info.offset, i));
			if (regval & ZEN_CCD_TEMP_VALID) {
				res |= wm_sensors::utility::bit<decltype(res)>(i);
			}
		}
		return res;
	}

	class Core {
	public:
		Core(
		    const Amd17Cpu& cpu, int id, wm_sensors::impl::SensorCollection<Sensor>& sensors,
		    const SensorHandle busSpeedSensor);
		Core(Core&&) = default;

		int id() const
		{
			return id_;
		}

		const std::vector<const CPUIDData*>& threads() const
		{
			return threads_;
		}

		std::vector<const CPUIDData*>& threads()
		{
			return threads_;
		}

		void updateSensors() const;

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(Core)

		const Amd17Cpu& cpu_;
		wm_sensors::impl::SensorCollection<Sensor>& sensors_;
		std::vector<const CPUIDData*> threads_;

		int id_;

		const SensorHandle clock_;
		const SensorHandle multiplier_;
		const SensorHandle power_;
		const SensorHandle vcore_;
		const SensorHandle busSpeed_;

		mutable std::chrono::steady_clock::time_point lastPwrTime_;
		mutable u32 lastPwrValue_;
	};

	class NumaNode {
	public:
		NumaNode(const Amd17Cpu& cpu, unsigned id);
		NumaNode(NumaNode&&) = default;

		const std::vector<Core>& cores() const
		{
			return cores_;
		}

		unsigned nodeId() const
		{
			return nodeId_;
		}

		void appendThread(
		    const CPUIDData& thread, int coreId, wm_sensors::impl::SensorCollection<Sensor>& sensors,
		    const SensorHandle busSpeedSensor);
		void updateSensors() const {}

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(NumaNode)

		const Amd17Cpu& cpu_;
		std::vector<Core> cores_;
		unsigned nodeId_;
	};

} // namespace

class wm_sensors::hardware::cpu::Amd17Cpu::Impl {
public:
	Impl(const Amd17Cpu& cpu, SensorChip::Config::ChannelCounts baseCounts);
	void appendThread(const CPUIDData& thread, unsigned numaId, int coreId);

	const std::vector<NumaNode>& nodes() const
	{
		return nodes_;
	}

	void updateSensors();

	wm_sensors::impl::SensorCollection<Sensor>& sensorsCollection()
	{
		return sensors_;
	}

	const wm_sensors::impl::SensorCollection<Sensor>& sensorsCollection() const
	{
		return sensors_;
	}

private:
	void detectOptionalSensors();
	const CPUIDData* firstThreadData() const;

	const CPUIDData& cpuid() const
	{
		return cpu_.cpu0IdData();
	}

	double timeStampCounterMultiplier();
	DELETE_COPY_CTOR_AND_ASSIGNMENT(Impl)

	const Amd17Cpu& cpu_;
	RyzenSMU smu_;
	std::vector<NumaNode> nodes_;
	wm_sensors::impl::SensorCollection<Sensor> sensors_;
	AmdSmn smn_;

	SensorHandle packagePower_;
	SensorHandle coreTemperatureTctl_;
	SensorHandle coreTemperatureTdie_;

	struct CCDSensors {
		unsigned ccdIndex;
		SensorHandle temperature;
	};
	std::vector<CCDSensors> ccdSensors_;

	SensorHandle coreTemperatureTctlTdie_;

	SensorHandle coreVoltage_;
	SensorHandle socVoltage_;
	SensorHandle busClock_;

	std::map<unsigned, std::pair<RyzenSMU::SmuSensorType, SensorHandle>> smuSensors_;

	SensorHandle ccdsAverageTemperature_;
	SensorHandle ccdsMaxTemperature_;
	float tclTemperatureOffset_;
	u32 ccdOffset_;
	std::chrono::steady_clock::time_point lastPwrTime_;
	u32 lastPwrValue_;
	// TODO: find a better way because these will probably keep changing in the future.
	unsigned sviPlane0Offset_;
	unsigned sviPlane1Offset_;
};

wm_sensors::hardware::cpu::Amd17Cpu::Amd17Cpu(unsigned processorIndex, CpuIdDataArray&& cpuId)
    : base{processorIndex, std::move(cpuId)}
    , impl_{std::make_unique<Impl>(*this, base::config().nrChannels())}
{
	// _sensorTypeIndex = new Dictionary<SensorType, int>();
	// foreach (SensorType type in Enum.GetValues(typeof(SensorType))) {
	//	_sensorTypeIndex.Add(type, 0);
	// }

	// _sensorTypeIndex[SensorType.Load] = _active.Count(x = > x.SensorType == SensorType.Load);


	// Add all numa nodes.
	// Register ..1E_2, [10:8] + 1

	// Add all numa nodes.
	int coreId = 0;
	int lastCoreId = -1; // Invalid id.

	// Ryzen 3000's skip some core ids.
	// So start at 1 and count upwards when the read core changes.
	using CpuIdDataPtrVec = std::vector<const CPUIDData*>;
	std::vector<CpuIdDataPtrVec> cpuIdDataPtr;
	cpuIdDataPtr.reserve(cpuIdData().size());
	for (const auto& cidvec: cpuIdData()) {
		cpuIdDataPtr.push_back({});
		for (const auto& cid: cidvec) {
			cpuIdDataPtr.back().push_back(&cid);
		}
	}

	std::sort(cpuIdDataPtr.begin(), cpuIdDataPtr.end(), [](const CpuIdDataPtrVec& left, const CpuIdDataPtrVec& right) {
		return (left[0]->extData(0x1e, 1) & 0xff) < (right[0]->extData(0x1e, 1) & 0xff);
	});

	for (const CpuIdDataPtrVec& cpu: cpuIdDataPtr) {
		const CPUIDData& thread = *cpu[0];

		// CPUID_Fn8000001E_EBX, Register ..1E_1, [7:0]
		// threads per core =  CPUID_Fn8000001E_EBX[15:8] + 1
		// CoreId: core ID =  CPUID_Fn8000001E_EBX[7:0]
		auto coreIdRead = static_cast<int>(thread.extData(0x1e, 1) & 0xff);

		// CPUID_Fn8000001E_ECX, Node Identifiers, Register ..1E_2
		// NodesPerProcessor =  CPUID_Fn8000001E_ECX[10:8]
		// nodeID =  CPUID_Fn8000001E_ECX[7:0]
		auto nodeId = thread.extData(0x1e, 2) & 0xff;

		if (coreIdRead != lastCoreId) {
			coreId++;
		}

		lastCoreId = coreIdRead;

		impl_->appendThread(thread, nodeId, coreId);
	}
}

wm_sensors::SensorChip::Config wm_sensors::hardware::cpu::Amd17Cpu::config() const
{
	Config res = base::config();
	impl_->sensorsCollection().config(res);
	return res;
}

int wm_sensors::hardware::cpu::Amd17Cpu::read(SensorType type, u32 attr, std::size_t channel, double& val) const
{
	impl_->updateSensors();
	int r = impl_->sensorsCollection().read(type, attr, channel, val);
	if (!r) {
		return r;
	}

	return base::read(type, attr, channel, val);
}

int wm_sensors::hardware::cpu::Amd17Cpu::read(
    SensorType type, u32 attr, std::size_t channel, std::string_view& str) const
{
	int r = impl_->sensorsCollection().read(type, attr, channel, str);
	if (!r) {
		return r;
	}

	return base::read(type, attr, channel, str);
}

wm_sensors::hardware::cpu::Amd17Cpu::Impl::Impl(const Amd17Cpu& cpu, SensorChip::Config::ChannelCounts baseCounts)
    : cpu_{cpu}
    , smu_{cpu_.family(), cpu_.model(), cpu_.packageType()}
    , sensors_{std::move(baseCounts)}
    , smn_{0}
    , packagePower_{sensors_.add("Package", SensorType::power, true)}
    , coreTemperatureTctl_{}
    , coreTemperatureTdie_{}
    , coreTemperatureTctlTdie_{}
    , coreVoltage_{}
    , socVoltage_{}
    , busClock_{}
    , ccdsAverageTemperature_{}
    , ccdsMaxTemperature_{}
    , tclTemperatureOffset_{std::numeric_limits<float>::quiet_NaN()}
    , lastPwrTime_{}
{
	detectOptionalSensors();
}

void wm_sensors::hardware::cpu::Amd17Cpu::Impl::detectOptionalSensors()
{
	auto tctlOffsetIt =
	    std::find_if(std::begin(tctl_offset_table), std::end(tctl_offset_table), [this](const tctl_offset& of) {
		    return of.model == this->cpuid().model() && this->cpuid().name() == of.id;
	    });
	if (tctlOffsetIt != std::end(tctl_offset_table)) {
		coreTemperatureTctl_ = sensors_.add("Core (Tctl)", SensorType::temp, true);
		coreTemperatureTdie_ = sensors_.add("Core (Tdie)", SensorType::temp, true);
		tclTemperatureOffset_ = tctlOffsetIt->offset;
	} else {
		coreTemperatureTctlTdie_ = sensors_.add("Core (Tctl/Tdie)", SensorType::temp, true);
	}

	if (smu_.isPmTableLayoutDefined()) {
		for (const auto& sensor: smu_.pmTableStructure()) {
			smuSensors_[sensor.first] =
			    std::make_pair(sensor.second, sensors_.add(std::string(sensor.second.name), sensor.second.type, true));
		}
	}

	bool supportsPerCcdTemperatures = false;
	switch (cpu_.model()) {
		case 0x31: // Threadripper 3000.
		{
			sviPlane0Offset_ = F17H_M01H_SVI + 0x14;
			sviPlane1Offset_ = F17H_M01H_SVI + 0x10;
			supportsPerCcdTemperatures = true;
			break;
		}
		case 0x71: // Zen 2.
		case 0x21: // Zen 3.
		{
			sviPlane0Offset_ = F17H_M01H_SVI + 0x10;
			sviPlane1Offset_ = F17H_M01H_SVI + 0xC;
			supportsPerCcdTemperatures = true;
			break;
		}
		default: // Zen and Zen+.
		{
			sviPlane0Offset_ = F17H_M01H_SVI + 0xC;
			sviPlane1Offset_ = F17H_M01H_SVI + 0x10;
			break;
		}
	}

	hardware::impl::GlobalMutexLock pciLock{hardware::impl::GlobalMutex::PCIBus, std::chrono::milliseconds(10)};
	wm_sensors::impl::ThreadGroupAffinityGuard affinityGuard{cpu_.cpu0IdData().affinity()};

	if (supportsPerCcdTemperatures) {
		const CCDInfo ccdInf = ccdInfo(cpu_.family(), cpu_.model());
		ccdOffset_ = ccdInf.offset;

		auto ccdMask = availableCCDMask(smn_, ccdInf);
		for (unsigned i = 0; i < sizeof(ccdMask) * 4; i++) {
			if (utility::is_bit_set(ccdMask, i)) {
				ccdSensors_.push_back({i, sensors_.add(fmt::format("CCD{0} (Tdie)", i), SensorType::temp, true)});
			}
		}

		// No need to get the max / average ccds temp if there is only one CCD.
		if (ccdSensors_.size() > 1) {
			ccdsMaxTemperature_ = sensors_.add("CCDs Max (Tdie)", SensorType::temp, true);
			ccdsAverageTemperature_ = sensors_.add("CCDs Average (Tdie)", SensorType::temp, true);
		}
	}

	if (timeStampCounterMultiplier() > 0) {
		busClock_ = sensors_.add("Bus Speed", SensorType::frequency, true);
	}

	u32 smuSvi0Tfn = smn_.read(F17H_M01H_SVI + 0x8);
	if ((smuSvi0Tfn & 0x01) == 0) {
		coreVoltage_ = sensors_.add("Core (SVI2 TFN)", SensorType::voltage, true);
	}

	// SoC (0x02), not every Zen cpu has this voltage.
	if (cpu_.model() == 0x11 || cpu_.model() == 0x21 || cpu_.model() == 0x71 || cpu_.model() == 0x31 ||
	    (smuSvi0Tfn & 0x02) == 0) {
		socVoltage_ = sensors_.add("SoC (SVI2 TFN)", SensorType::voltage, true);
	}
}

const CPUIDData* wm_sensors::hardware::cpu::Amd17Cpu::Impl::firstThreadData() const
{
	if (nodes_.empty()) {
		return nullptr;
	}

	const NumaNode& node = nodes().front();
	if (node.cores().empty()) {
		return nullptr;
	}

	const Core& core = node.cores().front();
	if (core.threads().empty()) {
		return nullptr;
	}

	return core.threads().front();
}

double wm_sensors::hardware::cpu::Amd17Cpu::Impl::timeStampCounterMultiplier()
{
	impl::Ring0::MSRValue pstate0;
	impl::Ring0::instance().readMSR(MSR_PSTATE_0, pstate0);
	unsigned cpuDfsId = (pstate0.reg.eax >> 8) & 0x3f;
	unsigned cpuFid = pstate0.reg.eax & 0xff;
	return 2.0 * cpuFid / cpuDfsId;
}

void wm_sensors::hardware::cpu::Amd17Cpu::Impl::updateSensors()
{
	if (std::chrono::steady_clock::now() - lastPwrTime_ < updateTimeout) {
		return;
	}

	const CPUIDData* cpuId = firstThreadData();
	if (!cpuId) {
		return;
	}

	auto& ring0 = Ring0::instance();
	wm_sensors::impl::ThreadGroupAffinityGuard affinityGuard{cpuId->affinity()};

	// MSRC001_0299
	// TU [19:16]
	// ESU [12:8] -> Unit 15.3 micro Joule per increment
	// PU [3:0]
	Ring0::MSRValue tmpMSR;
	ring0.readMSR(MSR_PWR_UNIT, tmpMSR);

	// MSRC001_029B
	// total_energy [31:0]
	const auto sampleTime = std::chrono::steady_clock::now();
	ring0.readMSR(MSR_PKG_ENERGY_STAT, tmpMSR);

	unsigned totalEnergy = tmpMSR.reg.eax;

	u32 smuSvi0Tfn = 0;
	unsigned smuSvi0TelPlane0 = 0;
	unsigned smuSvi0TelPlane1 = 0;

	hardware::impl::GlobalMutexTryLock pciLock{hardware::impl::GlobalMutex::PCIBus, std::chrono::milliseconds(10)};
	if (pciLock.succeded()) {
		// THM_TCON_CUR_TMP
		// CUR_TEMP [31:21]
		u32 temperature = smn_.read(F17H_M01H_THM_TCON_CUR_TMP);

		// SVI0_TFN_PLANE0 [0]
		// SVI0_TFN_PLANE1 [1]
		smuSvi0Tfn = smn_.read(F17H_M01H_SVI + 0x8);

		// SVI0_PLANE0_VDDCOR [24:16]
		// SVI0_PLANE0_IDDCOR [7:0]
		smuSvi0TelPlane0 = smn_.read(sviPlane0Offset_);

		// SVI0_PLANE1_VDDCOR [24:16]
		// SVI0_PLANE1_IDDCOR [7:0]
		smuSvi0TelPlane1 = smn_.read(sviPlane1Offset_);

		affinityGuard.release(); // TODO refactor blocks

		// power consumption
		// power.Value = (float) ((double)pu * 0.125);
		// esu = 15.3 micro Joule per increment
		if (lastPwrTime_.time_since_epoch().count() == 0) {
			lastPwrTime_ = sampleTime;
			lastPwrValue_ = totalEnergy;
		}

		// ticks diff
		const auto time = sampleTime - lastPwrTime_;
		s64 pwr;
		if (lastPwrValue_ <= totalEnergy)
			pwr = totalEnergy - lastPwrValue_;
		else
			pwr = (0xffffffff - lastPwrValue_) + totalEnergy;

		// update for next sample
		lastPwrTime_ = sampleTime;
		lastPwrValue_ = totalEnergy;

		double energy = 15.3 * static_cast<double>(pwr);
		energy /= static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(time).count());

		if (std::isfinite(energy)) {
			sensors_[packagePower_].value(energy);
		}

		// current temp Bit [31:21]
		// If bit 19 of the Temperature Control register is set, there is an additional offset of 49 degrees
		// C.
		bool tempOffsetFlag = (temperature & F17H_TEMP_OFFSET_FLAG) != 0;
		temperature = (temperature >> 21) * 125;

		float t = static_cast<float>(temperature) * 0.001f;
		if (tempOffsetFlag) {
			t += -49.0f;
		}

		if (std::isnan(tclTemperatureOffset_)) {
			sensors_[coreTemperatureTctlTdie_].value(t);
		} else {
			sensors_[coreTemperatureTctl_].value(t);
			sensors_[coreTemperatureTdie_].value(t + tclTemperatureOffset_);
		}

		// Tested only on R5 3600 & Threadripper 3960X.
		if (!ccdSensors_.empty()) {
			double maxTemp = std::numeric_limits<double>::lowest();
			double tempSum = 0.;

			for (const auto& ccd: ccdSensors_) {
				u32 ccdRawTemp = smn_.read(ZEN_CCD_TEMP(ccdOffset_, ccd.ccdIndex));
				ccdRawTemp &= 0xFFF;
				double ccdTemp = ((ccdRawTemp * 125) - 305000) * 0.001;
				sensors_[ccd.temperature].value(ccdTemp);

				tempSum += ccdTemp;
				if (ccdTemp > maxTemp) {
					maxTemp = ccdTemp;
				}
			}

			sensors_[ccdsMaxTemperature_].value(maxTemp);
			sensors_[ccdsAverageTemperature_].value(tempSum / static_cast<double>(ccdSensors_.size()));
		}
	}

	// voltage
	const double vidStep = 0.00625;
	double vcc;
	unsigned svi0PlaneXVddCor;

	// Core (0x01).
	if (coreVoltage_) {
		svi0PlaneXVddCor = (smuSvi0TelPlane0 >> 16) & 0xff;
		vcc = 1.550 - vidStep * svi0PlaneXVddCor;
		sensors_[coreVoltage_].value(vcc);
	}

	// SoC (0x02), not every Zen cpu has this voltage.
	if (socVoltage_) {
		svi0PlaneXVddCor = (smuSvi0TelPlane1 >> 16) & 0xff;
		vcc = 1.550 - vidStep * svi0PlaneXVddCor;
		sensors_[socVoltage_].value(vcc);
	}

	sensors_[busClock_].value(cpu_.timeStampCounterFrequency() / this->timeStampCounterMultiplier());

	if (!smuSensors_.empty()) {
		std::vector<float> smuData = smu_.pmTable();

		for (auto& sensor: smuSensors_) {
			if (smuData.size() > sensor.first) {
				sensors_[sensor.second.second].value(smuData[sensor.first] * sensor.second.first.scale);
			}
		}
	}

	for (const NumaNode& node: nodes_) {
		node.updateSensors();

		for (const Core& c: node.cores()) {
			c.updateSensors();
		}
	}
}

Core::Core(
    const Amd17Cpu& cpu, int id, wm_sensors::impl::SensorCollection<Sensor>& sensors, const SensorHandle busSpeedSensor)
    : cpu_{cpu}
    , sensors_{sensors}
    , id_{id}
    , clock_{sensors.add(fmt::format("Core #{0}", id_), SensorType::frequency, true)}
    , multiplier_{sensors.add(fmt::format("Core #{0}", id_), SensorType::raw, true)}
    , power_{sensors.add(fmt::format("Core #{0} (SMU)", id_), SensorType::power, true)}
    , vcore_{sensors.add(fmt::format("Core #{0} VID", id_), SensorType::voltage, true)}
    , busSpeed_{busSpeedSensor}
{
}

void Core::updateSensors() const
{
	// CPUID cpu = threads.FirstOrDefault();
	const auto cpu = threads_[0];
	if (!cpu) { // TODO seems impossible
		return;
	}

	int curCpuVid;
	int curCpuDfsId;
	int curCpuFid;
	unsigned totalEnergy;

	const auto sampleTime = std::chrono::steady_clock::now();
	auto& ring0 = Ring0::instance();
	{
		wm_sensors::impl::ThreadGroupAffinityGuard affinityLock{cpu->affinity()};

		// MSRC001_0299
		// TU [19:16]
		// ESU [12:8] -> Unit 15.3 micro Joule per increment
		// PU [3:0]
		Ring0::MSRValue tmpMSR;
		ring0.readMSR(MSR_PWR_UNIT, tmpMSR);

		// MSRC001_029A
		// total_energy [31:0]
		ring0.readMSR(MSR_CORE_ENERGY_STAT, tmpMSR);
		totalEnergy = tmpMSR.reg.eax;

		// MSRC001_0293
		// CurHwPstate [24:22]
		// CurCpuVid [21:14]
		// CurCpuDfsId [13:8]
		// CurCpuFid [7:0]
		ring0.readMSR(MSR_HARDWARE_PSTATE_STATUS, tmpMSR);
		curCpuVid = (int)((tmpMSR.reg.eax >> 14) & 0xff);
		curCpuDfsId = (int)((tmpMSR.reg.eax >> 8) & 0x3f);
		curCpuFid = (int)(tmpMSR.reg.eax & 0xff);

		// MSRC001_0064 + x
		// IddDiv [31:30]
		// IddValue [29:22]
		// CpuVid [21:14]
		// CpuDfsId [13:8]
		// CpuFid [7:0]
		// Ring0.ReadMsr(MSR_PSTATE_0 + (uint)CurHwPstate, out eax, out edx);
		// int IddDiv = (int)((eax >> 30) & 0x03);
		// int IddValue = (int)((eax >> 22) & 0xff);
		// int CpuVid = (int)((eax >> 14) & 0xff);
	}

	// clock
	// CoreCOF is (Core::X86::Msr::PStateDef[CpuFid[7:0]] / Core::X86::Msr::PStateDef[CpuDfsId]) * 200
	double clock = 200.0;
	if (busSpeed_ && sensors_[busSpeed_].value() > 0) {
		clock = sensors_[busSpeed_].value() * 2;
	}

	sensors_[clock_].value(curCpuFid / (double)curCpuDfsId * clock);

	// multiplier
	sensors_[multiplier_].value(curCpuFid / (double)curCpuDfsId * 2.0);

	// Voltage
	const double vidStep = 0.00625;
	double vcc = 1.550 - vidStep * curCpuVid;
	sensors_[vcore_].value(vcc);

	// power consumption
	// power.Value = (float) ((double)pu * 0.125);
	// esu = 15.3 micro Joule per increment
	if (lastPwrTime_.time_since_epoch().count() == 0) {
		lastPwrTime_ = sampleTime;
		lastPwrValue_ = totalEnergy;
	}

	// ticks diff
	const auto time = sampleTime - lastPwrTime_;
	long long pwr = totalEnergy + (lastPwrValue_ <= totalEnergy ? 0 - lastPwrValue_ : 0xffffffff - lastPwrValue_);

	// update for next sample
	lastPwrTime_ = sampleTime;
	lastPwrValue_ = totalEnergy;

	double energy = 15.3 * static_cast<double>(pwr);
	energy /= static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(time).count());

	if (std::isfinite(energy)) {
		sensors_[power_].value(energy);
	}
}

NumaNode::NumaNode(const Amd17Cpu& cpu, unsigned id)
    : cpu_{cpu}
    , nodeId_{id}
{
}

void NumaNode::appendThread(
    const CPUIDData& thread, int coreId, wm_sensors::impl::SensorCollection<Sensor>& sensors,
    const SensorHandle busSpeedSensor)
{
	auto it = std::find_if(cores_.begin(), cores_.end(), [coreId](const Core& c) { return c.id() == coreId; });
	// const Amd17Cpu& cpu, int id, wm_sensors::impl::SensorCollection<Sensor>& sensors, const Sensor* busSpeedSensor
	Core& core = (it == cores_.end() ? cores_.emplace_back(cpu_, coreId, sensors, busSpeedSensor) : *it);

	// if (thread != null)
	core.threads().push_back(&thread);
}

void wm_sensors::hardware::cpu::Amd17Cpu::Impl::appendThread(const CPUIDData& thread, unsigned numaId, int coreId)
{
	const auto it =
	    std::find_if(nodes_.begin(), nodes_.end(), [numaId](const NumaNode& n) { return n.nodeId() == numaId; });
	NumaNode& node = it == nodes_.end() ? nodes_.emplace_back(cpu_, numaId) : *it;
	// if (thread != null) {
	node.appendThread(thread, coreId, sensors_, busClock_);
	//}
}
