#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include "UnitTest.h"
/// \endcond


//==================================================================================================
namespace seLib {

///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T MaskBitsHigh(unsigned bits)
/// @brief	Return a value of the specified type with the highest n=bits bits set to 1.
/// @tparam	t_T	Type of the t.
/// @param	bits	The number of bits.
/// @returns	The mask value.
template <typename Int_T>
inline constexpr Int_T MaskBitsHigh(unsigned bits) {
	return static_cast<Int_T>(
		static_cast<typename std::make_unsigned<Int_T>::type>(~(Int_T)0) << (sizeof(Int_T) * 8 - bits)
	);
}
SELIB_UNITTEST_EQUALITY("MaskBitsHigh", MaskBitsHigh<uint8_t>(3), 0xE0);
SELIB_UNITTEST_EQUALITY("MaskBitsHigh", MaskBitsHigh<uint8_t>(0), 0x00);
SELIB_UNITTEST_EQUALITY("MaskBitsHigh", MaskBitsHigh<uint8_t>(8), 0xFF);

///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T MaskBitsLow(unsigned bits)
/// @brief	Return a value of the specified type with the lowest n=bits bits set to 1.
/// @tparam	t_T	Type of the t.
/// @param	bits	The number of bits.
/// @returns	The mask value.
template <typename Int_T>
inline constexpr Int_T MaskBitsLow(unsigned bits) {
	return ~(static_cast<Int_T>(
		static_cast<typename std::make_unsigned<Int_T>::type>(~(Int_T)0) << bits
	));
}
SELIB_UNITTEST_EQUALITY("MaskBitsLow", MaskBitsLow<uint8_t>(3), 0x07);
SELIB_UNITTEST_EQUALITY("MaskBitsLow", MaskBitsLow<uint8_t>(0), 0x00);
SELIB_UNITTEST_EQUALITY("MaskBitsLow", MaskBitsLow<uint8_t>(8), 0xFF);

///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T FillBitsLow(unsigned value)
/// @brief	Set all bits to lower than the highest set bit.
/// @tparam	Int_T	Integer type.
/// @param 	value	The value.
/// @returns	An Int_T.
template <typename Int_T>
inline constexpr Int_T FillBitsLow(Int_T value) {
	auto value_u = static_cast<typename std::make_unsigned<Int_T>::type>(value);
	for (size_t i = sizeof(Int_T) * 4; i != 0; i >>= 1)
		value_u |= (value_u >> i);
	return static_cast<Int_T>(value_u);
}
SELIB_UNITTEST_EQUALITY("FillBitsLow", FillBitsLow<uint8_t>(0x10), 0x1F);
SELIB_UNITTEST_EQUALITY("FillBitsLow", FillBitsLow<uint8_t>(0x00), 0x00);
SELIB_UNITTEST_EQUALITY("FillBitsLow", FillBitsLow<uint8_t>(0x80), 0xFF);


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename Int_T> inline constexpr Int_T FillBitsHigh(unsigned value)
/// @brief	Set all bits to higher than the lowest set bit.
/// @tparam	Int_T	Integer type.
/// @param 	value	The value.
/// @returns	An Int_T.
template <typename Int_T>
inline constexpr Int_T FillBitsHigh(Int_T value) {
	auto value_u = static_cast<typename std::make_unsigned<Int_T>::type>(value);
	for (size_t i = sizeof(Int_T) * 4; i != 0; i >>= 1)
		value_u |= (value_u << i);
	return static_cast<Int_T>(value_u);
}
SELIB_UNITTEST_EQUALITY("FillBitsHigh", FillBitsHigh<uint8_t>(0x20), 0xE0);
SELIB_UNITTEST_EQUALITY("FillBitsHigh", FillBitsHigh<uint8_t>(0x00), 0x00);
SELIB_UNITTEST_EQUALITY("FillBitsHigh", FillBitsHigh<uint8_t>(0x01), 0xFF);


} // namespace seLib
//==================================================================================================

