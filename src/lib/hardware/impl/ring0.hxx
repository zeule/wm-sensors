// SPDX-License-Identifier: GPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_IMPL_RING0_HXX
#define WM_SENSORS_LIB_HARDWARE_IMPL_RING0_HXX

#include "../../wm_sensor_types.hxx"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

namespace wm_sensors::impl {
	class GroupAffinity;
}

namespace wm_sensors::hardware::impl {

	enum class GlobalMutex
	{
		EC,
		ISABus,
		PCIBus,
		SMBus
	};

	class Ring0 {
	public:
		static Ring0& instance();

		bool acquireMutex(GlobalMutex mutex, std::chrono::milliseconds wait);
		void releaseMutex(GlobalMutex mutex);

		union MSRValue {
			u64 value;
			struct {
				u32 eax;
				u32 edx;
			} reg;
		};

		bool readMSR(u32 index, u32& eax, u32& edx);
		bool readMSR(u32 index, MSRValue& value);
		bool readMSR(u32 index, u32& eax, u32& edx, const wm_sensors::impl::GroupAffinity& affinity);
		bool readMSR(u32 index, MSRValue& value, const wm_sensors::impl::GroupAffinity& affinity);
		bool writeMSR(u32 index, u32 eax, u32 edx);
		bool writeMSR(u32 index, MSRValue value);
		bool writeMSR(u32 index, u32 eax, u32 edx, const wm_sensors::impl::GroupAffinity& affinity);
		bool writeMSR(u32 index, MSRValue value, const wm_sensors::impl::GroupAffinity& affinity);

		u8 readIOPort(u16 port);
		void writeIOPOrt(u16 port, u8 value);

		bool readPciConfig(u32 pciAddress, u32 regAddress, u32& value);
		bool writePciConfig(u32 pciAddress, u32 regAddress, u32 value);

		bool readMemory(const void* address, void* buffer, std::size_t size);

		static u32 PCIAddress(u8 bus, u8 device, u8 function)
		{
			return static_cast<u32>((bus << 8) | ((device & 0x1F) << 3) | (function & 7));
		}

		static constexpr const u32 INVALID_PCI_ADDRESS = 0xffffffff;

	private:
		struct Impl;
		Ring0();
		~Ring0();

		Ring0(const Ring0&) = delete;
		Ring0& operator=(const Ring0&) = delete;

		std::unique_ptr<Impl> impl_;
	};

	class GlobalMutexTryLock {
	public:
		GlobalMutexTryLock(GlobalMutex mutex, std::chrono::milliseconds wait);
		~GlobalMutexTryLock();

		bool succeded() const {
			return success_;
		}

		bool failed() const
		{
			return !succeded();
		}

	private:
		GlobalMutex mutex_;
		bool success_;
	};

	class GlobalMutexLock {
	public:
		GlobalMutexLock(GlobalMutex mutex, std::chrono::milliseconds wait);
		~GlobalMutexLock();

		// exceptions
		class AcquireFailed: public std::runtime_error {
		public:
			AcquireFailed(const std::string& name);
		};

	private:
		GlobalMutex mutex_;
	};
} // namespace wm_sensors::hardware::impl

#endif
