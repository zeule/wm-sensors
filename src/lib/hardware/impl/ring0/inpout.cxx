// SPDX-License-Identifier: GPL-3.0+

#include "./inpout.hxx"

#include "./ioctl.hxx"

#include <filesystem>

#include "ring0-drv-config.h"

namespace {
	namespace ioctl = wm_sensors::hardware::impl::ioctl;

	const DWORD INPOUT_TYPE = 40000;
	constexpr const auto IOCTL_READ_PORT_UCHAR = ioctl::controlCode(INPOUT_TYPE, 0x801);
	constexpr const auto IOCTL_WRITE_PORT_UCHAR = ioctl::controlCode(INPOUT_TYPE, 0x802);
	constexpr const auto IOCTL_READ_PORT_USHORT = ioctl::controlCode(INPOUT_TYPE, 0x803);
	constexpr const auto IOCTL_WRITE_PORT_USHORT = ioctl::controlCode(INPOUT_TYPE, 0x804);
	constexpr const auto IOCTL_READ_PORT_ULONG = ioctl::controlCode(INPOUT_TYPE, 0x805);
	constexpr const auto IOCTL_WRITE_PORT_ULONG = ioctl::controlCode(INPOUT_TYPE, 0x806);
	constexpr const auto IOCTL_WINIO_MAPPHYSTOLIN = ioctl::controlCode(INPOUT_TYPE, 0x807);
	constexpr const auto IOCTL_WINIO_UNMAPPHYSADDR = ioctl::controlCode(INPOUT_TYPE, 0x808);

#if _M_X64
#	define DRIVER_FILE_NAME L"inpoutx64"
#else
#	define DRIVER_FILE_NAME L"inpout32"
#endif

#pragma pack(push)
#pragma pack(1)

	struct tagPhys32Struct {
		HANDLE PhysicalMemoryHandle;
		SIZE_T dwPhysMemSizeInBytes;
		PVOID pvPhysAddress;
		PVOID pvPhysMemLin;
	};

#pragma pack(pop)
} // namespace

wm_sensors::hardware::impl::InpOut::InpOut()
    : driver_{driverFileName(INPOUT_DRIVER_FILE), serviceName(L"INPOUT"), DRIVER_FILE_NAME}
{
}

wm_sensors::hardware::impl::InpOut::~InpOut() {}

void* wm_sensors::hardware::impl::InpOut::mapPhysycalMemory(
    void* pbPhysAddr, std::size_t dwPhysSize, HANDLE& physicalMemoryHandle)
{
	PBYTE pbLinAddr;
	tagPhys32Struct Phys32Struct;

	Phys32Struct.dwPhysMemSizeInBytes = dwPhysSize;
	Phys32Struct.pvPhysAddress = pbPhysAddr;

	if (!driver_.deviceIOControl(
	        IOCTL_WINIO_MAPPHYSTOLIN, &Phys32Struct, sizeof(tagPhys32Struct), &Phys32Struct, sizeof(tagPhys32Struct)))
		return nullptr;
	else {
#ifdef _M_X64
		pbLinAddr =
		    (PBYTE)((LONGLONG)Phys32Struct.pvPhysMemLin + (LONGLONG)pbPhysAddr - (LONGLONG)Phys32Struct.pvPhysAddress);
#else
		pbLinAddr = (PBYTE)((DWORD)Phys32Struct.pvPhysMemLin + (DWORD)pbPhysAddr - (DWORD)Phys32Struct.pvPhysAddress);
#endif
		physicalMemoryHandle = Phys32Struct.PhysicalMemoryHandle;
	}
	return pbLinAddr;
}

bool wm_sensors::hardware::impl::InpOut::unmapPhysicalMemory(HANDLE PhysicalMemoryHandle, void* pbLinAddr)
{
	tagPhys32Struct Phys32Struct;

	Phys32Struct.PhysicalMemoryHandle = PhysicalMemoryHandle;
	Phys32Struct.pvPhysMemLin = pbLinAddr;

	if (!driver_.deviceIOControl(IOCTL_WINIO_UNMAPPHYSADDR, &Phys32Struct, sizeof(tagPhys32Struct), NULL, 0))
		return false;

	return true;
}
