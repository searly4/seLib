#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <atomic>
/// \endcond

//#include "SimpleBuffer.h"
//#include <IDCLib/Platform.h>

#define SIZE_OF_ARRAY(array) (sizeof(array) / sizeof(array[0]))

#define IDCLIB_STR_HELPER(x) #x
#define IDCLIB_STR(x) IDCLIB_STR_HELPER(x)


//==================================================================================================
namespace seLib {


template <typename T>
T volatile & AsVolatile(T& val) noexcept { return static_cast<T volatile &>(val); }

template <typename T>
T const volatile & AsVolatile(T const & val) noexcept { return static_cast<T const volatile &>(val); }


///-------------------------------------------------------------------------------------------------
/// @fn	inline int ConstexprAssertFail()
/// @brief	Calling this function causes a constexpr context to fail.
/// @returns	An int.
inline int ConstexprAssertFail() {
	static int i = 5;
	return i++;
}


template <typename T>
constexpr bool IsBetween(T value, T a, T b) {
	return (a < b && a <= value && value <= b)
		|| (a >= value && value >= b);
}


///-------------------------------------------------------------------------------------------------
/// @class	OptionalHeapContainer_t
/// @brief	Container for managing a single object on the heap. Able to allocate and
/// 		deallocate on demand.
/// @tparam	T	Generic type parameter.
template <typename T>
class OptionalHeapContainer_t {
protected:
	T* mPtr = nullptr;

public:
	inline void Delete() noexcept {
		if (mPtr != nullptr) {
			T* t = mPtr;
			mPtr = nullptr;
			delete t;
		}
	}
	
	OptionalHeapContainer_t() {}

	OptionalHeapContainer_t(OptionalHeapContainer_t<T>&& source) {
		T* t = source.mPtr;
		source.mPtr = nullptr;
		mPtr = t;
	}

	OptionalHeapContainer_t& operator=(OptionalHeapContainer_t<T>&& source) {
		Delete();
		
		T* t = source.mPtr;
		source.mPtr = nullptr;
		mPtr = t;
	}

	~OptionalHeapContainer_t() noexcept {
		Delete();
	}

	template <typename... Args>
	T& Instantiate(Args... args) {
		Delete();
		mPtr = new T(args...);
		return *mPtr;
	}
	
	T* operator->() noexcept {
		return mPtr;
	}
	
	T& operator*() noexcept {
		return *mPtr;
	}
	
	operator T* () noexcept {
		return mPtr;
	}
	
	inline bool IsNull() const noexcept {
		return mPtr == nullptr;
	}
};


///-------------------------------------------------------------------------------------------------
/// @class	AccessGuard_t
/// @brief	Wrapper class that adds automatic atomic locking when performing a modification
/// 		operation on the contained data.
/// @tparam	T	  	Generic type parameter.
/// @tparam	Lock_T	Type of the lock t.
///
/// ### tparam	T	  	Generic type parameter.
/// ### tparam	Lock_T	Type of the lock.
template <typename T, typename Lock_T>
class AccessGuard_t {
private:
	T mValue;
	
public:
	class Lock_t {
	protected:
		friend class AccessGuard_t<T, Lock_T>;
		Lock_T mLockObj;
		T* const mValue;
	public:
		Lock_t(T& value)
			: mValue(&value)
		{}
		
		T* operator->() const noexcept {
			return mValue;
		}
	};
	
	AccessGuard_t() noexcept { }

	template <typename... Args_T>
	AccessGuard_t(Args_T... args)
		: mValue(args...)
	{ }

	template <typename Args_T>
	AccessGuard_t(std::initializer_list<Args_T> args)
		: mValue(args)
	{ }

	/// @brief	Gets read access to the contained data.
	/// @return	A pointer to a const.
	T const * R() const noexcept {
		return &mValue;
	}

	/// @brief	Gain write access through a lock object.
	/// @return	The lock/accessor object.
	Lock_t W() noexcept {
		return Lock_t(mValue);
	}

	/// @brief	Gain write access without a lock.
	/// @return	A reference to the contained data.
	T & Bypass() const noexcept {
		return mValue;
	}
};// */


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T> constexpr bool MemCmp(T const * a, T const * b, size_t size) noexcept
/// @brief	Bitwise compare two blocks of memory
/// @tparam	T	Generic type parameter.
/// @param	a   	A T to process.
/// @param	b   	A T to process.
/// @param	size	The size.
/// @returns	True if it succeeds, false if it fails.
template <typename T>
constexpr bool MemCmp(T const * a, T const * b, size_t size) noexcept {
	if (!size)
		return false;
	do {
		if (!(*a == *b))
			return false;
		a++;
		b++;
		size--;
	} while (size) ;
	return true;
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Callback_T, typename... Args_T> void TryCallback(Callback_T callback, Args_T... args)
/// @brief	Helper for safely calling a function pointer.
/// @tparam	Callback_T	The function pointer type.
/// @tparam	Args_T	  	Type of the arguments to the referenced function.
/// @param	callback	The function pointer.
/// @param	args		Arguments passed to the referenced function.
template <typename Callback_T, typename... Args_T>
void TryCallback(Callback_T callback, Args_T... args) {
	if (callback != nullptr)
		callback(args...);
}


template <typename Mask_T>
struct ValueMask_t {
	template <typename... Value_T>
	constexpr static inline Mask_T mask(Value_T... value) {
		return (((Mask_T)1 << (Mask_T)value) | ...);
	}
};



template <typename T, typename Enum_T>
constexpr T MaskEnum(Enum_T e) noexcept {
	//typedef typename std::underlying_type<Enum_T>::type e_t;
	if constexpr (std::is_enum<T>::value)
		static_assert((size_t)e < sizeof(T) * 8, "Enum will not fit in mask value type.");
    return static_cast<T>(1) << (size_t)e;
}
//template <typename T>
//TransmitPredefined_t(RFIDReg_e, Args&&... t) -> TransmitPredefined_t<sizeof...(Args) + 1>;


///-------------------------------------------------------------------------------------------------
/// @class	Bits
/// @brief	Implements an offset bit range within a larger integer type.
/// @tparam	T		  	The integer type.
/// @tparam	Offset_N  	The number of bits offset.
/// @tparam	BitCount_N	The number of bits for the value.
template <typename T, size_t Offset_N, size_t BitCount_N>
class Bits {
private:
	static inline constexpr T mMask = static_cast<T>(MaskBits<T>(BitCount_N) << Offset_N);
	T mValue;

	static inline constexpr T mConvertTo(T value) noexcept {
		return static_cast<T>(value << Offset_N) & mMask;
	}

	static inline constexpr T mConvertFrom(T value) noexcept {
		return static_cast<T>(value >> Offset_N) & mMask;
	}

public:
	static_assert(std::is_integral_v<T>, "Type must be integral.");

	Bits& operator=(T newval) noexcept {
		mValue = (mValue & ~mMask) | mConvertTo(newval);
		return *this;
	}

	constexpr operator T() const noexcept {
		return mConvertFrom(mValue);
	}
};


///-------------------------------------------------------------------------------------------------
/// @class	Bits<std::atomic<T>,Offset_N,BitCount_N>
/// @brief	Implements an offset bit range within a larger integer type. Specialization for atomic types.
/// @tparam	T		  	The integer type.
/// @tparam	Offset_N  	The number of bits offset.
/// @tparam	BitCount_N	The number of bits for the value.
template <typename T, size_t Offset_N, size_t BitCount_N>
class Bits<std::atomic<T>, Offset_N, BitCount_N> {
private:
	using Core_t = Bits<T, Offset_N, BitCount_N>;
	std::atomic<T> mValue;

public:
	static_assert(std::is_integral_v<T>, "Type must be integral.");

	Bits& operator=(T newval) noexcept {
		newval = Core_t::mConvertTo(newval);
		T oldval = mValue.load();

		while (!mValue.compare_exchange_weak(oldval, (oldval & ~Core_t::mMask) | newval)) {}

		return *this;
	}

	constexpr operator T() const noexcept {
		return Core_t::mConvertFrom(mValue);
	}
};




} // namespace IDCLib
//==================================================================================================

