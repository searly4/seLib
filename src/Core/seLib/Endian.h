#pragma once

/*
   Copyright 2015, 2021 by Scott Early

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

#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <array>
#include "UnitTest.h"

namespace seLib {
namespace Endian {


template <unsigned int N> struct integer_type;
template <> struct integer_type<1> { using type = int8_t; };
template <> struct integer_type<2> { using type = int16_t; };
template <> struct integer_type<4> { using type = int32_t; };
template <> struct integer_type<8> { using type = int64_t; };

template <unsigned int N> struct unsigned_integer_type;
template <> struct unsigned_integer_type<1> { using type = uint8_t; };
template <> struct unsigned_integer_type<2> { using type = uint16_t; };
template <> struct unsigned_integer_type<4> { using type = uint32_t; };
template <> struct unsigned_integer_type<8> { using type = uint64_t; };



/// @fn	template <typename In_T, typename Out_T> mask_lowest_bits
/// @brief	Returns a mask value of type Out_T that masks LSB bits equivalent to the size of In_T.
template <typename In_T, typename Out_T>
struct mask_lowest_bits {
	inline static constexpr auto value {
		static_cast<Out_T>(static_cast<typename std::make_unsigned<Out_T>::type>(
			static_cast<typename std::make_unsigned<In_T>::type>(~In_T{0})
		))
	};
};

static_assert(mask_lowest_bits<uint8_t, uint16_t>::value == 0x00FF, "");
static_assert(mask_lowest_bits<int8_t, uint32_t>::value == 0x000000FF, "");


/// @fn	template <typename T> inline constexpr size_t BitsSize()
/// @brief	Returns the size of the type in bits.
template <typename T>
[[nodiscard]] inline constexpr size_t BitsSize() noexcept {
	return sizeof(T) * 8;
}

static_assert(BitsSize<uint8_t>() == 8, "");
static_assert(BitsSize<uint16_t>() == 16, "");


/// @fn	template <typename Value_T, typename Element_T> inline constexpr size_t PackRatio()
/// @brief	Returns the number of elements of type Element_T will fit in type Collection_T.
template <typename Collection_T, typename Element_T>
struct pack_ratio {
	inline static constexpr auto value {
		sizeof(Collection_T) / sizeof(Element_T)
	};
};

static_assert(pack_ratio<uint8_t, uint8_t>::value == 1, "");
static_assert(pack_ratio<uint16_t,uint8_t>::value == 2, "");
static_assert(pack_ratio<uint32_t, uint8_t>::value == 4, "");


template <size_t N, typename In_T, typename Out_T = typename integer_type<N * sizeof(In_T)>::type>
[[nodiscard]] inline constexpr Out_T FromLittleEndian(In_T const* in_value) noexcept {
	Out_T retval{ 0 };

	for (size_t i = N; i > 0; i--)
		retval = (retval << BitsSize<In_T>()) | in_value[i - 1];

	return retval;
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <size_t N, typename T = typename integer_type<N>::type> [[nodiscard]] inline constexpr T FromLittleEndian<size_t N, T = integer_type<N>::FromLittleEndian(std::array<uint8_t, N> in_value) noexcept
/// @brief	Create a native signed integer from an array containing integer values. The array is in little-
/// 		endian format.
/// @details Generally, you should pass in an array of bytes. If you pass in larger values, the individual values must
///          already be in native format. 
/// @tparam	N		Number of In_T items in the array.
/// @tparam	In_T	The type of item in the array.
/// @param 	in_value	The array of values.
/// @returns	Signed integer with a type that is N * sizeof(In_T) in size.
template <size_t N, typename In_T, typename Out_T = typename integer_type<N * sizeof(In_T)>::type>
[[nodiscard]] inline constexpr Out_T FromLittleEndian(std::array<In_T, N> const & in_value) noexcept {
	Out_T retval {0};

	for (size_t i = N; i > 0; i--)
		retval = (retval << BitsSize<In_T>()) | in_value[i - 1];

	return retval;
}


template <size_t N, typename In_T>
[[nodiscard]] inline constexpr auto FromLittleEndianU(In_T const* in_value) noexcept {
	return FromLittleEndian<N, In_T, typename unsigned_integer_type<N * sizeof(In_T)>::type>(in_value);
}



///-------------------------------------------------------------------------------------------------
/// @fn	template <size_t N> [[nodiscard]] inline constexpr auto FromLittleEndianU(std::array<uint8_t, N> in_value)
/// @brief	Create a native unsigned integer from a byte array containing an integer in little-endian format.
/// @tparam	N	Size of the array.
/// @param 	in_value	The little-endian value.
/// @returns	Unsigned integer.
template <size_t N, typename In_T>
[[nodiscard]] inline constexpr auto FromLittleEndianU(std::array<In_T, N> const & in_value) noexcept {
	return FromLittleEndian<N, In_T, typename unsigned_integer_type<N * sizeof(In_T)>::type>(in_value);
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <size_t N, typename T = typename integer_type<N>::type> [[nodiscard]] inline constexpr T FromBigEndian<size_t N, T = integer_type<N>::FromBigEndian(std::array<uint8_t, N> in_value)
/// @brief	Create a native signed integer from a byte array containing an integer in big-endian format.
/// @tparam	N	Size of the array.
/// @tparam	T	The return type.
/// @param 	in_value	The little-endian value.
/// @returns	Signed integer.
template <size_t N, typename In_T, typename Out_T = typename integer_type<N * sizeof(In_T)>::type>
[[nodiscard]] inline constexpr Out_T FromBigEndian(std::array<In_T, N> const & in_value) noexcept {
	Out_T retval {0};

	for (auto b : in_value)
		retval = (retval << BitsSize<In_T>()) | b;

	return retval;
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <size_t N> [[nodiscard]] inline constexpr auto FromBigEndianU(std::array<uint8_t, N> in_value)
/// @brief	Create a native unsigned integer from a byte array containing an integer in big-endian format.
/// @tparam	N	Size of the array.
/// @param 	in_value	The little-endian value.
/// @returns	Unsigned integer.
template <size_t N, typename In_T>
[[nodiscard]] inline constexpr auto FromBigEndianU(std::array<In_T, N> const & in_value) noexcept {
	return FromBigEndian<N, In_T, typename unsigned_integer_type<N * sizeof(In_T)>::type>(in_value);
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T> [[nodiscard]] inline constexpr typename std::enable_if<std::is_integral<T>::value, std::array<uint8_t, sizeof(T)>>::type ToLittleEndian(T in_value) noexcept
/// @brief	Converts an integer to a little-endian formatted byte array.
/// @tparam	T	Type of the integer.
/// @param 	in_value	The integer value.
/// @returns	Little-endian formatted byte array.
template <typename In_T, typename Out_T = uint8_t>
[[nodiscard]] inline constexpr typename std::enable_if<std::is_integral<In_T>::value, std::array<Out_T, pack_ratio<In_T, Out_T>::value>>::type
ToLittleEndian(In_T in_value) noexcept {
	constexpr auto element_count = pack_ratio<In_T, Out_T>::value;
	std::array<Out_T, element_count> retval {};

	for (size_t i = 0; i < element_count; i++, in_value >>= BitsSize<Out_T>())
		retval[i] = (Out_T)(in_value & mask_lowest_bits<In_T, Out_T>::value);

	return retval;
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T> [[nodiscard]] inline constexpr typename std::enable_if<std::is_integral<T>::value, std::array<uint8_t, sizeof(T)>>::type ToBigEndian(T in_value) noexcept
/// @brief	Converts an integer to a big-endian formatted byte array.
/// @tparam	T	Type of the integer.
/// @param 	in_value	The integer value.
/// @returns	Big-endian formatted byte array.
template <typename In_T, typename Out_T = uint8_t>
[[nodiscard]] inline constexpr typename std::enable_if<std::is_integral<In_T>::value, std::array<Out_T, pack_ratio<In_T, Out_T>::value>>::type
ToBigEndian(In_T in_value) noexcept {
	constexpr auto element_count = pack_ratio<In_T, Out_T>::value;
	std::array<Out_T, element_count> retval {};

	for (size_t i = element_count; i > 0; i--, in_value >>= BitsSize<Out_T>())
		retval[i - 1] = (Out_T)(in_value & mask_lowest_bits<In_T, Out_T>::value);

	return retval;
}

// Useful shortcuts
//template <typename T>
//[[nodiscard]] inline constexpr auto FromLittleEndian(T v)
//{
//	return FromLittleEndian(*(std::array<uint8_t, sizeof(T)>*)&v);
//}
//
//template <typename T>
//[[nodiscard]] inline constexpr auto ToLittleEndian(T v)
//{
//	// Since we are just swapping, this is easier that toLittleEndian above, but does the same thing.
//	return FromLittleEndian(*(std::array<uint8_t, sizeof(T)>*)&v);
//}


//==================================================================================================
// Unit tests

static_assert(FromLittleEndian(std::array<uint8_t, 4> { 1, 2, 3, 4 }) == 0x04030201, "");
static_assert(FromBigEndian(std::array<uint8_t, 4> { 1, 2, 3, 4 }) == 0x01020304, "");
static_assert(ToLittleEndian(0x04030201)[0] == 1 && ToLittleEndian(0x04030201)[1] == 2 && ToLittleEndian(0x04030201)[3] == 4, "");
static_assert(ToBigEndian(0x04030201)[0] == 4 && ToBigEndian(0x04030201)[1] == 3 && ToBigEndian(0x04030201)[3] == 1, "");

//static_assert(FromLittleEndian(std::array<uint8_t, 4> { 1, 2, 3, 4 }) != 0x01020304, "");
//static_assert(FromBigEndian(std::array<uint8_t, 4> { 1, 2, 3, 4 }) != 0x04030201, "");
//static_assert(ToLittleEndian(0x01020304) != (std::array<uint8_t, 4> { 1, 2, 3, 4 }), "");
//static_assert(ToBigEndian(0x04030201) != (std::array<uint8_t, 4> { 1, 2, 3, 4 }), "");

}} // namespace IDCLib::Endian
