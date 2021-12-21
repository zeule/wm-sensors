// SPDX-License-Identifier: GPL-3.0+

#include "./ring0.hxx"

#include "./ring0/inpout.hxx"
#include "./ring0/winring0.hxx"
#include "../../impl/group_affinity.hxx"

#include <Windows.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace {
	using wm_sensors::hardware::impl::GlobalMutex;
	std::map<GlobalMutex, std::wstring_view> globalMutexNames = {
		{GlobalMutex::EC, L"Global\\Access_EC"},
		{GlobalMutex::ISABus, L"Global\\Access_ISABUS.HTP.Method"},
		{GlobalMutex::PCIBus, L"Global\\Access_PCI"},
		{GlobalMutex::SMBus, L"Global\\Access_SMBUS.HTP.Method"},
	};

	struct CloseHandleDeleter {
		void operator()(HANDLE h)
		{
			if (h && h != INVALID_HANDLE_VALUE) {
				::CloseHandle(h);
			}
		}
	};

	using MutexPtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, CloseHandleDeleter>;

	MutexPtr globalMutex(GlobalMutex m)
	{
		MutexPtr res{::CreateMutex(nullptr, FALSE, globalMutexNames.at(m).data())};
		if (!res) {
			res.reset(::OpenMutex(SYNCHRONIZE, FALSE, globalMutexNames.at(m).data()));
		}
		return res;
	}

	std::string_view mutexName(GlobalMutex m)
	{
		switch (m) {
			case GlobalMutex::EC: return "Global\\Access_EC";
			case GlobalMutex::ISABus: return "Global\\Access_ISABUS.HTP.Method";
			case GlobalMutex::PCIBus: return "Global\\Access_PCI";
			case GlobalMutex::SMBus: return "Global\\Access_SMBUS.HTP.Method";
		}
		return "Unknown";
	}
}

struct wm_sensors::hardware::impl::Ring0::Impl {
	WinRing0 wr0;
	InpOut inpout;
	std::map<GlobalMutex, MutexPtr> globalMutices;

	Impl() = default;
	Impl(const Impl&) = delete;
	Impl& operator=(const Impl&) = delete;
};

wm_sensors::hardware::impl::Ring0::Ring0()
    : impl_{std::make_unique<Impl>()}
{
	impl_->globalMutices.insert({GlobalMutex::EC, globalMutex(GlobalMutex::EC)});
	impl_->globalMutices.insert({GlobalMutex::ISABus, globalMutex(GlobalMutex::ISABus)});
	impl_->globalMutices.insert({GlobalMutex::PCIBus, globalMutex(GlobalMutex::PCIBus)});
	impl_->globalMutices.insert({GlobalMutex::SMBus, globalMutex(GlobalMutex::SMBus)});
}

wm_sensors::hardware::impl::Ring0::~Ring0() = default;

wm_sensors::hardware::impl::Ring0& wm_sensors::hardware::impl::Ring0::instance()
{
	static Ring0 instance;
	return instance;
}

bool wm_sensors::hardware::impl::Ring0::acquireMutex(GlobalMutex mutex, std::chrono::milliseconds wait)
{
	return ::WaitForSingleObject(impl_->globalMutices.at(mutex).get(), static_cast<DWORD>(wait.count())) == WAIT_OBJECT_0;
}

void wm_sensors::hardware::impl::Ring0::releaseMutex(GlobalMutex mutex)
{
	::ReleaseMutex(impl_->globalMutices.at(mutex).get());
}

bool wm_sensors::hardware::impl::Ring0::readMSR(u32 index, u32& eax, u32& edx)
{
	return impl_->wr0.readMSR(index, eax, edx);
}

bool wm_sensors::hardware::impl::Ring0::readMSR(u32 index, MSRValue& value)
{
	return readMSR(index, value.reg.eax, value.reg.edx);
}

bool wm_sensors::hardware::impl::Ring0::readMSR(
    u32 index, u32& eax, u32& edx, const wm_sensors::impl::GroupAffinity& affinity)
{
	wm_sensors::impl::ThreadGroupAffinityGuard g{affinity};
	return readMSR(index, eax, edx);
}

bool wm_sensors::hardware::impl::Ring0::readMSR(u32 index, MSRValue& value, const wm_sensors::impl::GroupAffinity& affinity)
{
	wm_sensors::impl::ThreadGroupAffinityGuard g{affinity};
	return readMSR(index, value);
}

bool wm_sensors::hardware::impl::Ring0::writeMSR(u32 index, u32 eax, u32 edx)
{
	return impl_->wr0.writeMSR(index, eax, edx);
}

bool wm_sensors::hardware::impl::Ring0::writeMSR(u32 index, MSRValue value)
{
	return writeMSR(index, value.reg.eax, value.reg.edx);
}

bool wm_sensors::hardware::impl::Ring0::writeMSR(
    u32 index, u32 eax, u32 edx, const wm_sensors::impl::GroupAffinity& affinity)
{
	wm_sensors::impl::ThreadGroupAffinityGuard g{affinity};
	return writeMSR(index, eax, edx);
}

bool wm_sensors::hardware::impl::Ring0::writeMSR(
    u32 index, MSRValue value, const wm_sensors::impl::GroupAffinity& affinity)
{
	wm_sensors::impl::ThreadGroupAffinityGuard g{affinity};
	return writeMSR(index, value);
}

wm_sensors::u8 wm_sensors::hardware::impl::Ring0::readIOPort(u16 port)
{
	return impl_->wr0.readIOPort(port);
}

void wm_sensors::hardware::impl::Ring0::writeIOPOrt(u16 port, u8 value)
{
	impl_->wr0.writeIOPort(port, value);
}

bool wm_sensors::hardware::impl::Ring0::readPciConfig(u32 pciAddress, u32 regAddress, u32& value)
{
	return impl_->wr0.readPciConfig(pciAddress, regAddress, value);
}

bool wm_sensors::hardware::impl::Ring0::writePciConfig(u32 pciAddress, u32 regAddress, u32 value)
{
	return impl_->wr0.writePciConfig(pciAddress, regAddress, value);
}

bool wm_sensors::hardware::impl::Ring0::readMemory(const void* address, void* buffer, std::size_t size)
{
	HANDLE hPhysMemory;
	void* linPtr = impl_->inpout.mapPhysycalMemory(const_cast<void*>(address), size, hPhysMemory);
	if (linPtr) {
		::memcpy(buffer, linPtr, size);
		impl_->inpout.unmapPhysicalMemory(hPhysMemory, linPtr);
		return true;
	}
	return false;
}

wm_sensors::hardware::impl::GlobalMutexTryLock::GlobalMutexTryLock(GlobalMutex mutex, std::chrono::milliseconds wait)
    : mutex_{mutex}
    , success_{Ring0::instance().acquireMutex(mutex_, wait)}
{
}

wm_sensors::hardware::impl::GlobalMutexTryLock::~GlobalMutexTryLock()
{
	if (success_) {
		Ring0::instance().releaseMutex(mutex_);
	}
}

wm_sensors::hardware::impl::GlobalMutexLock::GlobalMutexLock(GlobalMutex mutex, std::chrono::milliseconds wait)
    : mutex_{mutex}
{
	if (!Ring0::instance().acquireMutex(mutex_, wait)) {
		throw AcquireFailed(std::string(mutexName(mutex_)));
	}
}

wm_sensors::hardware::impl::GlobalMutexLock::~GlobalMutexLock()
{
	Ring0::instance().releaseMutex(mutex_);
}

wm_sensors::hardware::impl::GlobalMutexLock::AcquireFailed::AcquireFailed(const std::string& name)
    : std::runtime_error("Failed to acquire global mutex '" + name + "'")
{
}
