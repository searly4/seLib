#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <array>
/// \endcond

// C++ only below here
#ifdef   __cplusplus


//==================================================================================================
namespace seLib { namespace Sort {


template <typename T>
using CompareFunc_t = int (*)(T const & a, T const & b);

template <typename T>
struct BasicComparator_t {
	static inline constexpr int compare(T const & a, T const & b) {
		return (a == b) ? 0 : (a < b) ? -1 : 1;
	}
};

template <typename T>
struct FuncComparator_t {
	CompareFunc_t<T> Comparator;
	inline constexpr int compare(T const & a, T const & b) const {
		return Comparator(a, b);
	}
};


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T, size_t Size, typename Compare_T> constexpr std::array<size_t, Size> SortIndex(std::array<T, Size> const & in_array, Compare_T comp)
/// @brief	Basic selection sort that is constexpr compatible. Outputs a sorted array of indexes into the source array.
/// @tparam	T		 	Generic type parameter.
/// @tparam	Size	 	Type of the size.
/// @tparam	Compare_T	Type of the compare t.
/// @param 	in_array	Array of INS.
/// @param 	comp		The component.
/// @returns	The sorted values.
template <typename T, size_t Size, typename Compare_T>
constexpr std::array<size_t, Size> SortIndex(std::array<T, Size> const & in_array, Compare_T comp) {
	std::array<size_t, Size> out_array {};

	// find first element
	size_t lowest = 0;
	for (size_t i = 1; i < Size; i++) {
		if (comp.compare(in_array[i], in_array[lowest]) < 0)
			lowest = i;
	}
	out_array[0] = lowest;

	// sort remaining elements
	for (size_t i2 = 1; i2 < Size; i2++) {
		lowest = Size;
		for (size_t i = 0; i < Size; i++) {
			if (comp.compare(in_array[i], in_array[out_array[i2 - 1]]) > 0 && (lowest == Size || comp.compare(in_array[i], in_array[lowest]) < 0))
				lowest = i;
		}
		out_array[i2] = lowest;
	}
	
	return out_array;
}


/*template <typename T, size_t Size>
constexpr std::array<size_t, Size> SortIndex(std::array<T, Size> const & in_array, CompareFunc_t<T> comp) {
	return SortIndex<T, Size, FuncComparator_t>(in_array, FuncComparator_t<T> {comp});
}// */


template <typename T, size_t Size>
constexpr std::array<size_t, Size> SortIndex(std::array<T, Size> const & in_array) {
	BasicComparator_t<T> comp {};
	return SortIndex<T, Size, BasicComparator_t<T>>(in_array, comp);
}


template <typename Key_T, typename Value_T, size_t Size, typename Compare_T>
constexpr std::array<size_t, Size> SortBy(std::array<Key_T, Size> const & key_array, std::array<Value_T, Size> const & value_array, Compare_T comp) {
}


template <typename Key_T, typename Value_T, size_t Size>
constexpr std::array<size_t, Size> SortBy(std::array<Key_T, Size> const & key_array, CompareFunc_t<Key_T> comp) {
	return SortBy<Key_T, Size, FuncComparator_t>(key_array, FuncComparator_t<Key_T> {comp});
}


template <typename Key_T, typename Value_T, size_t Size>
constexpr std::array<size_t, Size> SortBy(std::array<Key_T, Size> const & key_array) {
	return SortBy<Key_T, Size, FuncComparator_t>(key_array, BasicComparator_t<Key_T> {});
}

template <typename T, size_t Size>
constexpr std::array<T, Size> MapIndex(std::array<size_t, Size> const & map, std::array<size_t, Size> const & values) {
	std::array<T, Size> out;
	for(size_t i = 0; i < Size; i++) {
		auto from_i = map[i];
		out[i] = values[from_i];
	}
	return out;
}

///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T, size_t Size, typename Compare_T> constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array)
/// @brief	Basic selection sort (not in-place) that is constexpr compatible.
/// @tparam	T		 	Generic type parameter.
/// @tparam	Size	 	Type of the size.
/// @tparam	Compare_T	Type of the compare t.
/// @param 	in_array	Array of INS.
/// @returns	The sorted values.
///
/// ### param 	comp	The component.
template <typename T, size_t Size, typename Compare_T>
constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array, Compare_T comp) {
	std::array<T, Size> out_array(in_array);

	// find first element
	size_t lowest = 0;
	for (size_t i = 1; i < Size; i++) {
		if (comp.compare(in_array[i], in_array[lowest]) < 0)
			lowest = i;
	}
	out_array[0] = in_array[lowest];

	// sort remaining elements
	for (size_t i2 = 1; i2 < Size; i2++) {
		lowest = Size;
		for (size_t i = 0; i < Size; i++) {
			if (comp.compare(in_array[i], out_array[i2 - 1]) > 0 && (lowest == Size || comp.compare(in_array[i], in_array[lowest]) < 0))
				lowest = i;
		}
		out_array[i2] = in_array[lowest];
	}
	
	return out_array;
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T, size_t Size> constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array, CompareFunc_t<T> comp)
/// @brief	Basic selection sort (not in-place) that is constexpr compatible.
/// @tparam	T   	Generic type parameter.
/// @tparam	Size	Type of the size.
/// @param	in_array	Array of INS.
/// @param	comp		The component.
/// @returns	The sorted values.
template <typename T, size_t Size>
constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array, CompareFunc_t<T> comp) {
	return Sort<T, Size, FuncComparator_t<T>>(in_array, FuncComparator_t<T> {comp});
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T, size_t Size> constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array)
/// @brief	Basic selection sort (not in-place) that is constexpr compatible.
/// @tparam	T   	Generic type parameter.
/// @tparam	Size	Type of the size.
/// @param	in_array	Array of INS.
/// @returns	The sorted values.
template <typename T, size_t Size>
constexpr std::array<T, Size> Sort(std::array<T, Size> const & in_array) {
	return Sort<T, Size, BasicComparator_t<T>>(in_array, BasicComparator_t<T> {});
}


}} // namespace IDCLib::Sort
//==================================================================================================

#endif // __cplusplus
