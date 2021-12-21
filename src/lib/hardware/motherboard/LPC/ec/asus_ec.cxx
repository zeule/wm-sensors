// SPDX-License-Identifier: LGPL-3.0+

#include "./asus_ec.hxx"

#include "./ec.hxx"
#include "../../../../utility/utility.hxx"
#include "../../../../utility/unaligned.hxx"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <limits>
#include <map>
#include <mutex>
#include <set>
#include <type_traits>
#include <vector>

namespace {
	using namespace wm_sensors::stdtypes;
	using wm_sensors::SensorType;
	using wm_sensors::hardware::motherboard::Model;
	using wm_sensors::hardware::motherboard::lpc::ec::EC;

	using namespace std::chrono_literals;

	const auto updateTimeout = 1s;

	constexpr const std::size_t invalidIndex = static_cast<std::size_t>(-1);

	template <typename T>
	constexpr unsigned long bit(T n) requires std::is_integral_v<T>
	{
		return 1ul << n;
	};

	template <typename... ArgTypes>
	constexpr unsigned long bits(ArgTypes... args);

	template <typename T, typename... ArgTypes>
	constexpr unsigned long bits(T t, ArgTypes... args) requires std::is_enum_v<T>
	{
		return bit(wm_sensors::utility::to_underlying(t)) | bits(args...);
	}

	template <>
	constexpr unsigned long bits()
	{
		return 0UL;
	}

	enum ec_sensors
	{
		/** chipset temperature [℃] */
		sensorTempChipset,
		/** CPU temperature [℃] */
		sensorTempCPU,
		/** motherboard temperature [℃] */
		sensorTempMB,
		/** "T_Sensor" temperature sensor reading [℃] */
		sensorTempTSensor,
		/** VRM temperature [℃] */
		sensorTempVrm,
		/** CPU_Opt fan [RPM] */
		sensorFanCPUOpt,
		/** VRM heat sink fan [RPM] */
		sensorFanVrmHS,
		/** Chipset fan [RPM] */
		sensorFanChipset,
		/** Water flow sensor reading [RPM] */
		sensorFanWaterFlow,
		/** CPU current [A] */
		sensorCurrCPU,
		/** "Water_In" temperature sensor reading [℃] */
		sensorTempWaterIn,
		/** "Water_Out" temperature sensor reading [℃] */
		sensorTempWaterOut,
		sensorMax
	};


	union SensorAddress {
		u32 value;
		struct {
			u8 index;
			u8 bank;
			u8 size;
			u8 dummy;
		} components;

		u16 reg() const {
			return static_cast<u16>((components.bank << 8) + components.index);
		}
	};

	u8 regBank(u16 reg)
	{
		return wm_sensors::utility::hibyte(reg);
	}

	u8 regIndex(u16 reg)
	{
		return wm_sensors::utility::lobyte(reg);
	}

	int operator<=>(const SensorAddress& left, const SensorAddress& right)
	{
		if (left.components.bank != right.components.bank) {
			return left.components.bank - right.components.bank;
		}
		return left.components.index - right.components.index;

	}

	struct ECSensorInfo {
		ECSensorInfo(const char* label, wm_sensors::SensorType type, u8 size, u8 bank, u8 index);

		const char* label;
		wm_sensors::SensorType type;
		SensorAddress addr;
	};

	ECSensorInfo::ECSensorInfo(const char* tlabel, SensorType ttype, u8 size, u8 bank, u8 index)
	    : label{tlabel}, type{ttype}, addr{static_cast<u32>((size << 16) + (bank << 8) + index)}
	{
	}

	/**
	 * All the known sensors for ASUS EC controllers
	 */
	const std::map<ec_sensors, ECSensorInfo> knownECSensors = {
	    {sensorTempChipset, {"Chipset", SensorType::temp, 1, 0x00, 0x3a}},
	    {sensorTempCPU, {"CPU", SensorType::temp, 1, 0x00, 0x3b}},
	    {sensorTempMB, {"Motherboard", SensorType::temp, 1, 0x00, 0x3c}},
	    {sensorTempTSensor, {"T_Sensor", SensorType::temp, 1, 0x00, 0x3d}},
	    {sensorTempVrm, {"VRM", SensorType::temp, 1, 0x00, 0x3e}},
	    {sensorFanCPUOpt, {"CPU_Opt", SensorType::fan, 2, 0x00, 0xb0}},
	    {sensorFanVrmHS, {"VRM HS", SensorType::fan, 2, 0x00, 0xb2}},
	    {sensorFanChipset, {"Chipset", SensorType::fan, 2, 0x00, 0xb4}},
	    {sensorFanWaterFlow, {"Water_Flow", SensorType::fan, 2, 0x00, 0xbc}},
	    {sensorCurrCPU, {"CPU", SensorType::curr, 1, 0x00, 0xf4}},
	    {sensorTempWaterIn, {"Water_In", SensorType::temp, 1, 0x01, 0x00}},
	    {sensorTempWaterOut, {"Water_Out", SensorType::temp, 1, 0x01, 0x01}},
	};

	/* Shortcuts for common combinations */
	constexpr const auto sensorsTempChipsetCPUMB = bits(sensorTempChipset, sensorTempCPU, sensorTempMB);
	constexpr const auto sensorsTempWater = bits(sensorTempWaterIn, sensorTempWaterOut);

	const std::map<Model, unsigned long> boardSensors = {
	    {Model::PRIME_X570_PRO, sensorsTempChipsetCPUMB | bits(sensorTempVrm, sensorTempTSensor, sensorFanChipset)},
	    {Model::PRO_WS_X570_ACE, sensorsTempChipsetCPUMB | bits(sensorTempVrm, sensorFanChipset, sensorCurrCPU)},
	    {Model::ROG_CROSSHAIR_VIII_HERO, sensorsTempChipsetCPUMB | sensorsTempWater |
	                                         bits(
	                                             sensorTempTSensor, sensorTempVrm, sensorFanCPUOpt, sensorFanChipset,
	                                             sensorFanWaterFlow, sensorCurrCPU)},
	    {Model::ROG_CROSSHAIR_VIII_HERO_WIFI, sensorsTempChipsetCPUMB | sensorsTempWater |
	                                              bits(
	                                                  sensorTempTSensor, sensorTempVrm, sensorFanCPUOpt,
	                                                  sensorFanChipset, sensorFanWaterFlow, sensorCurrCPU)},
	    {Model::ROG_CROSSHAIR_VIII_DARK_HERO,
	     sensorsTempChipsetCPUMB | sensorsTempWater |
	         bits(sensorTempTSensor, sensorTempVrm, sensorFanCPUOpt, sensorFanWaterFlow, sensorCurrCPU)},
	    {Model::ROG_CROSSHAIR_VIII_FORMULA,
	     sensorsTempChipsetCPUMB |
	         bits(sensorTempTSensor, sensorTempVrm, sensorFanCPUOpt, sensorFanChipset, sensorCurrCPU)},
	    {Model::ROG_CROSSHAIR_VIII_IMPACT,
	     sensorsTempChipsetCPUMB | bits(sensorTempTSensor, sensorTempVrm, sensorFanChipset, sensorCurrCPU)},
	    {Model::ROG_STRIX_B550_E_GAMING,
	     sensorsTempChipsetCPUMB | bits(sensorTempTSensor, sensorTempVrm, sensorFanCPUOpt)},
	    {Model::ROG_STRIX_B550_I_GAMING,
	     sensorsTempChipsetCPUMB | bits(sensorTempTSensor, sensorTempVrm, sensorFanVrmHS, sensorCurrCPU)},
	    {Model::ROG_STRIX_X570_E_GAMING,
	     sensorsTempChipsetCPUMB | bits(sensorTempTSensor, sensorTempVrm, sensorFanChipset, sensorCurrCPU)},
	    {Model::ROG_STRIX_X570_F_GAMING, sensorsTempChipsetCPUMB | bits(sensorTempTSensor, sensorFanChipset)},
	    {Model::ROG_STRIX_X570_I_GAMING, bits(sensorTempTSensor, sensorFanVrmHS, sensorFanChipset, sensorCurrCPU)},
	};

		const u8 asusECBankRegister = 0xff;

/*
 * Switches ASUS EC banks.
 */
void ecBankSwitch(u8 bank, u8 *old)
{
	if (old) {
		*old = EC::first().read(asusECBankRegister);
	}
	if ((old && (*old != bank))) {
		EC::first().write(asusECBankRegister, bank);
	}
}
} // namespace


struct wm_sensors::hardware::motherboard::lpc::ec::AsusEC::Impl {
	Impl(Model model);

	DELETE_COPY_CTOR_AND_ASSIGNMENT(Impl)

	struct SensorState {
		const ECSensorInfo* info;
		int cachedValue;
	};

	std::size_t sensorIndex(SensorType type, std::size_t channel) const;
	void update();
//	void ecBlockRead();

	std::vector<SensorState> state;
	std::chrono::steady_clock::time_point lastUpdate;
	std::vector<u16> registers;
	std::vector<u8> readBuffer;
	u8 nrBanks;
	std::mutex updateMutex;
};

wm_sensors::hardware::motherboard::lpc::ec::AsusEC::Impl::Impl(Model model)
	: lastUpdate{std::chrono::steady_clock::now() - 2 * updateTimeout}
{
	std::bitset<sensorMax> sensors{boardSensors.at(model)};
	// we collect all sensors for this board...
	std::size_t nrRegisters = 0;
	std::set<u8> banks;
	for (std::size_t i = 0 ; i < sensors.size(); ++i) {
		if (sensors.test(i)) {
			const auto& si = knownECSensors.at(static_cast<ec_sensors>(i));
			state.push_back({&si, 0});
			nrRegisters += si.addr.components.size;
			banks.insert(si.addr.components.bank);
		}
	}

	// ...and sort them by address, which implies ordered by bank first
	std::sort(state.begin(), state.end(), [](const SensorState& left, const SensorState& right) {
		return (left.info->addr <=> right.info->addr) < 0;
	});

	// now collect sensor registers
	assert(banks.size() <= std::numeric_limits<u8>::max());
	nrBanks = static_cast<u8>(banks.size());
	registers.reserve(nrRegisters);
	state.reserve(state.size());
	readBuffer.resize(nrRegisters);

	for (const auto& st: state) {
		for (u8 regI = 0; regI < st.info->addr.components.size; ++regI) {
			registers.push_back(static_cast<u16>(st.info->addr.reg() + regI));
		}
	}

	const auto sensorValue = [](const ECSensorInfo& si, const u8* data) -> int {
		switch (si.addr.components.size) {
			case 1: return static_cast<s8>(*data);
			case 2: return utility::get_unaligned_be<s16>(data);
			case 4: return utility::get_unaligned_be<s32>(data);
			default: return 0;
		};
	};

	const u8* data = &readBuffer.front();
	for (auto& s: state) {
		const ECSensorInfo& si = *s.info;
		s.cachedValue = sensorValue(si, data);
		data += si.addr.components.size;
	}
}

std::size_t wm_sensors::hardware::motherboard::lpc::ec::AsusEC::Impl::sensorIndex(
    wm_sensors::SensorType type, std::size_t channel) const
{
	for (std::size_t i = 0; i < state.size(); i++) {
		if (state[i].info->type == type) {
			if (channel == 0)
				return i;
			channel--;
		}
	}
	return invalidIndex;
}

void wm_sensors::hardware::motherboard::lpc::ec::AsusEC::Impl::update()
{
	std::lock_guard<std::mutex> lock{updateMutex};

	u8 bank = 0, prevBank;
	ecBankSwitch(bank, &prevBank);

	if (prevBank) {
		/* oops... somebody else is working with the EC too */
		spdlog::warn("Concurrent access to the ACPI EC "
			      "detected.\nRace condition possible.");
	}

	/*
	 * read registers minimizing bank switches.
	 */
	for (std::size_t i = 0; i < registers.size(); i++) {
		// registers are sorted by bank
		if (regBank(registers[i]) > bank) {
			ecBankSwitch(regBank(registers[i]), nullptr);
			bank = regBank(registers[i]);
		}
		readBuffer[i] = EC::first().read(regIndex(registers[i]));
	}

	ecBankSwitch(prevBank, nullptr);
}


wm_sensors::hardware::motherboard::lpc::ec::AsusEC::AsusEC(motherboard::Model model)
    : base({"ASUS EC", "ec", BusType::ISA})
	, impl_{std::make_unique<Impl>(model)}
{
}

wm_sensors::hardware::motherboard::lpc::ec::AsusEC::~AsusEC() = default;

bool wm_sensors::hardware::motherboard::lpc::ec::AsusEC::isAvailable(motherboard::Model model)
{
	return boardSensors.count(model) > 0;
}

wm_sensors::SensorChip::Config wm_sensors::hardware::motherboard::lpc::ec::AsusEC::config() const
{
	const std::map<SensorType, u32> attributes = {
		{SensorType::chip, attributes::chip_register_tz},
		{SensorType::temp, attributes::temp_input | attributes::temp_label},
		{SensorType::in, attributes::in_input | attributes::in_label},
		{SensorType::curr, attributes::curr_input | attributes::curr_label},
		{SensorType::fan, attributes::fan_input | attributes::fan_label},
	};

	Config res;

	for (const auto& st: impl_->state) {
		res.sensors[st.info->type].channelAttributes.push_back(attributes.at(st.info->type));
	}

	res.sensors[SensorType::chip].channelAttributes.push_back(attributes.at(SensorType::chip));

	return res;
}


int wm_sensors::hardware::motherboard::lpc::ec::AsusEC::read(
    SensorType type, u32 /*attr*/, std::size_t channel, double& val) const
{
	if (std::chrono::steady_clock::now() > impl_->lastUpdate + updateTimeout) {
		impl_->update();
	}
	std::size_t ind = impl_->sensorIndex(type, channel);
	if (ind == invalidIndex) {
		return -ENOENT;
	}
	val = impl_->state[ind].cachedValue;
	return 0;
}

int wm_sensors::hardware::motherboard::lpc::ec::AsusEC::read(
    SensorType type, u32 /*attr*/, std::size_t channel, std::string_view& str) const
{
	std::size_t ind = impl_->sensorIndex(type, channel);
	if (ind == invalidIndex) {
		return -ENOENT;
	}
	str = impl_->state[ind].info->label;
	return 0;
}
