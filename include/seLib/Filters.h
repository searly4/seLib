#pragma once

/// \cond
#include <assert.h>
/// \endcond

// C++ only below here
#ifdef   __cplusplus

/// \cond
#include <algorithm>
/// \endcond


//==================================================================================================
namespace seLib { namespace Filters {

struct RangeException : std::exception {};

/// @brief A simple low-pass filter.
/// @tparam T Generic type parameter.
template <typename T>
class Filter_LP_t {
public:
	T Fraction;

	/// @brief Constructor
	/// @param fraction The fraction of a new value that is applied to the previous value.
	constexpr Filter_LP_t(T fraction) : Fraction(fraction) {
		if (fraction < 0 || fraction > 1)
			throw RangeException();
	}

	constexpr Filter_LP_t(Filter_LP_t const &) = default;

	inline constexpr T operator()(T newval, T oldval) const {
		return newval * Fraction + oldval * ((T)1 - Fraction);
	}
};


}} // namespace IDCLib::Filters
//==================================================================================================


#endif // __cplusplus
