#pragma once

/// \cond
/// \endcond

// C++ only below here
#ifdef   __cplusplus

/// \cond
#include <algorithm>
#include <type_traits>
#include "Util.h"
#include "BitMask.h"
/// \endcond


//==================================================================================================
namespace seLib { namespace Timing {


template <typename T>
constexpr T Mask(unsigned int bits) {
	return (T)((~static_cast<typename std::make_unsigned<T>::type>(0)) << bits);
}


template <typename T>
class IntervalMask_t {
public:
	T LastValue = 0;
	T Latch = 0;


	///-------------------------------------------------------------------------------------------------
	/// @fn	void Increment(T timestamp)
	/// @brief	Latch all bits that have ticked over since the last increment.
	/// @param	timestamp	The timestamp.
	T Increment(T timestamp) {
		T mask = timestamp ^ LastValue;
		LastValue = timestamp;

		// Latch all bits to the right of the highest changed bit.
		//for (size_t i = sizeof(T) * 4; i != 0; i >>= 1)
		//	mask |= (mask >> i);
		mask = seLib::FillBitsLow(mask);

		Latch |= mask;

		return mask;
	}
	
	bool Check(T mask) const {
		return Latch & mask;
	}

	T Clear(T mask) {
		T value = Latch & mask;
		Latch &= ~mask;
		return value;
	}

	T Set(T mask) {
		T value = Latch & mask;
		Latch |= mask;
		return value ^ mask;
	}
};


template <typename Value_T, typename Storage_T> class IntervalLatch_t;

///-------------------------------------------------------------------------------------------------
/// @class	IntervalLatch_t<Value_T,Value_T*>
/// @brief	Sets a flag every time a specified interval has passed.
/// @tparam	Value_T	Type of the timer value.
template <typename Value_T, typename Storage_T = Value_T>
class IntervalLatch_t {
protected:

public:
	Storage_T LastTime;
	Value_T Mask;
	Value_T Offset;
	
	
	constexpr IntervalLatch_t(Value_T mask, Value_T offset, Value_T & ext_storage)
		: LastTime(&ext_storage), Mask(mask), Offset(offset)
	{}

	constexpr IntervalLatch_t(Value_T mask, Value_T offset)
		: LastTime(0), Mask(mask), Offset(offset)
	{}

	IntervalLatch_t(IntervalLatch_t const &) = default;
	IntervalLatch_t& operator=(IntervalLatch_t const &) = default;
	
	
	inline Value_T MaskTime(Value_T time) const noexcept {
		return (time + Offset) & Mask;
	}

	inline bool Check(Value_T time) const noexcept {
		return (MaskTime(time) ^ AutoDeref(LastTime)) != 0;
	}

	
	template <typename T = bool>
	inline auto Update(Value_T time) const noexcept -> std::enable_if_t<std::is_pointer<Storage_T>::value, T> {
		Value_T mask_time = MaskTime(time);
		if ((mask_time ^ *LastTime) == 0)
			return false;
		
		*LastTime = mask_time;
		return true;
	}

	template <typename T = bool>
	inline auto Update(Value_T time) noexcept->std::enable_if_t<!std::is_pointer<Storage_T>::value, T> {
		Value_T mask_time = MaskTime(time);
		//if ((mask_time ^ AutoDeref(LastTime)) == 0)
		if ((mask_time ^ LastTime) == 0)
			return false;
		
		LastTime = mask_time;
		return true;
	}
};


template<class Value_T, typename Value_T2>
IntervalLatch_t(Value_T, Value_T2, Value_T &) -> IntervalLatch_t<Value_T, Value_T*>;


template<typename Value_T, typename Value_T2>
IntervalLatch_t(Value_T, Value_T2) -> IntervalLatch_t<Value_T, Value_T>;


template <typename Time_T>
class Timer_t {
public:
	typedef Time_T(* TimeFunction_t)() noexcept;

	Time_T const StartTime;

private:
	TimeFunction_t const mTimeFunction;
	Time_T & mTimeOutput;

public:

	inline Timer_t(Time_T & time_output, TimeFunction_t const time_function) :
		StartTime(time_function != nullptr ? time_function() : 0),
		mTimeFunction(time_function),
		mTimeOutput(time_output)
	{}

	inline Time_T Elapsed(Time_T time) const noexcept {
		return time - StartTime;
	}

	inline Time_T Elapsed() const noexcept {
		return mTimeFunction != nullptr ? Elapsed(mTimeFunction()) : 0;
	}
	
	inline ~Timer_t() noexcept {
		mTimeOutput = Elapsed();
	}
};


template <typename Time_T>
class TimeExecution_t {
public:
	typedef void(* Callback_t)(Time_T interval) noexcept;
	typedef Time_T(* TimeFunction_t)() noexcept;

	Time_T const StartTime;
	Time_T MarkElapsed;
	
private:
	TimeFunction_t const mTimeFunction;
	Callback_t const mCallback;

public:
	inline TimeExecution_t(TimeFunction_t const time_function, Callback_t callback) :
		StartTime(time_function != nullptr ? time_function() : Time_T{0}), mTimeFunction(time_function), mCallback(callback)
	{}

	inline ~TimeExecution_t() noexcept {
		if (mCallback != nullptr)
			mCallback(Elapsed());
	}
	
	inline Time_T Elapsed() const noexcept {
		return mTimeFunction != nullptr ? mTimeFunction() - StartTime : Time_T{0};
	}

	inline Time_T Mark() noexcept {
		return MarkElapsed = Elapsed();
	}
};


template <typename Time_T, typename Storage_T = size_t>
class RateCounter_t {
public:
	Time_T const Interval;
	Storage_T Accumulator { 0 };
	Storage_T Value { 0 };
	Time_T Match { 0 };
	
	auto& Add(Time_T time, Storage_T count = 1) noexcept {
		Accumulator += count;
		if (time > Match) {
			Value = (Value + Accumulator) >> 1;
			Accumulator = 0;
			Match += Interval;
		}
		return *this;
	}
};


template <typename Time_T>
class PeakInterval_t {
public:
	Time_T const TimeConstant;
	Time_T Value { 0 };
	Time_T Match { 0 };
	Time_T LastTime { 0 };
	
	auto& Add(Time_T time) noexcept {
		Value = std::max<Time_T>(time - LastTime, Value);
		LastTime = time;
		if (time > Match) {
			Value >>= 1;
			Match += TimeConstant;
		}
		return *this;
	}
};


}} // namespace seLib::Timing
//==================================================================================================


#endif // __cplusplus
