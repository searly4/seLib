#pragma once
/*
   Copyright 2018 by Scott Early

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <exception>

namespace seLib {

class FixedPoint_OverflowException : std::exception { };

// Return true if the specified fixed point value will fit in the output storage type.
template <typename In_T, int In_Mag, typename Out_T, int Out_Mag>
static constexpr bool FixedPoint_CheckRange(In_T val) {
	int out_storagebits = sizeof(Out_T) * 8;
	Out_T out_signmask = (Out_T)1 << (out_storagebits - 1);
	int out_radix = (int)out_storagebits - Out_Mag - 1;

	Out_T min = out_signmask >> out_radix;
	Out_T max = ~min;
	if ((val - 1) >= max || val < min)
		return false;
	else
		return true;
}

// Convert a fixed point value from one size/radix type to another.
template <typename In_T, int In_Mag, typename Out_T, int Out_Mag>
static constexpr Out_T FixedPoint_ConvertFixed(In_T val, bool check = true) {
	int in_storagebits = sizeof(In_T) * 8;
	int in_radix = (int)in_storagebits - In_Mag - 1;
	int out_storagebits = sizeof(Out_T) * 8;
	int out_radix = (int)out_storagebits - Out_Mag - 1;

	Out_T out_val = 0;

	if (check && In_Mag > Out_Mag && !FixedPoint_CheckRange<In_T, In_Mag, Out_T, Out_Mag>(val >> in_radix))
		throw FixedPoint_OverflowException();

	if (sizeof(In_T) > sizeof(Out_T)) {
		out_val = (in_radix > out_radix) ? (val >> (in_radix - out_radix)) : (val << (out_radix - in_radix));
	} else {
		out_val = (in_radix > out_radix) ? ((Out_T)val >> (in_radix - out_radix)) : (val << (out_radix - in_radix)) ;
	}

	return out_val;
}

template <int magnitude, bool safe_checks = true, typename Storage_T = int32_t, typename Math_T = int64_t>
class FixedPoint {
public:
	typedef Storage_T Storage_t;
	class OverflowException : std::exception { };

	static_assert(sizeof(Storage_T) <= sizeof(Math_T));
	typedef FixedPoint<magnitude, safe_checks, Storage_T, Math_T> Self_T;

protected:
	Storage_T Value;


	// Internal-use contructor for passing the raw storage type.
	constexpr FixedPoint(bool raw_flag_ignored, Storage_T val) : Value(val) { }


	template <typename Value_T>
	static constexpr void CheckRange(Value_T val) {
		if (!safe_checks)
			return;
		if (!FixedPoint_CheckRange<Value_T, sizeof(Value_T) * 8 - 1, Storage_T, magnitude>(val))
			throw OverflowException();
		/*Storage_T min = SignMask() >> Radix();
		Storage_T max = ~min;
		if ((val - 1) >= max || val < min)
			throw OverflowException();*/
	}


public:
	constexpr FixedPoint() : Value(0) {}

	constexpr FixedPoint(int val) : Value((Storage_T)val << Self_T::Radix()) {
		CheckRange(val);
	}

	constexpr FixedPoint(unsigned val) : Value((Storage_T)val << Self_T::Radix()) {
		CheckRange(val);
	}

	constexpr FixedPoint(float val) : Value((Storage_T)(val * ConversionFactor<float>())) {
		CheckRange(val);
	}

	constexpr FixedPoint(double val) : Value((Storage_T)(val * ConversionFactor<double>())) {
		CheckRange(val);
	}

	constexpr FixedPoint(const Self_T& val) = default;

	template <int magnitude2, bool safe_checks2, typename Storage_T2, typename Math_T2>
	constexpr FixedPoint(FixedPoint<magnitude2, safe_checks2, Storage_T2, Math_T2> val) {
		Value = FixedPoint_ConvertFixed<Storage_T2, magnitude2, Storage_T, magnitude>(val.GetRaw(), safe_checks);
		/*Storage_T2 val_raw = val.GetRaw();
		int radix = val.Radix();
		if (magnitude2 > magnitude)
			CheckRange(val_raw >> val.Radix());
		if (sizeof(Storage_T2) > sizeof(Storage_T)) {
			Value = (radix > Radix()) ? (val_raw >> (radix - Radix())) : (val_raw << (Radix() - radix));
		} else {
			Value = (radix > Radix()) ? ((Storage_T)val_raw >> (radix - Radix())) : (val_raw << (Radix() - radix)) ;
		}*/
	}

	Self_T& operator=(const Self_T& val) = default;

	constexpr Storage_T GetRaw() const { return Value; }
	
	void SetRaw(Storage_T val) { Value = val; }

	constexpr Self_T Copy() const {
		return Self_T(*this);
	}
	
	template <typename Out_T>
	constexpr Out_T toFP() const {
		return (Out_T)Value * InvConversionFactor<Out_T>();
	}

	template <typename Out_T>
	constexpr Out_T toInt() const {
		return (magnitude < 0) ? 0 : (Value >> Self_T::Radix());
	}

	template <typename Int_T, int magnitude2>
	constexpr Int_T toFixed() {
		return FixedPoint_ConvertFixed<Storage_T, magnitude, Int_T, magnitude2>(Value, safe_checks);
	}

	constexpr operator double() const {
		return toFP<double>();
		//return (double)Value * InvConversionFactor<double>();
	}

	constexpr operator float() const {
		return toFP<float>();
		//return (float)Value * InvConversionFactor<float>();
	}

	constexpr operator int() const {
		return toInt<int>();
		//return (magnitude < 0) ? 0 : (Value >> Self_T::Radix());
	}

	constexpr bool operator==(const Self_T& b) const {
		return Value == b.Value;
	}

	constexpr bool operator<(const Self_T& b) const {
		return Value < b.Value;
	}

	constexpr bool operator>(const Self_T& b) const {
		return Value > b.Value;
	}

	constexpr bool operator<=(const Self_T& b) const {
		return Value <= b.Value;
	}

	constexpr bool operator>=(const Self_T& b) const {
		return Value >= b.Value;
	}
	
	Self_T& operator=(unsigned val) {
		//assert((SignMask() & Value) == 0);
		Value = Self_T(val).GetRaw();
		return *this;
	}

	Self_T& operator=(int val) {
		//assert(magnitude >= 0);
		//Value = (Storage_T)val << Self_T::Radix();
		//assert((SignMask() & Value) == 0);
		Value = Self_T(val).GetRaw();
		return *this;
	}

	Self_T& operator=(double val) {
		//Value = (Storage_T)(val * ConversionFactor<double>());
		Value = Self_T(val).GetRaw();
		return *this;
	}

	Self_T& operator=(float val) {
		Value = Self_T(val).GetRaw();
		//Value = (Storage_T)(val * ConversionFactor<float>());
		return *this;
	}

	Self_T& operator+=(const Self_T& val) {
		if (safe_checks) {
			Math_T t = Value + val.Value;
			CheckRange(t >> Self_T::Radix());
			Value = (Storage_T)t;
		} else {
			Value += val.Value;
		}
		return *this;
	}

	Self_T& operator-=(const Self_T& val) {
		if (safe_checks) {
			Math_T t = Value - val.Value;
			CheckRange(t >> Self_T::Radix());
			Value = (Storage_T)t;
		} else {
			Value -= val.Value;
		}
		return *this;
	}

	Self_T& operator*=(const Self_T& val) {
		Math_T sum = (Math_T)Value * val.Value;
		sum >>= Self_T::Radix();
		CheckRange(sum >> Self_T::Radix());
		Value = (Storage_T)sum;
		return *this;
	}

	Self_T& operator*=(int val) {
		Math_T sum = (Math_T)Value * val;
		//sum >>= Self_T::Radix();
		CheckRange(sum >> Self_T::Radix());
		Value = (Storage_T)sum;
		return *this;
	}

	Self_T& operator/=(const Self_T& val) {
		Math_T a = (Math_T)Value;
		a <<= Self_T::Radix();
		Value = (Storage_T)(a / (Math_T)val.Value);
		return *this;
	}

	Self_T& operator>>=(int count) {
		Value >>= count;
		return *this;
	}

	Self_T& operator<<=(int count) {
		Value <<= count;
		return *this;
	}

	constexpr Self_T operator-() const {
		return Self_T(true, -Value);
	}

	constexpr Self_T operator+(const Self_T& val) const {
		return Copy() += val;
	}

	constexpr Self_T operator-(const Self_T& val) const {
		return Copy() -= val;
	}

	constexpr Self_T operator*(const Self_T& val) const {
		return Copy() *= val;
	}

	constexpr Self_T operator*(int val) const {
		return Copy() *= val;
	}

	constexpr Self_T mult(int val) const {
		return Copy() *= val;
	}

	constexpr Self_T operator/(const Self_T& val) const {
		return Copy() /= val;
	}

	constexpr Self_T operator>>(int count) const {
		return Copy() >>= count;
	}

	constexpr Self_T operator<<(int count) const {
		return Copy() <<= count;
	}

	template <typename Precision_T=double>
	Self_T Sqrt() const {
		Self_T t(sqrt((Precision_T)*this));
		return t;
	}

	// Number of bits representing the stored whole-number magnitude.
	inline static constexpr int Magnitude() {
		return magnitude;
	}

	// Number of bits representing the stored fractional component.
	inline static constexpr int Radix() {
		return (int)StorageBits() - magnitude - 1;
	}

	// Multiplication factor for converting from floating point to fixed point.
	template <typename Fac_T = double>
	inline static constexpr Fac_T ConversionFactor() {
		return (magnitude < 0) ? ((Fac_T)1.0 / ((Math_T)1 << Radix())) : (Fac_T)((Math_T)1 << Radix());
	}

	// Multiplication factor for converting from fixed point to floating point.
	template <typename Fac_T = double>
	inline static constexpr Fac_T InvConversionFactor() {
		return (Fac_T)1.0 / Self_T::ConversionFactor<Fac_T>();
	}

	inline static constexpr size_t StorageBits() {
		return sizeof(Storage_T) * 8;
	}

	inline static constexpr size_t MathBits() {
		return sizeof(Math_T) * 8;
	}

	inline static constexpr Storage_T SignMask() {
		return (Storage_T)1 << (StorageBits() - 1);
	}

	inline static constexpr Self_T MaxVal() {
		return Self_T(true, ~SignMask());
	}

	inline static constexpr Self_T MinVal() {
		return Self_T(true, SignMask());
	}

	inline static constexpr Self_T SmallestPositive() {
		return Self_T(true, 1);
	}

	inline static constexpr Self_T SmallestNegative() {
		return Self_T(true, 1);
	}

	inline static constexpr Self_T fromRaw(Storage_T val) {
		return Self_T(true, val);
	}

	template <typename Int_T, int magnitude2>
	static constexpr Self_T fromFixed(Int_T val) {
		Storage_T val_raw = FixedPoint_ConvertFixed<Int_T, magnitude2, Storage_T, magnitude>(val, safe_checks);
		return Self_T(true, val_raw);
	}

	template <typename Int_T>
	static constexpr Self_T fromFixed(Int_T val, int magnitude2) {
		Storage_T val_raw = FixedPoint_ConvertFixed<Int_T, magnitude2, Storage_T, magnitude>(val, safe_checks);
		return Self_T(true, val_raw);
	}
protected:
};

}
