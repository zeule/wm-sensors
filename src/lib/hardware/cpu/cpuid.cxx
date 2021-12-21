// SPDX-License-Identifier: LGPL-3.0+

#include "./cpuid.hxx"

#include <cmath>
#include <stdexcept>

#ifdef _WIN32
#	include <intrin.h>
#else
#	include <cpuid.h>
#endif

namespace {
	void cpuid(unsigned i, unsigned regs[4])
	{
#ifdef _WIN32
		__cpuid(reinterpret_cast<int*>(regs), static_cast<int>(i));
#else
		__get_cpuid(i, regs_ + 0, regs_ + 1, regs_ + 2, regs_ + 3);
#endif
	}

	void appendRegister(std::string& b, unsigned value)
	{
		b += static_cast<char>(value & 0xff);
		b += static_cast<char>((value >> 8) & 0xff);
		b += static_cast<char>((value >> 16) & 0xff);
		b += static_cast<char>((value >> 24) & 0xff);
	}

	unsigned nextLog2(unsigned long x)
	{
		if (x == 0)
			return 0;

		x--;
		unsigned count = 0;
		while (x > 0) {
			x >>= 1;
			count++;
		}

		return count;
	}

	std::string cpuName(const std::vector<wm_sensors::hardware::cpu::CPUIDData::Rec>& extData)
	{
		std::string res;
		for (unsigned i = 2; i <= 4; ++i) {
			for (unsigned reg = 0; reg < 4; ++reg) {
				if (!extData[i].regs[reg]) {
					return res;
				}
				appendRegister(res, extData[i].regs[reg]);
			}
		}
		return res;
	}

	const unsigned CPUID_0 = 0;
	const unsigned CPUID_EXT = 0x80000000;
	// ReSharper restore InconsistentNaming
} // namespace

wm_sensors::hardware::cpu::CPUIDData::CPUIDData(u16 group, u8 thread, impl::GroupAffinity affinity)
    : affinity_{affinity}
    , apicId_{0}
    , coreId_{0}
    , family_{0}
    , model_{0}
    , processorId_{0}
    , stepping_{0}
    , threadId_{0}
    , vendor_{Vendor::Unknown}
    , pkgType_{0}
    , group_{group}
    , thread_{thread}
{
	unsigned threadMaskWith;
	unsigned coreMaskWith;
	unsigned maxCpuidExt;

	if (thread >= 64)
		throw new std::out_of_range("thread out of range (>= 64)");


	unsigned regs[4];
	cpuid(CPUID_0, regs);
	if (regs[0] == 0) {
		return;
	}
	unsigned maxCpuid = regs[0];
	std::string cpuVendor;
	appendRegister(cpuVendor, regs[1]); // EBX
	appendRegister(cpuVendor, regs[3]); // EDX
	appendRegister(cpuVendor, regs[2]); // ECX
	
	
	if (cpuVendor == "GenuineIntel") {
		vendor_ = Vendor::Intel;
	} else if (cpuVendor == "AuthenticAMD") {
		vendor_ = Vendor::AMD;
	} else { // TODO: add VM vendors
		vendor_ = Vendor::Unknown;
	}
	
	cpuid(CPUID_EXT, regs);
	if (regs[0] > CPUID_EXT) {
		maxCpuidExt = regs[0] = CPUID_EXT;
	} else {
		return;
	}

	maxCpuid = std::min(maxCpuid, 1024u);
	maxCpuidExt = std::min(maxCpuidExt, 1024u);

	data_.resize(maxCpuid + 1);
	for (unsigned i = 0; i < maxCpuid + 1; i++) {
		cpuid(CPUID_0 + i, data_[i].regs);
	}

	extData_.resize(maxCpuidExt + 1);
	for (unsigned i = 0; i < maxCpuidExt + 1; i++) {
		cpuid(CPUID_EXT + i, extData_[i].regs);
	}


	if (maxCpuidExt > 4) {
		name_ = cpuName(extData_);	
		brand_ = name_;
	}
#if 0
	BrandString = nameBuilder.ToString().Trim();
	nameBuilder.Replace("(R)", string.Empty);
	nameBuilder.Replace("(TM)", string.Empty);
	nameBuilder.Replace("(tm)", string.Empty);
	nameBuilder.Replace("CPU", string.Empty);
	nameBuilder.Replace("Dual-Core Processor", string.Empty);
	nameBuilder.Replace("Triple-Core Processor", string.Empty);
	nameBuilder.Replace("Quad-Core Processor", string.Empty);
	nameBuilder.Replace("Six-Core Processor", string.Empty);
	nameBuilder.Replace("Eight-Core Processor", string.Empty);
	nameBuilder.Replace("6-Core Processor", string.Empty);
	nameBuilder.Replace("8-Core Processor", string.Empty);
	nameBuilder.Replace("12-Core Processor", string.Empty);
	nameBuilder.Replace("16-Core Processor", string.Empty);
	nameBuilder.Replace("24-Core Processor", string.Empty);
	nameBuilder.Replace("32-Core Processor", string.Empty);
	nameBuilder.Replace("64-Core Processor", string.Empty);

	for (int i = 0; i < 10; i++)
		nameBuilder.Replace("  ", " ");

	Name = nameBuilder.ToString();
	if (Name.Contains("@"))
		Name = Name.Remove(Name.LastIndexOf('@'));

	Name = Name.Trim();
#endif
	family_ = ((data_[1][0] & 0x0FF00000) >> 20) + ((data_[1][0] & 0x0F00) >> 8);
	model_ = ((data_[1][0] & 0x0F0000) >> 12) + ((data_[1][0] & 0xF0) >> 4);
	stepping_ = data_[1][0] & 0x0F;
	apicId_ = (data_[1][1] >> 24) & 0xFF;
	pkgType_ = (extData_[1][1] >> 28) & 0xFF;

	switch (vendor_) {
		case Vendor::Intel: {
			unsigned maxCoreAndThreadIdPerPackage = (data_[1][1] >> 16) & 0xFF;
			unsigned maxCoreIdPerPackage = maxCpuid >= 4 ? ((data_[4][0] >> 26) & 0x3F) + 1 : 1;

			threadMaskWith = nextLog2(maxCoreAndThreadIdPerPackage / maxCoreIdPerPackage);
			coreMaskWith = nextLog2(maxCoreIdPerPackage);
			break;
		}
		case Vendor::AMD: {
			unsigned corePerPackage = maxCpuidExt >= 8 ? (extData_[8][2] & 0xFF) + 1 : 1;
			
			threadMaskWith = 0;
			coreMaskWith = nextLog2(corePerPackage);

			if (family_ == 0x17 || family_ == 0x19) {
				// ApicIdCoreIdSize: APIC ID size.
				// cores per DIE
				// we need this for Ryzen 5 (4 cores, 8 threads) ans Ryzen 6 (6 cores, 12 threads)
				// Ryzen 5: [core0][core1][dummy][dummy][core2][core3] (Core0 EBX = 00080800, Core2 EBX =
				// 08080800)
				unsigned maxCoresPerDie = (extData_[8][2] >> 12) & 0xF;
				switch (maxCoresPerDie) {
					case 0x04: // Ryzen
					{
						coreMaskWith = nextLog2(16);
						break;
					}
					case 0x05: // Threadripper
					{
						coreMaskWith = nextLog2(32);
						break;
					}
					case 0x06: // Epic
					{
						coreMaskWith = nextLog2(64);
						break;
					}
				}
			}

			break;
		}
		default: {
			threadMaskWith = 0;
			coreMaskWith = 0;
			break;
		}
	}

	processorId_ = apicId_ >> (int)(coreMaskWith + threadMaskWith);
	coreId_ = (apicId_ >> (int)threadMaskWith) - (processorId_ << (int)coreMaskWith);
	threadId_ = apicId_ - (processorId_ << (int)(coreMaskWith + threadMaskWith)) - (coreId_ << (int)threadMaskWith);
}

wm_sensors::hardware::cpu::CPUIDData wm_sensors::hardware::cpu::CPUIDData::get(u16 group, u8 thread)
{
	if (thread >= 64)
		throw std::out_of_range("thread must be in range [0:63]");	

	const auto affinity = wm_sensors::impl::GroupAffinity::single(group, thread);
	wm_sensors::impl::ThreadGroupAffinityGuard tga{affinity};
	return CPUIDData(group, thread, affinity);
}
