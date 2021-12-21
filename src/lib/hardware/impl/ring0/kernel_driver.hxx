// SPDX-License-Identifier: GPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_WINRING0_KERNEL_DRIVER_HXX
#define WM_SENSORS_LIB_IMPL_WINRING0_KERNEL_DRIVER_HXX

#include <Windows.h>

#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

namespace wm_sensors::hardware::impl {
	class KernelDriver {
	public:
		KernelDriver(const std::filesystem::path& driverFileName, std::wstring serviceName, std::wstring driverId);
		~KernelDriver();

		bool isOpen() const
		{
			return static_cast<bool>(device_);
		}

		bool deviceIOControl(
		    DWORD code, const void* inBuffer, std::size_t size, void* outBuffer = nullptr,
		    std::size_t outBufferSize = 0);

		template <class T>
		bool deviceIOControl(DWORD code, const T& data)
		{
			return deviceIOControl(code, &data, sizeof(T));
		}

		template <class In, class Out>
		bool deviceIOControl(DWORD code, const In& data, Out& res)
		{
			return deviceIOControl(code, &data, sizeof(In), &res, sizeof(Out));
		}

		void deleteOnClose(bool v);

	private:
		bool open();
		std::string installDriver(const std::filesystem::path& path);
		bool deleteDriver();

		static void closeHandle(HANDLE h);

		KernelDriver(const KernelDriver&) = delete;
		KernelDriver& operator=(KernelDriver&) = delete;

		std::wstring serviceName_;
		std::wstring driverId_;
		std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&closeHandle)> device_;
		bool deleteOnClose_;
	};

	std::wstring serviceName(const std::wstring prefix);
	std::filesystem::path driverFileName(const char* driverFileName);

} // namespace wm_sensors::hardware::impl

#endif
