#ifndef WM_SENSORS_LIB_STDINT_HXX
#	define WM_SENSORS_LIB_STDINT_HXX

#include <cstdint>
#include <type_traits>
#ifndef NDEBUG
#	include <limits>
#	include <stdexcept>
#endif

namespace wm_sensors {
	namespace stdtypes {
		using s8 = std::int8_t;
		using s16 = std::int16_t;
		using s32 = std::int32_t;
		using s64 = std::int64_t;

		using u8 = std::uint8_t;
		using u16 = std::uint16_t;
		using u32 = std::uint32_t;
		using u64 = std::uint64_t;

		namespace detail {
			template <typename T>
			constexpr T cast_with_debug_check(unsigned long long int v) requires std::is_unsigned_v<T>
			{
#	ifndef NDEBUG
				return v > std::numeric_limits<T>::max() ? throw std::logic_error("Literal value is ou of range") :
                                                           static_cast<T>(v);
#	else
				return static_cast<T>(v);
#	endif
			}
		} // namespace detail

		constexpr u8 operator""_u8(unsigned long long int v)
		{
			return detail::cast_with_debug_check<u8>(v);
		}

		constexpr u16 operator""_u16(unsigned long long int v)
		{
			return detail::cast_with_debug_check<u16>(v);
		}

		constexpr u32 operator""_u32(unsigned long long int v)
		{
			return detail::cast_with_debug_check<u32>(v);
		}
	} // namespace stdtypes
	using namespace stdtypes;
} // namespace wm_sensors

#endif
