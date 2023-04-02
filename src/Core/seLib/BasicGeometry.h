#pragma once

/// \cond
#include <stdint.h>
#include <array>
#include <cmath>
#include <optional>
/// \endcond

#include <seLib/Vector.h>

namespace seLib {
namespace Geometry {

//==================================================================================================

template <typename T>
using XYFunction = T(*)(T x, T y) noexcept;

template <typename T, size_t Size>
using Point_t = Vector_t<T, Size>;

template <typename T>
using Point2_t = Point_t<T, 2>;

template <typename T>
using Point3_t = Point_t<T, 3>;

template <typename T>
using Point4_t = Point_t<T, 4>;


//==================================================================================================

template <typename T>
class Line2_t {
public:
	using Self_t = Line2_t;
	T slope;
	T yIntercept;

	constexpr inline T Y(T x) const noexcept {
		return (slope * x) + yIntercept;
	}

	constexpr inline Point2_t<T> Y_Point(T x) const noexcept {
		return Point2_t<T> {{ x, (slope * x) + yIntercept}};
	}

	constexpr std::optional<Self_t> Inverse() const {
		if (slope == 0)
			return std::optional<Self_t>();
		T inv_slope = (T)1 / slope;
		return Self_t { -inv_slope, -yIntercept * inv_slope };
	}

	constexpr std::optional<Self_t> Intercept(Self_t b) const noexcept {
		T d_slope = slope - b.slope;
		if (d_slope == 0)
			return std::optional<Self_t>();
		T x = (b.yIntercept - yIntercept) / d_slope;
		return Self_t { x, Y(x) };
	}

	inline constexpr T Slope() const noexcept { return slope; }
	inline constexpr T XIntercept() const noexcept { return -yIntercept / slope; }
	inline constexpr T YIntercept() const noexcept { return yIntercept; }

	static constexpr std::optional<Self_t> FromPoints(T x1, T y1, T x2, T y2) {
		T dx = x1 - x2;
		if (dx == 0)
			return std::optional<Self_t>();

		T slope = (y1 - y2) / dx;
		return Self_t { slope, y1 - (slope * x1) };
	}

	static inline constexpr std::optional<Self_t> FromPoints(Point2_t<T> const & a, Point2_t<T> const & b) {
		return FromPoints(a.X(), a.Y(), b.X(), b.Y());
	}

	static inline constexpr Self_t FromPoint(T x, T y, T slope) {
		return Self_t { slope, y - (slope * x) };
	}

	static inline constexpr Self_t FromPoint(Point2_t<T> const & p, T slope) {
		return Self_t { slope, p.Y() - (slope * p.X()) };
	}
};


//==================================================================================================

template <typename T, size_t Dimensions>
T distance(Point_t<T, Dimensions> const & a, Point_t<T, Dimensions> const & b) {
	T val = 0;
	for (size_t i = 0; i < Dimensions; i++) {
		T d = a[i] - b[i];
		val += d * d;
	}
	return std::sqrt(val);
}


///-------------------------------------------------------------------------------------------------
/// @fn	template <typename T, size_t Dimensions> constexpr T distance2(Point_t<T, Dimensions> const & a, Point_t<T, Dimensions> const & b)
/// @brief	Returns the square of the distance between two points.
/// @tparam	T		  	Generic type parameter.
/// @tparam	Dimensions	Number of dimensions.
/// @param 	a	Point A.
/// @param 	b	Point B.
/// @returns	Distance^2.
template <typename T, size_t Dimensions>
constexpr T distance2(Point_t<T, Dimensions> const & a, Point_t<T, Dimensions> const & b) {
	T val = 0;
	for (size_t i = 0; i < Dimensions; i++) {
		T d = a[i] - b[i];
		val += d * d;
	}
	return val;
}

//==================================================================================================

}} // namespace IDCLib::Geometry
