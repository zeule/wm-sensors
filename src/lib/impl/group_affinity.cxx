// SPDX-License-Identifier: GPL-3.0+

#include "./group_affinity.hxx"

#include "../utility/string.hxx"

#include <spdlog/spdlog.h>

#include <Windows.h>

#include <limits>

wm_sensors::impl::ThreadGroupAffinityGuard::ThreadGroupAffinityGuard(GroupAffinity a)
    : previous_{GroupAffinity::set(a)}
    , set_{true}
{
}

wm_sensors::impl::ThreadGroupAffinityGuard::~ThreadGroupAffinityGuard()
{
	release();
}

void wm_sensors::impl::ThreadGroupAffinityGuard::release()
{
	if (set_) {
		GroupAffinity::set(previous_);
		set_ = false;
	}
}

wm_sensors::impl::GroupAffinity wm_sensors::impl::GroupAffinity::undefined()
{
	return GroupAffinity(std::numeric_limits<unsigned short>::max(), 0);
}

wm_sensors::impl::GroupAffinity wm_sensors::impl::GroupAffinity::single(unsigned short group, unsigned index)
{
	return GroupAffinity(group, 1ULL << index);
}

wm_sensors::impl::GroupAffinity::GroupAffinity(unsigned short group, unsigned long long mask)
    : mask_{mask}
    , group_{group}
{
}

wm_sensors::impl::GroupAffinity wm_sensors::impl::GroupAffinity::set(GroupAffinity affinity)
{
	HANDLE hCurThread = ::GetCurrentThread();
	GROUP_AFFINITY a{0}, prev;
	a.Group = affinity.group();
	a.Mask = affinity.mask();
	if (!::SetThreadGroupAffinity(hCurThread, &a, &prev)) {
		spdlog::error(windowsLastErrorMessage());
	}
	return GroupAffinity(prev.Group, prev.Mask);
}

unsigned short wm_sensors::impl::processorGroupCount()
{
	return ::GetActiveProcessorGroupCount();
}
