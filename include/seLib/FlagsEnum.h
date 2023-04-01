#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
/// \endcond

// C++ only below here
#ifdef   __cplusplus

/// \cond
#include <algorithm>
#include <array>
#include <atomic>
/// \endcond

//#include <atomic> can't find confirmation that interrupts are disabled during atomic ops
#include "Endian.h"


//==================================================================================================
namespace seLib {


// TODO: Add flag argument checking.


///-------------------------------------------------------------------------------------------------
/// @class	FlagsEnum_t
/// @brief	The flags enum.
/// @tparam	Enum_T	Type of the enum t.
///
/// ### tparam	Enum_T	Type of the enum t.
template <typename Enum_T, typename Mask_T = unsigned, typename Storage_T = Mask_T>
class FlagsEnum_t {
public:
	using Flags = Enum_T;
	using Storage_t = Mask_T;
	using My_T = FlagsEnum_t;

	static inline constexpr bool IsAtomic = std::is_same<Storage_T, std::atomic<Mask_T>>::value;

	Storage_T Value;

protected:
	static constexpr inline Mask_T mFromEnums(Enum_T flag) noexcept {
		assert((Mask_T)flag < (Mask_T)(sizeof(Mask_T) * 8)); //, "Enum value overflows bit field.");
		return (Mask_T)1 << (Mask_T)flag;
	}

	static constexpr Mask_T mFromEnums(std::initializer_list<Enum_T> flags) noexcept {
		Mask_T values = 0;
		for (auto iter = flags.begin(); iter != flags.end(); iter++) {
			values |= My_T::mFromEnums(*iter);
		}
		return values;
	}

	constexpr FlagsEnum_t(Mask_T flags) noexcept
		: Value(flags) {}

public:
	constexpr FlagsEnum_t() noexcept
		: Value(0) {}

	constexpr FlagsEnum_t(Enum_T single_flag) noexcept
		: Value(mFromEnums(single_flag)) {}

	//constexpr FlagsEnum_t(std::initializer_list<Enum_T> flags) noexcept
	//	: Value(mFromEnums(flags)) {
	//}

	template <typename...Flags_T>
	constexpr FlagsEnum_t(Enum_T flag, Flags_T...flags) noexcept
		: Value((mFromEnums(flags) | ... | mFromEnums(flag)))
	{}

	constexpr FlagsEnum_t(My_T const &) = default;

	template <typename Mask2_T, typename Storage2_T>
	constexpr FlagsEnum_t(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) noexcept
		: Value((Mask_T)b.Value)
	{}

	template <typename Mask2_T, typename Storage2_T>
	constexpr My_T& operator=(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) const noexcept {
		Value = (Mask_T)b.Value;
		return *this;
	}

	template <typename Mask2_T, typename Storage2_T>
	constexpr My_T operator|(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) const noexcept {
		return My_T((Mask_T)Value | (Mask_T)b.Value);
	}

	template <typename Mask2_T, typename Storage2_T>
	constexpr My_T operator&(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) const noexcept {
		return My_T((Mask_T)Value & (Mask_T)b.Value);
	}

	template <typename Mask2_T, typename Storage2_T>
	My_T& operator|=(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) noexcept {
		static_assert(IsAtomic);
		Value |= (Mask_T)b.Value;
		return *this;
	}

	My_T& Set(Enum_T flag) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = mFromEnums(flag);
		Value |= flag_mask;
		return *this;
	}

	My_T& Set(std::initializer_list<Enum_T> flags) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = mFromEnums(flags);
		Value |= flag_mask;
		return *this;
	}

	template <typename Mask2_T, typename Storage2_T>
	My_T& Set(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = (Mask_T)b.Value;
		Value |= flag_mask;
		return *this;
	}

	My_T& Clear(Enum_T flag) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = mFromEnums(flag);
		Value &= ~flag_mask;
		return *this;
	}

	My_T& Clear(std::initializer_list<Enum_T> flags) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = mFromEnums(flags);
		Value &= ~flag_mask;
		return *this;
	}

	template <typename Mask2_T, typename Storage2_T>
	My_T& Clear(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) noexcept {
		static_assert(IsAtomic);
		auto flag_mask = (Mask_T)b.Value;
		Value &= ~flag_mask;
		return *this;
	}

	My_T& Assign(Enum_T flag, bool value) noexcept {
		return (value) ? Set(flag) : Clear(flag);
	}

	My_T TestAndSet(Enum_T flag) noexcept {
		auto flag_mask = mFromEnums(flag);
		return Value.fetch_or(flag_mask) & flag_mask;
	}
	
	My_T TestAndSet(std::initializer_list<Enum_T> flags) noexcept {
		auto flag_mask = mFromEnums(flags);
		return Value.fetch_or(flag_mask) & flag_mask;
	}

	My_T TestAndClear(Enum_T flag) noexcept {
		auto flag_mask = mFromEnums(flag);
		return Value.fetch_and(~flag_mask) & flag_mask;
	}
	
	//template <typename Atomic_T = Storage_T>
	//std::enable_if<std::is_same<Atomic_T, std::atomic<Mask_T>>::value, My_T>
	My_T TestAndClear(std::initializer_list<Enum_T> flags) noexcept {
		auto flag_mask = mFromEnums(flags);
		return Value.fetch_and(~flag_mask) & flag_mask;
	}

	template <typename Mask2_T, typename Storage2_T>
	constexpr bool operator==(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & b) const noexcept {
		return Value == b.Value;
	}

	constexpr inline bool IsSet(Enum_T flag) const noexcept {
		return (Value & mFromEnums(flag)) != 0;
	}

	template <typename Mask2_T, typename Storage2_T>
	constexpr inline bool Any(FlagsEnum_t<Enum_T, Mask2_T, Storage2_T> const & flags) const noexcept {
		return (flags.Value & Value) != 0;
	}

	constexpr inline bool Any(std::initializer_list<Enum_T> flags) const noexcept {
		return Any(mFromEnums(flags));
	}

	constexpr inline bool Any() const noexcept {
		return Value != 0;
	}
	
	constexpr inline bool operator[](Enum_T flag) const noexcept {
		return IsSet(flag);
	}
};


template <typename Enum_T, typename Mask_T = unsigned>
using AtomicFlagsEnum_t = FlagsEnum_t<Enum_T, Mask_T, std::atomic<Mask_T>>;


template <typename Enum_T, size_t Max_N,
	typename Storage_T = typename Endian::unsigned_integer_type<(Max_N + 8) / 8>::type,
	size_t Index_N = Max_N>
union FlagsEnumUnion_t {
	static inline constexpr bool Last = { false };
	FlagsEnumUnion_t<Enum_T, Max_N, Storage_T, Index_N - 1> Next;
	Storage_T Value;
};

template <typename Enum_T, size_t Max_N, typename Storage_T>
union FlagsEnumUnion_t<Enum_T, Max_N, Storage_T, 0> {
	static inline constexpr bool Last = { true };
	Storage_T Value;
};

} // namespace seLib
//==================================================================================================


#endif // __cplusplus
