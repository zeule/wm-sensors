// SPDX-License-Identifier: LGPL-3.0+

#include "./chip_registrator.hxx"
#include "../sensor_tree.hxx"

#include <algorithm>

wm_sensors::impl::ChipProbe::~ChipProbe() = default;

wm_sensors::impl::PersistentHardwareRegistratorImpl::PersistentHardwareRegistratorImpl(std::unique_ptr<ChipProbe> probe)
    : probe_{probe.get()}
{
	ChipProbesRegistry::instance().add(std::move(probe));
}

wm_sensors::impl::PersistentHardwareRegistratorImpl::~PersistentHardwareRegistratorImpl()
{
	ChipProbesRegistry::instance().remove(probe_);
}

wm_sensors::impl::ChipProbesRegistry::ChipProbesRegistry()
{
}

wm_sensors::impl::ChipProbesRegistry& wm_sensors::impl::ChipProbesRegistry::instance()
{
	static ChipProbesRegistry instance;
	return instance;
}

wm_sensors::impl::ChipProbesRegistry::~ChipProbesRegistry() = default;

void wm_sensors::impl::ChipProbesRegistry::add(std::unique_ptr<ChipProbe> probe)
{
	probes_.push_back(std::move(probe));
}

void wm_sensors::impl::ChipProbesRegistry::remove(ChipProbe* probe)
{
	std::erase_if(probes_, [probe](const auto& p) {
		return p.get() == probe; });
}

void wm_sensors::impl::ChipProbesRegistry::probeAll(SensorChipTreeNode& tree)
{
	for (const auto& probe : probes_) {
		probe->probe(tree);
	}
}
