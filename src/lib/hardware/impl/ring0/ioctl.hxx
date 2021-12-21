#ifndef WM_SENSORS_LIB_HARDWARE_IMPL_RING0_IOCTL_HXX
#define WM_SENSORS_LIB_HARDWARE_IMPL_RING0_IOCTL_HXX

#include "../../../utility/utility.hxx"

namespace wm_sensors::hardware::impl::ioctl {
	enum class Method
	{
		Buffered = 0,
		InDirect = 1,
		OutDirect = 2,
		Neither = 3
	};

	enum class Access
	{
		Any = 0,
		Read = 1,
		Write = 2
	};

	constexpr u32 controlCode(unsigned deviceType, unsigned function, Method method, Access access)
	{
		return (deviceType << 16) | (wm_sensors::utility::to_underlying(access) << 14) | (function << 2) |
		       (wm_sensors::utility::to_underlying(method));
	}

	constexpr u32 controlCode(unsigned deviceType, unsigned function, Access access = Access::Any)
	{
		return controlCode(deviceType, function, Method::Buffered, access);
	}
}
#endif
