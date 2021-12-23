#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <type_traits>

namespace IDCLib {

//template <template<class> class Derived_T, typename T>
template <typename T, typename Derived_T>
class MathTypeWrapper_t {
public:
	typedef T Value_t;
	typedef T const ValueConst_t;
	typedef T & ValueRef_t;
	typedef T const & ValueConstRef_t;
	typedef T const & ValueArg_t;
	typedef Derived_T Derived_t;
	typedef Derived_T & DerivedRef_t;
	typedef Derived_T const & DerivedConstRef_t;
	typedef Derived_T const & DerivedArg_t;

	Value_t Value;

	explicit constexpr MathTypeWrapper_t(ValueArg_t value) : Value(value) {}
	//explicit constexpr MathTypeWrapper_t(ValueArg_t) = default;
	//constexpr MathTypeWrapper_t(DerivedArg_t) = default;
	//DerivedRef_t operator=(DerivedArg_t) = default;

	inline explicit constexpr operator Value_t() const { return Value; }
	
	inline constexpr auto operator+(DerivedArg_t b) const { return Derived_t { Value + b.Value }; }
	inline constexpr auto operator-(DerivedArg_t b) const { return Derived_t { Value - b.Value }; }
	inline constexpr auto operator*(DerivedArg_t b) const { return Derived_t { Value * b.Value }; }
	inline constexpr auto operator/(DerivedArg_t b) const { return Derived_t { Value / b.Value }; }
	inline constexpr auto operator%(DerivedArg_t b) const { return Derived_t { Value % b.Value }; }
	inline constexpr auto operator+() const { return Derived_t { + Value }; }
	inline constexpr auto operator-() const { return Derived_t { -Value }; }

	inline constexpr auto operator==(DerivedArg_t b) const { return Value == b.Value; }
	inline constexpr auto operator!=(DerivedArg_t b) const { return Value != b.Value; }
	inline constexpr auto operator<(DerivedArg_t b) const { return Value < b.Value; }
	inline constexpr auto operator>(DerivedArg_t b) const { return Value > b.Value; }
	inline constexpr auto operator<=(DerivedArg_t b) const { return Value <= b.Value; }
	inline constexpr auto operator>=(DerivedArg_t b) const { return Value >= b.Value; }

	inline auto operator+=(DerivedArg_t b) { Value += b.Value; return *this; }
	inline auto operator-=(DerivedArg_t b) { Value -= b.Value; return *this; }
	inline auto operator*=(DerivedArg_t b) { Value *= b.Value; return *this; }
	inline auto operator/=(DerivedArg_t b) { Value /= b.Value; return *this; }
	inline auto operator%=(DerivedArg_t b) { Value %= b.Value; return *this; }

	inline constexpr auto operator~() const { return Derived_t { ~Value }; }
	inline constexpr auto operator&(DerivedArg_t b) const { return Derived_t { Value & b.Value }; }
	inline constexpr auto operator|(DerivedArg_t b) const { return Derived_t { Value | b.Value }; }
	inline constexpr auto operator^(DerivedArg_t b) const { return Derived_t { Value ^ b.Value }; }
	inline constexpr auto operator<<(size_t count) const { return Derived_t { Value << count }; }
	inline constexpr auto operator>>(size_t count) const { return Derived_t { Value >> count }; }

	inline auto operator&=(DerivedArg_t b) { Value &= b.Value; return *this; }
	inline auto operator|=(DerivedArg_t b) { Value |= b.Value; return *this; }
	inline auto operator^=(DerivedArg_t b) { Value ^= b.Value; return *this; }
	inline auto operator<<=(size_t count) { Value <<= count; return *this; }
	inline auto operator>>=(size_t count) { Value >>= count; return *this; }
};

}
