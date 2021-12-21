// SPDX-License-Identifier: LGPL-3.0+

#include "./generic_memory.hxx"

#include "../../impl/chip_registrator.hxx"
#include "../.../../../sensor_tree.hxx"

namespace wm_sensors::hardware::memory {
	class MemoryProbe: public wm_sensors::impl::ChipProbe {
		// Inherited via ChipProbe
		virtual bool probe(SensorChipTreeNode& sensorsTree) override;
	};

	bool MemoryProbe::probe(SensorChipTreeNode& sensorsTree)
	{
		SensorChipTreeNode& memNode = sensorsTree.child("memory");
		memNode.addPayload(std::unique_ptr<SensorChip>(new GenericMemory()));
		return true;
	}

	static wm_sensors::impl::PersistentHardwareRegistrator<MemoryProbe> registrator;
} // namespace wm_sensors::hardware::memory
