// SPDX-License-Identifier: LGPL-3.0+

#include "./probe.hxx"

#include "../../impl/chip_registrator.hxx"
#include "../../impl/group_affinity.hxx"
#include "../../sensor_tree.hxx"
#include "./cpuid.hxx"
#include "./amd/amd0f_cpu.hxx"
#include "./amd/amd10_cpu.hxx"
#include "./amd/amd17_cpu.hxx"
#include "./intel/intel_cpu.hxx"

#ifdef _M_AMD64
#	define _AMD64_
#endif
#include <cmath>
#include <map>
#include <stdexcept>
#include <sysinfoapi.h>
#include <vector>

namespace {
	using wm_sensors::hardware::cpu::CPUIDData;
	using namespace wm_sensors::stdtypes;

	std::vector<u8> coresPerGroupCounts()
	{
		DWORD length = 0;
		::GetLogicalProcessorInformationEx(RelationGroup, NULL, &length);
		std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> buffer(
		    static_cast<std::size_t>(std::ceil(static_cast<float>(length) / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX))));
		BOOL ret = ::GetLogicalProcessorInformationEx(RelationGroup, buffer.data(), &length);
		if (!ret) {
			throw std::runtime_error("GetLogicalProcessorInformation error");
		}

		std::vector<u8> res;
		for (const auto& pi: buffer) {
			switch (pi.Relationship) {
				case RelationGroup:
					res.reserve(pi.Group.ActiveGroupCount);
					for (u16 i = 0; i < pi.Group.ActiveGroupCount; ++i) {
						res.push_back(static_cast<u8>(__popcnt64(pi.Group.GroupInfo[i].ActiveProcessorMask)));
					}
					return res;
				default: break;
			}
		}
		throw std::logic_error("Could not find processor group info data");
	}

	std::vector<std::vector<CPUIDData>> processorThreads()
	{
		std::vector<CPUIDData> threads;

		const std::vector<u8> coresPerGroup = coresPerGroupCounts();

		for (u16 i = 0; i < coresPerGroup.size(); i++) {
			for (u8 j = 0; j < coresPerGroup[i]; j++) {
				threads.push_back(CPUIDData::get(i, j));
			}
		}

		std::map<u32, std::vector<CPUIDData>> processors;
		for (const CPUIDData& thread: threads) {
			processors[thread.processorId()].push_back(thread);
		}

		std::vector<std::vector<CPUIDData>> processorThreads(processors.size());
		std::size_t index = 0;
		for (auto& p: processors) {
			processorThreads[index] = std::move(p.second);
			++index;
		}

		return processorThreads;
	}

	std::vector<std::vector<CPUIDData>> groupThreadsByCore(std::vector<CPUIDData>&& threads)
	{
		std::map<u32, std::vector<CPUIDData>> cores;
		for (auto& thread: threads) {
			cores[thread.coreId()].push_back(std::move(thread));
		}

		std::vector<std::vector<CPUIDData>> coreThreads;
		for (auto& lp: cores) {
			coreThreads.push_back(std::move(lp.second));
		}

		return coreThreads;
	}
} // namespace

static wm_sensors::impl::PersistentHardwareRegistrator<wm_sensors::hardware::cpu::CPUProbe> registrator;

bool wm_sensors::hardware::cpu::CPUProbe::probe(SensorChipTreeNode& sensorsTree)
{
	using Vendor = CPUIDData::Vendor;
	std::vector<std::vector<CPUIDData>> procThreads = processorThreads();

	//_threads = new CpuId[processorThreads.Length][][];

	unsigned index = 0;
	for (auto& threads: procThreads) {
		if (threads.empty())
			continue;

		const Vendor vendor = threads.front().vendor();
		const auto family = threads.front().family();
		std::vector<std::vector<CPUIDData>> coreThreads = groupThreadsByCore(std::move(threads));
		//_threads[index] = coreThreads;

		SensorChipTreeNode& cpuNode = sensorsTree.child("cpu");

		auto addCpuPayload = [&]<typename T>(unsigned index) {
			cpuNode.addPayload(std::unique_ptr<SensorChip>(new T(index, std::move(coreThreads))));
		};

		switch (vendor) {
			case Vendor::Intel:
				addCpuPayload.template operator()<IntelCPU>(index);
				break;
			case Vendor::AMD:
				switch (family) {
					case 0x0F:
						addCpuPayload.template operator()<Amd0FCpu>(index);
						break;
					case 0x10:
					case 0x11:
					case 0x12:
					case 0x14:
					case 0x15:
					case 0x16:
						addCpuPayload.template operator()<Amd10Cpu>(index);
						break;
					case 0x17:
					case 0x19:
						addCpuPayload.template operator()<Amd17Cpu>(index);
						break;
					default:
						addCpuPayload.template operator()<GenericCPU>(index);
						break;
				}

				break;
			default:
				addCpuPayload.template operator()<GenericCPU>(index);
				break;
		}

		index++;
	}
	return true;
}
