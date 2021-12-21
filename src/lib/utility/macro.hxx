// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_UTILITY_MACRO_HXX
#define WM_SENSORS_LIB_UTILITY_MACRO_HXX

#define DELETE_COPY_CTOR_AND_ASSIGNMENT(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete;

#define DEFAULT_MOVE_CTOR_AND_ASSIGNMENT(ClassName) \
	ClassName(ClassName&&) = default;           \
	ClassName& operator=(ClassName&&) = default;

#endif
