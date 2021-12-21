// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_UTILITY_BIT_HXX
#define WM_SENSORS_LIB_UTILITY_BIT_HXX

#include "../stdint.hxx"

#include <type_traits>

namespace wm_sensors::utility {
	/**
	 * @brief Returns a number of type T with the given bit set
	 */
	template <class R, class T>
	constexpr R bit(T n) noexcept requires std::is_integral_v<R>
	{
		return static_cast<R>(1) << n;
	}

	template <class T, class N>
	constexpr bool is_bit_set(T v, N n)
	{
		return (v & bit<T>(n)) != 0;
	}

	constexpr u8 hibyte(u16 val) noexcept
	{
		return static_cast<u8>(val >> 8);
	}

	constexpr u8 lobyte(u16 val) noexcept
	{
		return static_cast<u8>(val & 0x00ff);
	}

	constexpr u16 word(u8 hi, u8 lo) noexcept
	{
		return static_cast<u16>(hi << 8 | lo);
	}
} // namespace wm_sensors

#endif
