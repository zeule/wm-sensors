// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_UTILITY_STRING_HXX
#define WM_SENSORS_UTILITY_STRING_HXX

#include "wm-sensors_config.h"
#include "wm-sensors_export.h"

#include <string>
#include <string_view>

#ifndef HAVE_STRNDUP
char* strndup(const char* str, size_t size);
#else
#	include <string.h>
#endif

inline char* strndup(std::string_view s)
{
	return strndup(s.data(), s.size());
}

inline char* strndup(const std::string& s)
{
	return strndup(s.c_str(), s.size());
}

namespace wm_sensors {
	WM_SENSORS_EXPORT std::string windowsErrorMessage(unsigned long errorCode);
	WM_SENSORS_EXPORT std::string windowsLastErrorMessage();
}
#endif
