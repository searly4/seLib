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

namespace FixedPoint_util {

class FixedPoint_OverflowException : std::exception {
public:
	#ifdef __cpp_exceptions
	static inline void _throw() { throw FixedPoint_OverflowException(); }
	#else
	static inline void _throw() { abort(); }
	#endif
};

//class OverflowException : std::exception { };

// Return true if the specified value will fit in the output storage type.
template <typename In_T, typename Out_T>
constexpr
typename std::enable_if<std::is_unsigned<In_T>::value, bool>::type
CheckRange(In_T val, int In_Mag, int Out_Mag) {
    if (In_Mag <= Out_Mag)
        return true;
    val >>= (sizeof(In_T) * 8 - In_Mag);
    In_T mask = ((In_T)~(In_T)0) << In_Mag;
    return (val & mask) == 0;
}

template <typename In_T, typename Out_T>
constexpr
typename std::enable_if<std::is_integral<In_T>::value && !std::is_unsigned<In_T>::value, bool>::type
CheckRange(In_T val, int In_Mag, int Out_Mag) {
    if (In_Mag <= Out_Mag)
        return true;
    val >>= (sizeof(In_T) * 8 - In_Mag);
    In_T mask = ((In_T)~(In_T)0) << In_Mag;
    return ((val & mask) == 0) || ((~val & mask) == 0);
}

template <typename In_T, typename Out_T>
constexpr
typename std::enable_if<std::is_floating_point<In_T>::value, bool>::type
CheckRange(In_T val, int In_Mag, int Out_Mag) {
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


template <typename In_T, typename Out_T>
inline static constexpr bool CheckFixed(const In_T& val, int In_Mag, int Out_Mag) {
	int in_storagebits = sizeof(In_T) * 8;
	int in_radix = (int)in_storagebits - In_Mag - 1;
	int out_storagebits = sizeof(Out_T) * 8;
	int out_radix = (int)out_storagebits - Out_Mag - 1;

	return !(In_Mag > Out_Mag && !CheckRange<In_T, Out_T>(val >> in_radix, In_Mag, Out_Mag));
}

// Convert a fixed point value from one size/radix type to another.
template <typename In_T, typename Out_T>
static constexpr Out_T ConvertFixed(const In_T& val, int In_Mag, int Out_Mag, bool check = true) {
    int in_storagebits = sizeof(In_T) * 8;
    int in_radix = (int)in_storagebits - In_Mag - 1;
    int out_storagebits = sizeof(Out_T) * 8;
    int out_radix = (int)out_storagebits - Out_Mag - 1;

    Out_T out_val = 0;

    //if (check && In_Mag > Out_Mag && !CheckRange<In_T, In_Mag, Out_T, Out_Mag>(val >> in_radix))
    //    throw FixedPoint_OverflowException();

    if (sizeof(In_T) > sizeof(Out_T)) {
        out_val = (in_radix > out_radix) ? (val >> (in_radix - out_radix)) : (val << (out_radix - in_radix));
    } else {
        out_val = (in_radix > out_radix) ? ((Out_T)val >> (in_radix - out_radix)) : (val << (out_radix - in_radix));
    }

    return out_val;
}

const uint64_t _Sqrt2_2 = 13043817825332782212ull; // sqrt(0.5) in radix 64

}

template <int magnitude, bool safe_checks = true, typename Storage_T = int32_t, typename Math_T = int64_t>
class FixedPoint {
public:
	typedef Storage_T Storage_t;

	static_assert(sizeof(Storage_T) <= sizeof(Math_T));
	typedef FixedPoint<magnitude, safe_checks, Storage_T, Math_T> Self_T;

protected:
	Storage_T Value;


	// Internal-use contructor for passing the raw storage type.
	constexpr FixedPoint(bool raw_flag_ignored, Storage_T val) : Value(val) { }


	template <typename Value_T>
	static inline constexpr bool CheckRange(const Value_T& val) {
		if (!safe_checks)
			return true;
		//if (!FixedPoint_util::CheckRange<Value_T, sizeof(Value_T) * 8 - 1, Storage_T, magnitude>(val))
		return FixedPoint_util::CheckRange<Value_T, Storage_T>(val, sizeof(Value_T) * 8 - 1, magnitude);
	}

	static constexpr Storage_t Convert(int val) {
		return ((Storage_T)val << Self_T::Radix());
	}

	static constexpr Storage_t Convert(unsigned val) {
		return ((Storage_T)val << Self_T::Radix());
	}

	static constexpr Storage_t Convert(float val) {
		return ((Storage_T)(val * ConversionFactor<float>()));
	}

	static constexpr Storage_t Convert(double val) {
		return ((Storage_T)(val * ConversionFactor<double>()));
	}

public:
	constexpr FixedPoint() : Value(0) {}

	constexpr FixedPoint(int val) {
		if (!CheckRange(val))
			FixedPoint_util::FixedPoint_OverflowException::_throw();
		Value = Convert(val);
	}

	constexpr FixedPoint(unsigned val) {
		if (!CheckRange(val))
			FixedPoint_util::FixedPoint_OverflowException::_throw();
		Value = Convert(val);
	}

	constexpr FixedPoint(float val) {
		if (!CheckRange(val))
			FixedPoint_util::FixedPoint_OverflowException::_throw();
		Value = Convert(val);
	}

	constexpr FixedPoint(double val) {
		if (!CheckRange(val))
			FixedPoint_util::FixedPoint_OverflowException::_throw();
		Value = Convert(val);
	}

	template <int magnitude2, bool safe_checks2, typename Storage_T2, typename Math_T2>
	constexpr FixedPoint(FixedPoint<magnitude2, safe_checks2, Storage_T2, Math_T2> val) {
		if (safe_checks && !FixedPoint_util::CheckFixed<Storage_T2, Storage_T>(val.GetRaw(), magnitude2, magnitude))
			__throw_exception_again FixedPoint_util::FixedPoint_OverflowException();
		Value = FixedPoint_util::ConvertFixed<Storage_T2, Storage_T>(val.GetRaw(), magnitude2, magnitude);
	}

    constexpr FixedPoint(const Self_T& val) = default;
    constexpr FixedPoint(Self_T&& val) = default;

    Self_T& operator=(const Self_T& val) = default;
    Self_T& operator=(Self_T&& val) = default;

	template <int magnitude2, bool safe_checks2, typename Storage_T2, typename Math_T2>
	constexpr Self_T& operator=(FixedPoint<magnitude2, safe_checks2, Storage_T2, Math_T2> val) {
		if (safe_checks && !FixedPoint_util::CheckFixed<Storage_T2, Storage_T>(val.GetRaw(), magnitude2, magnitude))
			__throw_exception_again FixedPoint_util::FixedPoint_OverflowException();
		Value = FixedPoint_util::ConvertFixed<Storage_T2, Storage_T>(val.GetRaw(), magnitude2, magnitude);
	}

    template<class T>
    constexpr Self_T& operator=(const T& val) {
		if (!CheckRange(val))
			__throw_exception_again FixedPoint_util::FixedPoint_OverflowException();
		Value = Convert(val);
        //Value = Self_T(val).Value;
        return *this;
    }

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
		if (safe_checks && !FixedPoint_util::CheckFixed<Storage_T, Int_T>(Value, magnitude, magnitude2))
			__throw_exception_again FixedPoint_util::FixedPoint_OverflowException();

		return FixedPoint_util::ConvertFixed<Storage_T, Int_T>(Value, magnitude, magnitude2);
	}

	constexpr operator double() const {
		return toFP<double>();
	}

	constexpr operator float() const {
		return toFP<float>();
	}

	constexpr operator int() const {
		return toInt<int>();
	}

	constexpr bool operator==(const Self_T& b) const {
		return Value == b.Value;
	}

    template<class T>
    constexpr typename std::enable_if<std::is_integral<T>::value, bool >::type operator==(const T& val) {
        return ((Value & ~((Storage_T)-1 << Radix())) == 0) &&
            ( (sizeof(T) > sizeof(Storage_T)) ? (val == (Value >> Radix())) : ((Value >> Radix()) == val) );
    }

    template<class T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, bool >::type operator==(const T& val) {
	    if (!CheckRange(val))
		    return false;
		Storage_T comp_val = Convert(val);
        return Value == comp_val;
    }

	constexpr bool operator<(const Self_T& b) const {
		return Value < b.Value;
	}

	constexpr bool operator>(const Self_T& b) const {
		return Value > b.Value;
	}

    template<class T>
    constexpr typename std::enable_if<std::is_integral<T>::value, bool >::type operator<(const T& val) {
        return ((Value & ~((Storage_T)-1 << Radix())) == 0) && 
            ( (sizeof(T) > sizeof(Storage_T)) ? (val == (Value >> Radix())) : ((Value >> Radix()) == val) );
    }

    template<class T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, bool >::type operator<(const T& val) {
	    if (!CheckRange(val))
		    return (val > 0);
		Storage_T comp_val = Convert(val);
        return Value < comp_val;
    }

    template<class T>
    constexpr typename std::enable_if<std::is_integral<T>::value, bool >::type operator>(const T& val) {
        return ((Value & ~((Storage_T)-1 << Radix())) == 0) &&
            ((sizeof(T) > sizeof(Storage_T)) ? (val == (Value >> Radix())) : ((Value >> Radix()) == val));
    }

    template<class T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, bool >::type operator>(const T& val) {
	    if (!CheckRange(val))
		    return (val < 0);
		Storage_T comp_val = Convert(val);
        return Value > comp_val;
    }

    constexpr bool operator<=(const Self_T& b) const {
		return Value <= b.Value;
	}

	constexpr bool operator>=(const Self_T& b) const {
		return Value >= b.Value;
	}
	
    /*
	Self_T& operator=(unsigned val) {
		Value = Self_T(val).GetRaw();
		return *this;
	}

	Self_T& operator=(int val) {
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
    */

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

    template<class T>
    constexpr Self_T operator+(const T& val) const {
        return Copy() += Self_T(val);
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

    constexpr Self_T frac() const {
        Storage_T mask = ~((Storage_T)-1 << Radix());
        Self_T ret = fromRaw(Value & mask);
        return ret;
        //return fromRaw(Value & mask);
    }

    constexpr Self_T floor() const {
        Storage_T mask = (Storage_T)-1 << Radix();
        return fromRaw(Value & mask);
    }

    constexpr Self_T round() const {
        Storage_T mask = (Storage_T)-1 << Radix();
        Storage_T val = Value + ((Storage_T)1 << (Radix() - 1));
        return fromRaw(val & mask);
    }

	/*template <typename Precision_T=double>
	Self_T Sqrt() const {
		Self_T t(sqrt((Precision_T)*this));
		return t;
	}*/

    constexpr Self_T pow(unsigned exp) {
        Self_T result = 1;
        Self_T base = *this;
        while (exp) {
            if (exp & 1)
                result *= base;
            exp >>= 1;
            if (!exp)
                break;
            base *= base;
        }
        return result;
    }

    /*constexpr Self_T pow(Self_T exp) {
        if (Value < 0 || exp.Value < 0)
            throw FixedPoint_util::FixedPoint_OverflowException();

        int exp_int = (int)exp;
        Self_T result = pow(exp_int);
        exp -= (Self_T)exp_int;

        Storage_T mask1 = ((Storage_T)1) << (Radix() - 1);
        Storage_T mask2 = ((Storage_T)1) << (Radix() - 2);
        //Self_T _sqrt = Self_T::fromRaw(FixedPoint_util::_Sqrt2_2 >> (sizeof(FixedPoint_util::_Sqrt2_2) * 8 - Self_T::Radix() + 1));
        //Self_T _half = (Self_T)1;
        Self_T _sqrt = sqrt() >> 1;
        Self_T _half = *this >> 1;

        while (mask1) {
            if (mask1 & exp.Value)
                result *= *this * _sqrt;
            if (mask2 & exp.Value)
                result *= *this * _half;
            mask1 >>= 2;
            mask2 >>= 2;
            _sqrt >>= 1;
            _half >>= 1;
        }
        return result;
    }*/

    constexpr Self_T sqrt() {
        // Based on implementation given in Wikipedia:
        // https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_(base_2)
        // Modified for fixed point
        Math_T result = 0;
        Math_T bit = ((Math_T)1) << (sizeof(Storage_T) * 8 + Radix() - 2); // The second-to-top bit is set: 1 << 30 for 32 bits
        Math_T val = Value << Radix();
        
        while (bit > val) // "bit" starts at the highest power of four <= the argument.
            bit >>= 2;

        while (bit != 0) {
            if (val >= result + bit) {
                val -= result + bit;
                result = (result >> 1) + bit;
            } else
                result >>= 1;
            bit >>= 2;
        }
        return Self_T::fromRaw(result);
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

	inline static constexpr Self_T fromRaw(const Storage_T& val) {
        Self_T ret = Self_T(true, val);
		return ret;
	}

	template <typename Int_T, int magnitude2>
	static constexpr Self_T fromFixed(Int_T val) {
		if (safe_checks && !FixedPoint_util::CheckFixed<Int_T, Storage_T>(val, magnitude2, magnitude))
			FixedPoint_util::FixedPoint_OverflowException::_throw();

		Storage_T val_raw = FixedPoint_util::ConvertFixed<Int_T, Storage_T>(val, magnitude2, magnitude);
		Self_T outval;
		outval.Value = val_raw;
		return outval;
	}

	template <typename Int_T>
	static constexpr Self_T fromFixed(Int_T val, int magnitude2) {
		if (safe_checks && !FixedPoint_util::CheckFixed<Int_T, Storage_T>(val, magnitude2, magnitude))
			FixedPoint_util::FixedPoint_OverflowException::_throw();

		Storage_T val_raw = FixedPoint_util::ConvertFixed<Int_T, Storage_T>(val, magnitude2, magnitude);
		Self_T outval;
		outval.Value = val_raw;
		return outval;
	}
protected:
};

}
