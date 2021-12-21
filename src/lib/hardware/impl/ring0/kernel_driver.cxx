// SPDX-License-Identifier: GPL-3.0+

#include "./kernel_driver.hxx"

#include "../../../utility/string.hxx"
#include "../../../wm_sensor_types.hxx"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <Windows.h>

#include <AclAPI.h>
#include <sddl.h>

#include <filesystem>
#include <thread>

#pragma warning (disable: 5039) // pointer or reference to potentially throwing function passed to 'extern "C"' function...

namespace {
	struct ServiceHandleDeleter {
		void operator()(SC_HANDLE h)
		{
			::CloseServiceHandle(h);
		}
	};

	using ServiceHandlePtr = std::unique_ptr<std::remove_pointer_t<SC_HANDLE>, ServiceHandleDeleter>;
} // namespace

wm_sensors::hardware::impl::KernelDriver::KernelDriver(
    const std::filesystem::path& driverFileName, std::wstring serviceName, std::wstring driverId)
    : serviceName_{serviceName}
    , driverId_{driverId}
    , device_{nullptr, &closeHandle}
    , deleteOnClose_{true}
{
	if (open()) {
		return;
	}

	const auto installError = installDriver(driverFileName);

	if (installError.empty()) {
		if (!open()) {
			spdlog::error("Opening driver failed after install");
		}
	} else {
		// install failed, try to delete and reinstall
		deleteDriver();

		// wait a short moment to give the OS a chance to remove the driver
		std::this_thread::sleep_for(std::chrono::seconds(2));
		const auto reinstallError = installDriver(driverFileName);

		if (reinstallError.empty()) {
			if (!open()) {
				spdlog::error("Opening driver failed after reinstall");
			}
		} else {
			bool fileExists = std::filesystem::exists(driverFileName);
			spdlog::error("Installing driver \"{0}\" failed.", reinstallError);
			if (fileExists) {
				spdlog::error("And file exists");
			}
			spdlog::error("First error: {0}", installError);
		}
	}

	if (!isOpen()) {
		deleteDriver();
		throw std::runtime_error("WinRing0 driver installation failed");
	}
}

wm_sensors::hardware::impl::KernelDriver::~KernelDriver()
{
	if (deleteOnClose_) {
		device_.reset();
		deleteDriver();
	}
}

bool wm_sensors::hardware::impl::KernelDriver::deviceIOControl(
    DWORD code, const void* inBuffer, std::size_t size, void* outBuffer, std::size_t outBufferSize)
{
	DWORD numReceived;
	BOOL res = ::DeviceIoControl(
	    device_.get(), code, const_cast<void*>(inBuffer), static_cast<DWORD>(size), outBuffer,
	    static_cast<DWORD>(outBufferSize), &numReceived, nullptr);
	if (!res) {
		spdlog::error("DeviceIoControl() returned error: {0}", windowsLastErrorMessage());
		return false;
	}
	assert(numReceived <= outBufferSize);
	if (numReceived > outBufferSize) {
		spdlog::error(
		    "DeviceIoControl() requires larger buffer ({0}, while {1} was provided", numReceived, outBufferSize);
	}
	return res;
}

void wm_sensors::hardware::impl::KernelDriver::deleteOnClose(bool v)
{
	deleteOnClose_ = v;
}

bool wm_sensors::hardware::impl::KernelDriver::open()
{
	device_.reset(::CreateFile(
	    (L"\\\\.\\" + driverId_).c_str(), 0xC0000000, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if (device_.get() == INVALID_HANDLE_VALUE) {
		device_.reset();
	}
	return !!device_;
}

std::string wm_sensors::hardware::impl::KernelDriver::installDriver(const std::filesystem::path& path)
{
	ServiceHandlePtr manager{::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE)};
	if (!manager) {
		return fmt::format("OpenSCManager error: {0}", windowsLastErrorMessage());
	}

	ServiceHandlePtr service{::CreateService(
	    manager.get(), serviceName_.c_str(), serviceName_.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
	    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, path.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr)};

	if (!service) {
		auto error = ::GetLastError();
		if (error == ERROR_SERVICE_EXISTS) {
			return "Service already exists";
		}

		return fmt::format("CreateService returned the error: {0}", windowsErrorMessage(error));
	}

	if (!::StartService(service.get(), 0, nullptr)) {
		auto error = GetLastError();
		if (error != ERROR_SERVICE_ALREADY_RUNNING) {
			return fmt::format("StartService returned the error: {0}", windowsErrorMessage(error));
		}
	}

#if 0
	// restrict the driver access to system (SY) and builtin admins (BA)
	// TODO: replace with a call to IoCreateDeviceSecure in the driver
	std::wstring filePath = L"\\\\.\\" + driverId_;
	PSECURITY_DESCRIPTOR sd;
	::ConvertStringSecurityDescriptorToSecurityDescriptor(
	            L"O:BAG:SYD:(A;;FA;;;SY)(A;;FA;;;BA)", SDDL_REVISION_1, &sd, nullptr);
	::SetNamedSecurityInfo(filePath.data(), SE_FILE_OBJECT, sd);
	::LocalFree(sd);
#endif

	return {};
}

bool wm_sensors::hardware::impl::KernelDriver::deleteDriver()
{
	ServiceHandlePtr manager{::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS)};
	if (!manager)
		return false;


	ServiceHandlePtr service{::OpenService(manager.get(), serviceName_.c_str(), SERVICE_ALL_ACCESS)};
	if (!service) {
		return true;
	}

	SERVICE_STATUS status;
	::ControlService(service.get(), SERVICE_CONTROL_STOP, &status);
	::DeleteService(service.get());

	return true;
}

void wm_sensors::hardware::impl::KernelDriver::closeHandle(HANDLE h)
{
	if (h && h != INVALID_HANDLE_VALUE) {
		::CloseHandle(h);
	}
}

std::wstring wm_sensors::hardware::impl::serviceName(const std::wstring prefix)
{
	TCHAR executablePath[MAX_PATH];
	::GetModuleFileName(nullptr, executablePath, MAX_PATH);
	std::wstring name = prefix + std::filesystem::path(executablePath).filename().replace_extension().native();
	name.erase(std::remove_if(name.begin(), name.end(), [](wchar_t x) { return std::isspace(x); }), name.end());
	std::replace(name.begin(), name.end(), L'.', L'_');
	return name;
}

std::filesystem::path wm_sensors::hardware::impl::driverFileName(const char* driverFileName)
{
	HMODULE thisDllModule;
	BOOL r = ::GetModuleHandleEx(
	    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
	    reinterpret_cast<LPCTSTR>(&windowsErrorMessage), &thisDllModule);
	if (!r) {
		spdlog::error(
		    "Could not get current module handle, GetModuleHandleEx() returned error: {0}", windowsLastErrorMessage());
	}
	TCHAR executablePath[MAX_PATH];
	::GetModuleFileName(thisDllModule, executablePath, MAX_PATH);
	return std::filesystem::path(executablePath).parent_path() / driverFileName;
}
