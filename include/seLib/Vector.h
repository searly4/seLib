#pragma once

/// \cond
#include <stdint.h>
#include <array>
#include <cmath>
#include <optional>
/// \endcond

namespace seLib {

//==================================================================================================

template <typename T, size_t Size_N>
class Vector_t : public std::array<T, Size_N> {
public:
	using Self_t = Vector_t;
	using Data_t = T;
	//static constexpr size_t Size { Size_N };
	static constexpr size_t size() { return Size_N; };

	template <typename T2> struct contains_vector_impl : public std::false_type {};
	template <typename T2, size_t Size2> struct contains_vector_impl<Vector_t<T2, Size2>> : public std::true_type {};
	// Test if elements are also vectors, i.e. part of a matrix.
	typedef contains_vector_impl<T> contains_vector;


	// Alias functions to access elements by name, follows XYZW ordering convention.

	inline constexpr T& X() { return (*this)[0]; }
	inline constexpr T const & X() const { return (*this)[0]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 2, T& >::type
	inline constexpr Y() { return (*this)[1]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 2, T const & >::type
	inline constexpr Y() const { return (*this)[1]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 3, T& >::type
	inline constexpr Z() { return (*this)[2]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 3, T const & >::type
	inline constexpr Z() const { return (*this)[2]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 4, T& >::type
	inline constexpr W() { return (*this)[3]; }

	template<size_t SizeTest = Size_N>
	typename std::enable_if< SizeTest >= 4, T const & >::type
	inline constexpr W() const { return (*this)[3]; }


	Self_t& operator=(Self_t const & b) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] = b[i];
		return *this;
	}


	Self_t& operator+=(Self_t const & b) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] += b[i];
		return *this;
	}

	Self_t& operator-=(Self_t const & b) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] -= b[i];
		return *this;
	}

	Self_t& operator*=(Self_t const & b) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] *= b[i];
		return *this;
	}

	Self_t& operator/=(Self_t const & b) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] /= b[i];
		return *this;
	}

	Self_t& operator*=(T mfactor) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] *= mfactor;
		return *this;
	}

	Self_t& operator/=(T mfactor) noexcept {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] /= mfactor;
		return *this;
	}

	constexpr Self_t operator+(Self_t const & b) const noexcept { return Self_t{*this} += b; }

	constexpr Self_t operator-(Self_t const & b) const noexcept { return Self_t{*this} -= b; }

	constexpr Self_t operator*(Self_t const & b) const noexcept { return Self_t{*this} *= b; }

	constexpr Self_t operator/(Self_t const & b) const noexcept { return Self_t{*this} /= b; }

	constexpr Self_t operator*(T mfactor) const noexcept { return Self_t{*this} *= mfactor; }

	constexpr Self_t operator/(T mfactor) const noexcept { return Self_t{*this} /= mfactor; }

	constexpr Self_t operator-() const noexcept { return Self_t{} - *this; }

	constexpr Self_t inverse() const noexcept {
		Self_t ret;
		for (size_t i = 0; i < ret.size(); i++)
			ret[i] = static_cast<T>(1) / (*this)[i];
		return ret;
	}

	constexpr T magnitude2() const noexcept {
		T val = 0;
		for (size_t i = 0; i < Size_N; i++) {
			T d = (*this)[i];
			val += d * d;
		}
		return val;
	}

	constexpr T magnitude() const noexcept {
		return std::sqrt(magnitude2());
	}

	constexpr Vector_t normalize() const noexcept {
		T mag = magnitude();
		if (mag == 0)
			return *this;
		return (*this) * ((T)1 / mag);
	}

	constexpr operator Vector_t<T, Size_N - 1>() const {
		Vector_t<T, Size_N - 1> ret;
		for (size_t i = 0; i < ret.size(); i++)
			ret[i] = (*this)[i];
		return ret;
	}

private:
	template<typename T2, typename... Args_T>
	void IterateFunction(T (*func)(Args_T..., size_t), Args_T... args) {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i] = func(args..., i);
	}

	template<typename T2, typename... Args_T, typename... FArgs_T>
	void IterateFunction(T (*func)(FArgs_T...), Args_T... args) {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i].IterateFunction(func, args...);
	}

	template<typename T2, typename... FArgs_T>
	void IterateFunction(T (*func)(FArgs_T...)) {
		for (size_t i = 0; i < Size_N; i++)
			(*this)[i].IterateFunction(func, i);
	}

public:
	template<typename T2 = Vector_t>
	typename std::enable_if< !T2::contains_vector::value, size_t >::type
	inline static constexpr dimensions() {
		return 1;
	}

	template<typename T2 = Vector_t>
	typename std::enable_if< T2::contains_vector::value, size_t >::type
	inline static constexpr dimensions() {
		return T::dimensions() + 1;
	}
};

//==================================================================================================

} // namespace IDCLib
