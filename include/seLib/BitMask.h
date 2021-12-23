#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
/// \endcond


//==================================================================================================
namespace seLib {

///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T MaskBits(unsigned bits)
/// @brief	Return a value of the specified type with the rightmost n=bits bits set to 1.
/// @tparam	t_T	Type of the t.
/// @param	bits	The number of bits.
/// @returns	The mask value.
template <typename Int_T>
inline constexpr Int_T MaskBits(unsigned bits) {
	return static_cast<Int_T>(~(
		static_cast<typename std::make_unsigned<Int_T>::type>(~(Int_T)0) << bits
	));
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T FillBitsRight(unsigned value)
/// @brief	Latch all bits to the right of the highest set bit.
/// @tparam	Int_T	Integer type.
/// @param 	value	The value.
/// @returns	An Int_T.
template <typename Int_T>
inline constexpr Int_T FillBitsRight(Int_T value) {
	auto value_u = static_cast<typename std::make_unsigned<Int_T>::type>(value);
	for (size_t i = sizeof(Int_T) * 4; i != 0; i >>= 1)
		value_u |= (value_u >> i);
	return static_cast<Int_T>(value_u);
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T FillBitsLeft(unsigned value)
/// @brief	Latch all bits to the left of the lowest set bit.
/// @tparam	Int_T	Integer type.
/// @param 	value	The value.
/// @returns	An Int_T.
template <typename Int_T>
inline constexpr Int_T FillBitsLeft(Int_T value) {
	auto value_u = static_cast<typename std::make_unsigned<Int_T>::type>(value);
	for (size_t i = sizeof(Int_T) * 4; i != 0; i >>= 1)
		value_u |= (value_u << i);
	return static_cast<Int_T>(value_u);
}


} // namespace seLib
//==================================================================================================

