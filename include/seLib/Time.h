#pragma once
#include <stdint.h>
#include <cmath>
#include <assert.h>
#include <type_traits>
#include <optional>
//#include "FixedPoint.h" // 
//#include "Util.h"
#include "./seLib/BitMask.h"

namespace seLib {

template <typename Storage_T, int Radix_N>
struct TimeValue_t {
	static_assert(std::is_integral<Storage_T>::value, "Template type must be an integer.");
	static_assert(std::is_signed<Storage_T>::value, "Template type must be signed.");

	static constexpr int const Radix = Radix_N;
	using Storage_t = Storage_T;
	using Self_t = TimeValue_t;

	class DivideByZeroException_t : public std::exception {};

	template <typename Conv_T>
	static inline constexpr Conv_T ConversionFactor() noexcept {
		return (Radix > 0) ? ((Conv_T)1 / (Conv_T)(1 << Radix)) : (Conv_T)(1 << Radix);
	}

	static inline constexpr int BitsRange(size_t size, int radix) noexcept {
		return (int)size * 8 - radix; // range includes sign bit
	}

	template <typename T2 = Self_t>
	static inline constexpr int BitsRange() noexcept {
		return BitsRange(sizeof(typename T2::Storage_t), T2::Radix);
	}

	static inline constexpr bool PrecisionLoss(int dest_radix, int source_radix) noexcept {
		return dest_radix < source_radix;
	}

	template <typename ConvertTo_T>
	static inline constexpr bool PrecisionLoss() noexcept {
		return PrecisionLoss(ConvertTo_T::Radix, Radix);
	}

	static inline constexpr bool RangeLoss(size_t dest_size, int dest_radix, size_t source_size, int source_radix) noexcept {
		return BitsRange(dest_size, dest_radix) < BitsRange(source_size, source_radix);
	}

	template <typename ConvertTo_T>
	static inline constexpr bool RangeLoss() noexcept {
		return ConvertTo_T::BitsRange() < BitsRange();
	}

	Storage_T Value { 0 };

	//Self_t & operator=(Self_t const &) = default;

	/*template <typename Storage_T2, int Radix2>
	Self_t & operator=(TimeValue_t<Storage_T2, Radix2> const & val) noexcept {
		Value = (Storage_T)((Radix > Radix2) ? (val << (Radix - Radix2)) : (val >> (Radix2 - Radix)));
		return *this;
	}*/

	template <typename Storage_T2, int Radix2>
	explicit constexpr operator TimeValue_t<Storage_T2, Radix2>() const noexcept {
		// TODO: Add rounding logic.
		return TimeValue_t<Storage_T2, Radix2> {
			(Storage_T2)((Radix2 > Radix) ? (Value << (Radix2 - Radix)) : (Value >> (Radix - Radix2)))
		};
	}


	///-------------------------------------------------------------------------------------------------
	/// @fn	template <typename Storage_T2, int Radix2> Self_t & CatchUp(TimeValue_t<Storage_T2, Radix2> const & val) noexcept
	/// @brief	Increment this time value to match the specified time value. Preserve and if necessary increment
	/// 		the high bits of this value that exceed the range of the source value.
	/// @tparam	Storage_T2	Source storage type.
	/// @tparam	Radix2	  	Source radix.
	/// @param	val	Source value.
	/// @returns	A reference to a Self_t.
	template <typename Storage_T2, int Radix2>
	Self_t & CatchUp(TimeValue_t<Storage_T2, Radix2> const & val) noexcept {
		// TODO: Verify we can never go backward in time.

		constexpr int const preserve_bits = BitsRange() - BitsRange<TimeValue_t<Storage_T2, Radix2>>();

		if (preserve_bits > 0) {
			// Preserve extended range value when converting from format with smaller range.

			constexpr size_t const preserve_start_bit = sizeof(Storage_T) * 8 - preserve_bits;
			constexpr auto const mask = seLib::MaskBits<uint32_t>(preserve_start_bit); // lower bits 1
			constexpr auto const preserve_inc = (Storage_t)1 << preserve_start_bit;

			auto preserve = ~mask & Value;
			auto new_val = static_cast<Self_t>(val).Value;
			auto old_val = mask & Value;
			if (new_val < old_val) // val overflowed, assume only 1 overflow
				new_val += preserve_inc; // result should always be positive, negative values should overflow
			Value = preserve + new_val;

		} else if (Radix < Radix2) {
			// Handle loss of precision case
			Value = static_cast<Self_t>(val).Value;

		} else {
			Value = static_cast<Self_t>(val).Value;
		}

		return *this;
	}

	inline explicit constexpr operator float() const noexcept {
		return Value * ConversionFactor<float>();
	}

	inline explicit constexpr operator double() const noexcept {
		return Value * ConversionFactor<double>();
	}

	inline constexpr bool operator==(Self_t const & val) const noexcept {
		return Value == val.Value;
	}
	inline constexpr bool operator!=(Self_t const & val) const noexcept {
		return Value != val.Value;
	}
	inline constexpr bool operator>(Self_t const & val) const noexcept {
		return Value > val.Value;
	}
	inline constexpr bool operator>=(Self_t const & val) const noexcept {
		return Value >= val.Value;
	}
	inline constexpr bool operator<(Self_t const & val) const noexcept {
		return Value < val.Value;
	}
	inline constexpr bool operator<=(Self_t const & val) const noexcept {
		return Value <= val.Value;
	}

	inline Self_t & operator+=(Self_t const & val) noexcept {
		Value += val.Value;
		return *this;
	}
	inline Self_t & operator-=(Self_t const & val) noexcept {
		Value -= val.Value;
		return *this;
	}
	inline Self_t & operator*=(int val) noexcept {
		Value *= val;
		return *this;
	}

	#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711
	inline Self_t & operator/=(int val) noexcept {
		if (val == 0)
			throw DivideByZeroException_t{};
		Value /= val;
		return *this;
	}
	#endif

	inline Self_t & operator>>=(int val) noexcept {
		Value >>= val;
		return *this;
	}
	inline Self_t & operator<<=(int val) noexcept {
		Value <<= val;
		return *this;
	}

	inline constexpr Self_t operator+(Self_t const & val) const noexcept {
		return Self_t { Value + val.Value };
	}
	inline constexpr Self_t operator-(Self_t const & val) const noexcept {
		return Self_t { Value - val.Value };
	}
	inline constexpr Self_t operator*(int val) const noexcept {
		return Self_t { Value * val};
	}

	#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711
	inline constexpr Self_t operator/(int val) const {
		if (val == 0)
			throw DivideByZeroException_t{};
		return Self_t { Value / val};
	}
	#endif

	inline constexpr Self_t operator>>(int val) const noexcept {
		return Self_t { Value >> val};
	}
	inline constexpr Self_t operator<<(int val) const noexcept {
		return Self_t { Value << val };
	}

	constexpr static Self_t FromRawValue(Storage_T value) noexcept {
		Self_t retval;
		retval.Value = value;
		return retval;
	}
};

template <typename Storage_T>
using TimeValue32kHz_t = TimeValue_t<Storage_T, 15>;

template <typename Storage_T>
using TimeValue1Hz_t = TimeValue_t<Storage_T, 0>;


}
