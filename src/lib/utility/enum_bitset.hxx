// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_UTILITY_ENUM_BITSET_HXX
#define WM_SENSORS_LIB_UTILITY_ENUM_BITSET_HXX

#include <bitset>
#include <type_traits>

namespace wm_sensors::utility {
	template <class E>
	class enum_bitset {
	public:
		enum_bitset() {}
		enum_bitset(E bit)
		{
			set(bit);
		}
		enum_bitset(std::initializer_list<E> values)
		{
			for (E b: values) {
				set(b);
			}
		}

		void set(E bit, bool value = true)
		{
			bits_.set(static_cast<std::size_t>(bit), value);
		}

		bool test(E bit) const
		{
			return bits_.test(static_cast<std::size_t>(bit));
		}

		bool all() const noexcept
		{
			return bits_.all();
		}
		bool any() const noexcept
		{
			return bits_.any();
		}
		bool none() const noexcept
		{
			return bits_.none();
		}

	private:
		std::bitset<sizeof(std::underlying_type_t<E>) * 8> bits_;
	};
} // namespace wm_sensors::utility

#endif
