// SPDX-License-Identifier: LGPL-3.0+

#include "./ryzen_smu.hxx"

#include "../../../sensor.hxx"
#include "../../impl/ring0.hxx"

#include <cassert>
#include <cmath>
#include <map>
#include <string_view>
#include <thread>

#include "wm-sensors_config.h"

namespace {
	using namespace wm_sensors;

	using hardware::cpu::RyzenSMU;
	using hardware::impl::GlobalMutex;
	using hardware::impl::Ring0;

	const u8 SMU_PCI_ADDR_REG = 0xC4;
	const u8 SMU_PCI_DATA_REG = 0xC8;
	const unsigned SMU_RETRIES_MAX = 8096;

	enum Status
	{
		OK = 0x01,
		Failed = 0xFF,
		UnknownCmd = 0xFE,
		CmdRejectedPrereq = 0xFD,
		CmdRejectedBusy = 0xFC
	};

	const std::map<unsigned, std::map<unsigned, hardware::cpu::RyzenSMU::SmuSensorType>> supportedPmTableVersions{
	    {// Zen Raven Ridge APU.
	     0x001E0004,
	     {
	         {7, {"TDC", SensorType::curr, 1.0f}},
	         {11, {"EDC", SensorType::curr, 1.0f}},
	         //{ 61, {"Core", SensorType::voltage } },
	         //{ 62, {"Core", SensorType::curr, 1.f} },
	         //{ 63, {"Core", SensorType::power, 1.f } },
	         //{ 65, {"SoC", SensorType::voltage } },
	         {66, {"SoC", SensorType::curr, 1.f}},
	         {67, {"SoC", SensorType::power, 1.f}},
	         //{ 96, {"Core #1", SensorType::power } },
	         //{ 97, {"Core #2", SensorType::power } },
	         //{ 98, {"Core #3", SensorType::power } },
	         //{ 99, {"Core #4", SensorType::power } },
	         {108, {"Core #1", SensorType::temp, 1.f}},
	         {109, {"Core #2", SensorType::temp, 1.f}},
	         {110, {"Core #3", SensorType::temp, 1.f}},
	         {111, {"Core #4", SensorType::temp, 1.f}},
	         {150, {"GFX", SensorType::voltage, 1.f}},
	         {151, {"GFX", SensorType::temp, 1.f}},
	         {154, {"GFX", SensorType::frequency, 1.f}},
	         {156, {"GFX", SensorType::load, 1.f}},
	         {166, {"Fabric", SensorType::frequency, 1.f}},
	         {177, {"Uncore", SensorType::frequency, 1.f}},
	         {178, {"Memory", SensorType::frequency, 1.f}},
	         {342, {"Displays", SensorType::raw, 1.f}},
	     }},
	    {// Zen 2.
	     0x00240903,
	     {
	         {15, {"TDC", SensorType::curr, 1.f}},
	         {21, {"EDC", SensorType::curr, 1.f}},
	         {48, {"Fabric", SensorType::frequency, 1.f}},
	         {50, {"Uncore", SensorType::frequency, 1.f}},
	         {51, {"Memory", SensorType::frequency, 1.f}},
	         {115, {"SoC", SensorType::temp, 1.f}},
	         //{ 66, {"Bus Speed", SensorType::frequency, 1.f } },
	         //{ 188, {"Core #1", SensorType::frequency, 1000.f } },
	         //{ 189, {"Core #2", SensorType::frequency, 1000.f } },
	         //{ 190, {"Core #3", SensorType::frequency, 1000.f } },
	         //{ 191, {"Core #4", SensorType::frequency, 1000.f } },
	         //{ 192, {"Core #5", SensorType::frequency, 1000.f } },
	         //{ 193, {"Core #6", SensorType::frequency, 1000.f } },
	     }},
	    {// Zen 3.
	     0x00380805,
	     {
	         // TDC and EDC don't match the HWiNFO values
	         //{ 15, {"TDC", Type = SensorType.Current, Scale = 1 } },
	         //{ 21, {"EDC", Type = SensorType.Current, Scale = 1 } },
	         {48, {"Fabric", SensorType::frequency, 1.f}},
	         {50, {"Uncore", SensorType::frequency, 1.f}},
	         {51, {"Memory", SensorType::frequency, 1.f}},
	         //{ 115, {"SoC", Type = SensorType::temp, Scale = 1 } },
	         //{ 66, {"Bus Speed", Type = SensorType.Clock, Scale = 1 } },
	         //{ 188, {"Core #1", Type = SensorType.Clock, Scale = 1000 } },
	         //{ 189, {"Core #2", Type = SensorType.Clock, Scale = 1000 } },
	         //{ 190, {"Core #3", Type = SensorType.Clock, Scale = 1000 } },
	         //{ 191, {"Core #4", Type = SensorType.Clock, Scale = 1000 } },
	         //{ 192, {"Core #5", Type = SensorType.Clock, Scale = 1000 } },
	         //{ 193, {"Core #6", Type = SensorType.Clock, Scale = 1000 } },
	     }},
	};

	RyzenSMU::CpuCodeName cpuCodeName(u32 family, u32 model, u32 packageType)
	{
		using CpuCodeName = RyzenSMU::CpuCodeName;
		switch (family) {
			case 0x17:
				switch (model) {
					case 0x01: {
						return packageType == 7 ? CpuCodeName::Threadripper : CpuCodeName::SummitRidge;
					}
					case 0x08: {
						return packageType == 7 ? CpuCodeName::Colfax : CpuCodeName::PinnacleRidge;
					}
					case 0x11: {
						return CpuCodeName::RavenRidge;
					}
					case 0x18: {
						return packageType == 2 ? CpuCodeName::RavenRidge2 : CpuCodeName::Picasso;
					}
					case 0x20: {
						return CpuCodeName::Dali;
					}
					case 0x31: {
						return CpuCodeName::CastlePeak;
					}
					case 0x60: {
						return CpuCodeName::Renoir;
					}
					case 0x71: {
						return CpuCodeName::Matisse;
					}
					case 0x90: {
						return CpuCodeName::Vangogh;
					}
					default: {
						return CpuCodeName::Undefined;
					}
				}
			case 0x19:
				switch (model) {
					case 0x00: {
						return CpuCodeName::Milan;
					}
					case 0x20:
					case 0x21: {
						return CpuCodeName::Vermeer;
					}
					case 0x40: {
						return CpuCodeName::Rembrandt;
					}
					case 0x50: {
						return CpuCodeName::Cezanne;
					}
					default: {
						return CpuCodeName::Undefined;
					}
				}
			default: return CpuCodeName::Undefined;
		}
	}

	RyzenSMU::Address smuAddress(RyzenSMU::CpuCodeName codeName)
	{
		using CpuCodeName = RyzenSMU::CpuCodeName;
		switch (codeName) {
			case CpuCodeName::CastlePeak:
			case CpuCodeName::Matisse:
			case CpuCodeName::Vermeer:
				return {
				    0x3B10524,
				    0x3B10570,
				    0x3B10A40,
				};
			case CpuCodeName::Colfax:
			case CpuCodeName::SummitRidge:
			case CpuCodeName::Threadripper:
			case CpuCodeName::PinnacleRidge:
				return {
				    0x3B1051C,
				    0x3B10568,
				    0x3B10590,
				};
			case CpuCodeName::Renoir:
			case CpuCodeName::Picasso:
			case CpuCodeName::RavenRidge:
			case CpuCodeName::RavenRidge2:
			case CpuCodeName::Dali:
				return {
				    0x3B10A20,
				    0x3B10A80,
				    0x3B10A88,
				};
			default: {
				return {0, 0, 0};
			}
		}
	}

	bool waitForMutex(std::mutex& m, int milliseconds)
	{
		while (milliseconds > 0) {
			if (m.try_lock()) {
				return true;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			milliseconds -= 10;
		}
		return false;
	}

	void writePCIReg(u32 addr, u32 data, bool acquirePciMutexLock = true)
	{
		if (!acquirePciMutexLock ||
		    Ring0::instance().acquireMutex(GlobalMutex::PCIBus, std::chrono::milliseconds(10))) {
			if (Ring0::instance().writePciConfig(0x00, SMU_PCI_ADDR_REG, addr)) {
				Ring0::instance().writePciConfig(0x00, SMU_PCI_DATA_REG, data);
			}

			if (acquirePciMutexLock) {
				Ring0::instance().releaseMutex(GlobalMutex::PCIBus);
			}
		}
	}

	bool readPCIReg(unsigned addr, unsigned& datum, bool acquirePciMutexLock = true)
	{
		bool read = false;

		if (!acquirePciMutexLock ||
		    Ring0::instance().acquireMutex(GlobalMutex::PCIBus, std::chrono::milliseconds(10))) {
			if (Ring0::instance().writePciConfig(0x00, SMU_PCI_ADDR_REG, addr)) {
				read = Ring0::instance().readPciConfig(0x00, SMU_PCI_DATA_REG, datum);
			}

			if (acquirePciMutexLock) {
				Ring0::instance().releaseMutex(GlobalMutex::PCIBus);
			}
		}

		return read;
	}

} // namespace

wm_sensors::hardware::cpu::RyzenSMU::RyzenSMU(u32 family, u32 model, u32 packageType)
    : cpuCodeName_{cpuCodeName(family, model, packageType)}
    , addr_{smuAddress(cpuCodeName_)}
    , mutex_{}
    , supportedCPU_{sizeof(void*) == 8 && addr_.cmd > 0}
    , dramBaseAddr_{0}
    , pmTableSize_{0}
    , pmTableSizeAlt_{0}
    , pmTableVersion_{0}
{
	if (supportedCPU_) {
		// InpOut.Open();

		setupPmTableAddrAndSize();
	}
}

const std::map<unsigned, wm_sensors::hardware::cpu::RyzenSMU::SmuSensorType>&
wm_sensors::hardware::cpu::RyzenSMU::pmTableStructure() const
{
	return supportedPmTableVersions.at(pmTableVersion_);
}

std::vector<float> wm_sensors::hardware::cpu::RyzenSMU::pmTable()
{
	if (!supportedCPU_ || !transferTableToDRAM())
		return {0.f};

	std::vector<float> table = readDRAMToArray();

	// Fix for Zen+ empty values on first call.
	if (table.empty() || table.front() == 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		transferTableToDRAM();
		return readDRAMToArray();
	}

	return table;
}

bool wm_sensors::hardware::cpu::RyzenSMU::setupPmTableAddrAndSize()
{
	if (pmTableSize_ == 0)
		setupPmTableSize();

	if (dramBaseAddr_ == 0)
		setupDramBaseAddr();

	return dramBaseAddr_ != 0 && pmTableSize_ != 0;
}

bool wm_sensors::hardware::cpu::RyzenSMU::pmTableVersion(u32& version)
{
	unsigned args[SMU_REQ_MAX_ARGS] = {0};

	unsigned fn;

	switch (cpuCodeName_) {
		case CpuCodeName::RavenRidge:
		case CpuCodeName::Picasso: {
			fn = 0x0c;
			break;
		}
		case CpuCodeName::Matisse:
		case CpuCodeName::Vermeer: {
			fn = 0x08;
			break;
		}
		case CpuCodeName::Renoir: {
			fn = 0x06;
			break;
		}
		default: {
			return false;
		}
	}

	bool ret = sendCommand(fn, args);
	version = args[0];

	return ret;
}

void wm_sensors::hardware::cpu::RyzenSMU::setupPmTableSize()
{
	if (!pmTableVersion(pmTableVersion_)) {
		return;
	}

	switch (cpuCodeName_) {
		case CpuCodeName::Matisse: {
			switch (pmTableVersion_) {
				case 0x240902: {
					pmTableSize_ = 0x514;
					break;
				}
				case 0x240903: {
					pmTableSize_ = 0x518;
					break;
				}
				case 0x240802: {
					pmTableSize_ = 0x7E0;
					break;
				}
				case 0x240803: {
					pmTableSize_ = 0x7E4;
					break;
				}
				default: {
					return;
				}
			}

			break;
		}
		case CpuCodeName::Vermeer: {
			switch (pmTableVersion_) {
				case 0x2D0903: {
					pmTableSize_ = 0x594;
					break;
				}
				case 0x380904: {
					pmTableSize_ = 0x5A4;
					break;
				}
				case 0x380905: {
					pmTableSize_ = 0x5D0;
					break;
				}
				case 0x2D0803: {
					pmTableSize_ = 0x894;
					break;
				}
				case 0x380804: {
					pmTableSize_ = 0x8A4;
					break;
				}
				case 0x380805: {
					pmTableSize_ = 0x8F0;
					break;
				}
				default: {
					return;
				}
			}

			break;
		}
		case CpuCodeName::Renoir: {
			switch (pmTableVersion_) {
				case 0x370000: {
					pmTableSize_ = 0x794;
					break;
				}
				case 0x370001: {
					pmTableSize_ = 0x884;
					break;
				}
				case 0x370002:
				case 0x370003: {
					pmTableSize_ = 0x88C;
					break;
				}
				case 0x370004: {
					pmTableSize_ = 0x8AC;
					break;
				}
				case 0x370005: {
					pmTableSize_ = 0x8C8;
					break;
				}
				default: {
					return;
				}
			}

			break;
		}
		case CpuCodeName::Cezanne: {
			switch (pmTableVersion_) {
				case 0x400005: {
					pmTableSize_ = 0x944;
					break;
				}
				default: {
					return;
				}
			}

			break;
		}
		case CpuCodeName::Picasso:
		case CpuCodeName::RavenRidge:
		case CpuCodeName::RavenRidge2: {
			pmTableSizeAlt_ = 0xA4;
			pmTableSize_ = 0x608 + pmTableSizeAlt_;
			break;
		}
		default: {
			return;
		}
	}
}

void wm_sensors::hardware::cpu::RyzenSMU::setupDramBaseAddr()
{
	u32 fn[3] = {0, 0, 0};

	switch (cpuCodeName_) {
		case CpuCodeName::Vermeer:
		case CpuCodeName::Matisse:
		case CpuCodeName::CastlePeak: {
			fn[0] = 0x06;
			setupAddrClass1(fn[0]);

			return;
		}
		case CpuCodeName::Renoir: {
			fn[0] = 0x66;
			setupAddrClass1(fn[0]);

			return;
		}
		case CpuCodeName::Colfax:
		case CpuCodeName::PinnacleRidge: {
			fn[0] = 0x0b;
			fn[1] = 0x0c;
			setupAddrClass2(fn);

			return;
		}
		case CpuCodeName::Dali:
		case CpuCodeName::Picasso:
		case CpuCodeName::RavenRidge:
		case CpuCodeName::RavenRidge2: {
			fn[0] = 0x0a;
			fn[1] = 0x3d;
			fn[2] = 0x0b;
			setupAddrClass3(fn);

			return;
		}
		default: {
			return;
		}
	}
}

void wm_sensors::hardware::cpu::RyzenSMU::setupAddrClass1(u32 fn)
{
	u32 args[SMU_REQ_MAX_ARGS] = {1, 1};

	if (!sendCommand(fn, args)) {
		return;
	}

	dramBaseAddr_ = args[0]; // TODO | (args[1] << 32);
}

void wm_sensors::hardware::cpu::RyzenSMU::setupAddrClass2(u32 fn[2])
{
	u32 args[SMU_REQ_MAX_ARGS] = {0, 0, 0, 0, 0, 0};

	if (!sendCommand(fn[0], args)) {
		return;
	}

	memset(args, 0, sizeof(args));
	if (!sendCommand(fn[1], args)) {
		return;
	}

	dramBaseAddr_ = args[0];
}

void wm_sensors::hardware::cpu::RyzenSMU::setupAddrClass3(u32 fn[3])
{
	u32 parts[2] = {0, 0};

	// == Part 1 ==
	u32 args[SMU_REQ_MAX_ARGS] = {3};
	if (!sendCommand(fn[0], args)) {
		return;
	}

	memset(args, 0, sizeof(args));
	args[0] = 3;
	if (!sendCommand(fn[2], args)) {
		return;
	}

	// 1st Base.
	parts[0] = args[0];
	// == Part 1 End ==

	// == Part 2 ==
	memset(args, 0, sizeof(args));
	args[0] = 3;
	if (!sendCommand(fn[1], args)) {
		return;
	}

	memset(args, 0, sizeof(args));
	args[0] = 5;
	if (!sendCommand(fn[1], args)) {
		return;
	}

	memset(args, 0, sizeof(args));
	args[0] = 5;
	if (!sendCommand(fn[1], args)) {
		return;
	}

	// 2nd base.
	parts[1] = args[0];
	// == Part 2 End ==

	dramBaseAddr_ = parts[0] & 0xFFFFFFFF;
}

bool wm_sensors::hardware::cpu::RyzenSMU::transferTableToDRAM()
{
	u32 args[SMU_REQ_MAX_ARGS] = {0};
	u32 fn;

	switch (cpuCodeName_) {
		case CpuCodeName::Matisse:
		case CpuCodeName::Vermeer: fn = 0x05; break;
		case CpuCodeName::Renoir:
			args[0] = 3;
			fn = 0x65;
			break;
		case CpuCodeName::Picasso:
		case CpuCodeName::RavenRidge:
		case CpuCodeName::RavenRidge2:
			args[0] = 3;
			fn = 0x3d;
			break;
		default: return false;
	}

	return sendCommand(fn, args);
}

std::vector<float> wm_sensors::hardware::cpu::RyzenSMU::readDRAMToArray()
{
	std::vector<float> table(pmTableSize_ / sizeof(float));

#if SIZE_OF_VOID_P == 8
	const void* srcAddr = reinterpret_cast<const void*>(static_cast<u64>(dramBaseAddr_));
#else
	const void* srcAddr = reinterpret_cast<const void*>(dramBaseAddr_);
#endif

	if (hardware::impl::Ring0::instance().readMemory(srcAddr, table.data(), pmTableSize_)) {
		return table;
	}
	return {};
}

bool wm_sensors::hardware::cpu::RyzenSMU::isPmTableLayoutDefined() const
{
	return supportedPmTableVersions.contains(pmTableVersion_);
}

bool wm_sensors::hardware::cpu::RyzenSMU::sendCommand(unsigned msg, std::span<u32, SMU_REQ_MAX_ARGS> args)
{
	unsigned tmp = 0;
	if (waitForMutex(mutex_, 5000)) {
		// Step 1: Wait until the RSP register is non-zero.

		tmp = 0;
		unsigned retries = SMU_RETRIES_MAX;
		do {
			if (!readPCIReg(addr_.rsp, tmp)) {
				mutex_.unlock();
				return false;
			}
		} while (tmp == 0 && 0 != retries--);

		// Step 1.b: A command is still being processed meaning a new command cannot be issued.

		if (retries == 0 && tmp == 0) {
			mutex_.unlock();
			return false;
		}

		impl::GlobalMutexLock pciLock{impl::GlobalMutex::PCIBus, std::chrono::milliseconds(100)};

		// Step 2: Write zero (0) to the RSP register
		writePCIReg(addr_.rsp, 0, false);

		// Step 3: Write the argument(s) into the argument register(s)
		for (std::size_t i = 0; i < args.size(); ++i) {
			writePCIReg(addr_.args + static_cast<u32>(i * sizeof(u32)), args[i], false);
		}

		// Step 4: Write the message Id into the Message ID register
		writePCIReg(addr_.cmd, msg, false);

		// Step 5: Wait until the Response register is non-zero.
		tmp = 0;
		retries = SMU_RETRIES_MAX;
		do {
			if (!readPCIReg(addr_.rsp, tmp, false)) {
				mutex_.unlock();
				return false;
			}
		} while (tmp == 0 && retries-- != 0);

		if (retries == 0 && tmp != Status::OK) {
			mutex_.unlock();
			return false;
		}

		// Step 6: If the Response register contains OK, then SMU has finished processing the message.

		for (unsigned i = 0; i < SMU_REQ_MAX_ARGS; i++) {
			if (!readPCIReg(addr_.args + (i * sizeof(u32)), args[i], false)) {
				mutex_.unlock();
				return false;
			}
		}

		readPCIReg(addr_.rsp, tmp, false);
		mutex_.unlock();
	}

	return tmp == Status::OK;
}
