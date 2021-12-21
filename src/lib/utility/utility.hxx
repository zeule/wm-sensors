// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_UTILITY_UTILITY_HXX
#define WM_SENSORS_LIB_UTILITY_UTILITY_HXX

#include "./bit.hxx"

#include "../stdint.hxx"

#include <stdexcept>
#include <type_traits>

namespace wm_sensors::utility {
	template <class Enum>
	constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
	{
		return static_cast<std::underlying_type_t<Enum>>(e);
	}

	template <class T>
	constexpr std::make_unsigned_t<T> to_unsigned(T value) noexcept
	{
		return static_cast<std::make_unsigned_t<T>>(value);
	}

	template <class T>
	constexpr std::make_unsigned_t<T> to_unsigned_checked(T value)
	{
		return value >= 0 ? static_cast<std::make_unsigned_t<T>>(value) : throw std::range_error("value is negative");
	}

	template <class T, std::size_t N>
	constexpr std::size_t array_size(T (&)[N]) noexcept
	{
		return N; // std::size?
	}

	namespace detail {
		template <typename C>
		static std::true_type test_deref_op(decltype(&C::operator*));
		template <typename C>
		static std::false_type test_deref_op(...);

		template <typename C>
		static std::true_type test_arrow_op(decltype(&C::operator->));
		template <typename C>
		static std::false_type test_arrow_op(...);
	} // namespace detail

	template <class T>
	struct has_deref_op: public decltype(detail::test_deref_op<T>(nullptr)) {
	};

	template <class T>
	struct has_arrow_op: public decltype(detail::test_arrow_op<T>(nullptr)) {
	};

	template <class T>
	struct is_smart_pointer: public std::bool_constant<has_deref_op<T>::value && has_arrow_op<T>::value> {
	};

	template <class T>
	inline constexpr bool is_smart_pointer_v = is_smart_pointer<T>::value;

	namespace detail {
		template <bool isSmartPointer, class T>
		struct remove_smart_pointer {
		};

		template <class T>
		struct remove_smart_pointer<true, T> {
			using type = T::element_type;
		};

		template <class T>
		struct remove_smart_pointer<false, T> {
			using type = T;
		};
	} // namespace detail

	template <class T>
	struct remove_smart_pointer {
		using type = typename detail::remove_smart_pointer<is_smart_pointer_v<T>, T>::type;
	};

	template <class T>
	using remove_smart_pointer_t = typename remove_smart_pointer<T>::type;

} // namespace wm_sensors::utility

#endif
