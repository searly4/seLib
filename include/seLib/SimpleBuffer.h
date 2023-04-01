#pragma once
///-------------------------------------------------------------------------------------------------
/// @file	Source/IDCLib/Util/SimpleBuffer.h
/// @brief	Minimal buffer pointer class.

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <exception>
#include <assert.h>
#include <array>
/// \endcond


//==================================================================================================
namespace seLib { namespace Buffers {


///-------------------------------------------------------------------------------------------------
/// @struct	SimpleBuffer_t
/// @brief	A simple pointer with size information to an external array.
/// @tparam	T	Type of the buffer data.
template <typename T>
struct SimpleBuffer_t {
	T* Data;
	size_t Length;
	
	constexpr SimpleBuffer_t()
		: Data(nullptr)
		, Length(0) {}

	constexpr SimpleBuffer_t(T& data, size_t size)
		: Data(&data)
		, Length(size)
	{}

	constexpr SimpleBuffer_t(T* data, size_t size)
		: Data(data)
		, Length(size)
	{}

	constexpr SimpleBuffer_t(T* start, T* end)
		: Data(start)
		, Length(end - start)
	{}

	template <size_t Count>
	constexpr SimpleBuffer_t(std::array<T, Count> & data)
		: Data(data.data())
		, Length(Count)
	{}

	template <size_t Count>
	constexpr SimpleBuffer_t(T(&str)[Count])
		: Data(str)
		, Length(Count)
	{}

	template <typename T2>
	constexpr SimpleBuffer_t(SimpleBuffer_t<T2> const & b)
		: Data((T*)b.Data)
		, Length(b.Length)
	{
		static_assert(sizeof(T) == sizeof(T2));
	}
		
	SimpleBuffer_t(SimpleBuffer_t const &) = default;
	SimpleBuffer_t& operator=(SimpleBuffer_t const &) = default;
	
	
	constexpr bool operator==(SimpleBuffer_t<T> const & b) const noexcept {
		return Data == b.Data && Length == b.Length;
	}

	
	constexpr bool operator!=(SimpleBuffer_t<T> const & b) const noexcept {
		return Data == b.Data && Length == b.Length;
	}
	
	
	T& operator[](size_t index) const noexcept {
		return Data[index];
	}
	
	
	constexpr size_t size() const noexcept {
		return Length;
	}
	
	
	constexpr T* data() const noexcept {
		return Data;
	}
	
	constexpr T* begin() const noexcept {
		return Data;
	}

	constexpr T* end() const noexcept {
		return Data + Length;
	}
	
	constexpr SimpleBuffer_t<T> Subset(size_t start) const noexcept {
		if (start >= Length)
			return SimpleBuffer_t<T>(Data + Length, 0u);
		return SimpleBuffer_t<T>(Data + start, Length - start);
	}

	
	constexpr SimpleBuffer_t<T> Subset(size_t start, int length) const noexcept {
		if (start >= Length)
			return SimpleBuffer_t<T>(Data + Length, 0u);

		if (length < 0) {
			length = std::max<int>(0, (int)Length - (int)start + length);
		} else {
			length = std::min<int>(length, (int)Length - (int)start);
		}
		
		return SimpleBuffer_t<T>(Data + start, (size_t)length);
	}
	
	
	constexpr int Compare(SimpleBuffer_t<T> const & b) const {
		T* a_pos = Data;
		T* b_pos = b.Data;
		T* const a_end = Data + Length;
		T* const b_end = b.Data + b.Length;

		while (a_pos < a_end && b_pos < b_end) {
			if (*a_pos < *b_pos)
				return -1;
			if (*a_pos > *b_pos)
				return 1;
			a_pos++;
			b_pos++;
		}
		
		return (Length < b.Length) ? -1 : (Length > b.Length) ? 1 : 0;
	}
};


}} // namespace IDCLib::Buffers
//==================================================================================================
