// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_UTILITY_UNALIGNED_HXX
#define WM_SENSORS_LIB_UTILITY_UNALIGNED_HXX

#include "../stdint.hxx"

#include <bit>
#include <concepts>
#include <cstring>
#include <type_traits>

#ifdef _MSC_VER
#	include <stdlib.h>
#endif

namespace wm_sensors::utility {
	namespace bits {
#ifdef _MSC_VER
		static_assert(std::is_same_v<u16, unsigned short>);
		static_assert(sizeof(u32) == sizeof(unsigned long));
		inline u8 bswap(u8 v)
		{
			return v;
		}
		inline u16 bswap(u16 v)
		{
			return _byteswap_ushort(v);
		}
		inline u32 bswap(u32 v)
		{
			return _byteswap_ulong(v);
		}
		inline u64 bswap(u64 v)
		{
			return _byteswap_uint64(v);
		}
#else
		inline u8 bswap(u8 v)
		{
			return v;
		}
		inline u16 bswap(u16 v)
		{
			return __builtin_bswap16(v);
		}
		inline u32 bswap(u32 v)
		{
			return __builtin_bswap32(v);
		}
		inline u64 bswap(u64 v)
		{
			return __builtin_bswap64(v);
		}
#endif
	} // namespace bits

	template <std::integral T>
	inline T get_unaligned(const void* ptr)
	{
		T res;
		memcpy(&res, ptr, sizeof(T));
		return res;
	}

	template <std::integral T>
	inline void put_unaligned(void* ptr, const T v)
	{
		memcpy(ptr, &v, sizeof(T));
	}

	template <class T>
	static T bswap_le(T const v)
	{
		if constexpr (std::endian::native == std::endian::little) {
			return v;
		} else {
			return bits::bswap(v);
		}
	}

	template <class T>
	static T bswap_be(T const v)
	{
		if constexpr (std::endian::native == std::endian::little) {
			return bits::bswap(v);
		} else {
			return v;
		}
	}

	template <class T>
	inline T get_unaligned_le(const void* ptr)
	{
		return static_cast<T>(bswap_le(get_unaligned<std::make_unsigned_t<T>>(ptr)));
	}

	template <class T>
	inline T get_unaligned_be(const void* ptr)
	{
		return static_cast<T>(bswap_be(get_unaligned<std::make_unsigned_t<T>>(ptr)));
	}

	template <class T>
	inline void put_unaligned_le(const void* ptr, const T v)
	{
		put_unaligned(ptr, bswap_le(v));
	}

	template <class T>
	inline void put_unaligned_be(const void* ptr, const T v)
	{
		put_unaligned(ptr, bswap_be(v));
	}

} // namespace wm_sensors::utility

#endif
