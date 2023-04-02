#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
//#include <atomic>
#include <array>
/// \endcond

#include "Iterators.h"

//==================================================================================================
namespace seLib {

template <typename Value_T>
class ArrayAccessor_t {
public:
	Value_T * Start;
	size_t Count;

	using iterator_t = seLib::Iterators::pointer_iterator_t<Value_T>;

	template <size_t N>
	ArrayAccessor_t(std::array<Value_T, N> & ref)
		: Start(ref.data()), Count(N)
	{}

	template <size_t N>
	ArrayAccessor_t(std::array<Value_T, N> const & ref)
		: Start(ref.data()), Count(N)
	{}

	iterator_t begin() const noexcept { return iterator_t(Start); }
	iterator_t end() const noexcept { return iterator_t(Start + Count); }
	constexpr size_t size() const noexcept { return Count; }
};

template <typename Value_T, size_t N>
ArrayAccessor_t(std::array<Value_T, N> const &) -> ArrayAccessor_t<Value_T const>;

}