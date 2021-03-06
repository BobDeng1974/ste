//  StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <type_traits>

namespace ste {

/*
 *	@brief	Strong-typed wrapper around a numerical type T
 */
template <typename T, typename tag>
class numerical_type {
public:
	using value_type = T;

protected:
	value_type val;

public:
	constexpr numerical_type() = default;
	explicit constexpr numerical_type(value_type v) noexcept : val(v) {}
	template <typename S, typename = std::enable_if_t<std::is_constructible_v<value_type, S>>>
	explicit constexpr numerical_type(S v) noexcept : val(value_type(v)) {}

	numerical_type(numerical_type&&) = default;
	numerical_type(const numerical_type&) = default;
	
	constexpr numerical_type &operator=(numerical_type &&rhs) noexcept {
		val = std::move(rhs.val);
		return *this;
	}

	constexpr numerical_type& operator=(const numerical_type &rhs) noexcept {
		val = rhs.val;
		return *this;
	}
	constexpr numerical_type& operator+=(numerical_type rhs) noexcept {
		val += rhs.val;
		return *this;
	}
	constexpr numerical_type& operator-=(numerical_type rhs) noexcept {
		val -= rhs.val;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator*=(V rhs) noexcept {
		val *= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator/=(V rhs) noexcept {
		val /= rhs;
		return *this;
	}
	constexpr numerical_type& operator++() noexcept { ++val; return *this; }
	constexpr numerical_type& operator--() noexcept { --val; return *this; }
	constexpr numerical_type operator++(int) noexcept { return numerical_type(val++); }
	constexpr numerical_type operator--(int) noexcept { return numerical_type(val--); }

	template <typename V>
	constexpr numerical_type& operator%=(V rhs) noexcept {
		val %= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator&=(V rhs) noexcept {
		val &= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator|=(V rhs) noexcept {
		val |= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator^=(V rhs) noexcept {
		val ^= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator<<=(V rhs) noexcept {
		val <<= rhs;
		return *this;
	}
	template <typename V>
	constexpr numerical_type& operator>>=(V rhs) noexcept {
		val >>= rhs;
		return *this;
	}

	constexpr bool operator==(numerical_type rhs) const noexcept { return val == rhs.val; }
	constexpr bool operator!=(numerical_type rhs) const noexcept { return val != rhs.val; }
	constexpr bool operator<=(numerical_type rhs) const noexcept { return val <= rhs.val; }
	constexpr bool operator>=(numerical_type rhs) const noexcept { return val >= rhs.val; }
	constexpr bool operator<(numerical_type rhs) const noexcept { return val < rhs.val; }
	constexpr bool operator>(numerical_type rhs) const noexcept { return val > rhs.val; }

	constexpr numerical_type operator+() const noexcept { return numerical_type(+val); }
	constexpr numerical_type operator-() const noexcept { return numerical_type(-val); }

	constexpr auto operator!() const noexcept { return !val; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type operator~() const noexcept {
		return numerical_type(~val);
	}

	constexpr friend numerical_type operator+(numerical_type lhs, numerical_type rhs) noexcept { return lhs += rhs; }
	constexpr friend numerical_type operator-(numerical_type lhs, numerical_type rhs) noexcept { return lhs -= rhs; }
	template <typename V>
	constexpr friend numerical_type operator*(numerical_type lhs, V rhs) noexcept { return lhs *= rhs; }
	template <typename V>
	constexpr friend numerical_type operator*(V lhs, numerical_type rhs) noexcept {
		return numerical_type(lhs * rhs.val);
	}
	template <typename V>
	constexpr friend numerical_type operator/(numerical_type lhs, V rhs) noexcept { return lhs /= rhs; }

	template <typename V>
	constexpr friend numerical_type operator%(numerical_type lhs, V rhs) noexcept { return lhs %= rhs; }
	template <typename V>
	constexpr friend numerical_type operator&(numerical_type lhs, V rhs) noexcept { return lhs &= rhs; }
	template <typename V>
	constexpr friend numerical_type operator|(numerical_type lhs, V rhs) noexcept { return lhs |= rhs; }
	template <typename V>
	constexpr friend numerical_type operator^(numerical_type lhs, V rhs) noexcept { return lhs ^= rhs; }
	template <typename V>
	constexpr friend numerical_type operator<<(numerical_type lhs, V rhs) noexcept { return lhs <<= rhs; }
	template <typename V>
	constexpr friend numerical_type operator>>(numerical_type lhs, V rhs) noexcept { return lhs >>= rhs; }

	explicit constexpr operator value_type() const noexcept { return val; }
	template <typename S, typename = std::enable_if_t<std::is_convertible_v<value_type, S>>>
	explicit constexpr operator S() const noexcept { return static_cast<S>(val); }
};

}
