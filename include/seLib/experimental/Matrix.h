#pragma once
// Copyright (c) 2017 Scott Early
/*
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <initializer_list>
#include <cstdlib>
#include <type_traits>
//#include <exception>
#include <seLib/Vector.h>
#include <seLib/BasicGeometry.h>

namespace seLib {

//using Vector_t = Point_t;
//template <typename T, size_t Size>
//using Vector_t = Point_t<T, Size>;

namespace Matrix {

using namespace seLib::Geometry;

template <typename T, typename Base_T>
class MatrixHandler_t;

struct SizeMismatchException_t : std::exception {};
struct UnsupportedSizeException_t : std::exception {};


template <typename T>
struct t { using type = T; };

template <typename Matrix_T>
inline constexpr auto Identity();


template <typename T, size_t Cols_N, size_t Rows_N, typename Row_T = Vector_t<T, Cols_N>, typename Base_T = Vector_t<Row_T, Rows_N>>
class VectorMatrixAccessor_t : public Base_T {
public:
	using Scaler_t = T;
	using Row_t = Vector_t<T, Cols_N>;
	using Matrix_t = VectorMatrixAccessor_t;
	constexpr static size_t rows() { return Rows_N; }
	constexpr static size_t cols() { return Cols_N; }
	constexpr static VectorMatrixAccessor_t blank(size_t rows) { return {}; }

};


template <typename T, size_t Cols_N, size_t Rows_N, typename Row_T = Vector_t<T, Cols_N>, typename Base_T = Vector_t<Row_T, Rows_N>>
class VectorMatrixWrapper_t {
public:
	using Scaler_t = T;
	using Row_t = Row_T;
	using Matrix_t = Base_T;
	constexpr static size_t rows() { return Rows_N; }
	constexpr static size_t cols() { return Cols_N; }
	constexpr static size_t size() { return Rows_N; }
	constexpr static auto blank(size_t rows) { return VectorMatrixAccessor_t<T, Cols_N, Rows_N, Row_T>::blank(rows); }

	Base_T& ref;

	inline constexpr auto operator[](size_t i) const { return ref[i]; }
};





template <typename Matrix_T,
	typename T = typename Matrix_T::Scaler_t,
	typename Return_T = Vector_t<T, Matrix_T::cols()>
	>
inline constexpr auto operator*(typename Matrix_T::Row_t const & v, MatrixHandler_t<T, Matrix_T> const & m) {
	Return_T ret;

	if (v.size() != m.rows() || ret.size() != m.cols())
#if defined(__cpp_exceptions) && __cpp_exceptions==199711
		throw SizeMismatchException_t{};
#else
		abort();//return Identity<Return_T>();
#endif

    for (size_t i1 = 0; i1 < ret.size(); i1++) {
        T f = 0;
        for (size_t i2 = 0; i2 < m.cols(); i2++) {
            f += m[i2][i1] * v[i2];
        }
        ret[i1] = f;
    }
	return ret;
}




template <typename T, typename Base_T>
class MatrixHandler_t : public Base_T {
public:
	using Matrix_t = MatrixHandler_t<T, VectorMatrixAccessor_t<T, Base_T::cols(), Base_T::rows()>>;
	using Scaler_t = T;

	template <size_t Cols_N, size_t Rows_N>
	using MatrixN_t = MatrixHandler_t<T, VectorMatrixAccessor_t<T, Cols_N, Rows_N>>;

	auto& XY(size_t col_x, size_t row_y) {
		return (*this)[row_y][col_x];
	}

	auto& XY(size_t col_x, size_t row_y) const {
		return (*this)[row_y][col_x];
	}

	//template <typename Arg_T>
    //Matrix_t operator* (Arg_T const & data) const {
    //    return operator*(*this, data);
    //}

	inline constexpr auto operator*(Scaler_t value) const {
		MatrixN_t<MatrixHandler_t::cols(), MatrixHandler_t::rows()> ret;

		for (size_t row = 0; row < ret.rows(); row++) {
			for (size_t col = 0; col < MatrixHandler_t::cols(); col++) {
				ret[row][col] = (*this)[row][col] * value;
			}
		}
		return ret;
	}

	template <typename T2, typename MatrixB_T>
	inline constexpr auto operator*(MatrixHandler_t<T2, MatrixB_T> const & b) const {
		MatrixN_t<MatrixB_T::cols(), MatrixHandler_t::rows()> ret;

		if (MatrixHandler_t::cols() != b.rows() || ret.size() != MatrixHandler_t::rows())
			#if defined(__cpp_exceptions) && __cpp_exceptions==199711
				throw SizeMismatchException_t{};
			#else
				abort();//return Identity<Return_T>();
			#endif

		for (size_t i1 = 0; i1 < ret.size(); i1++) {
			ret[i1] = (*this)[i1] * b;
		}
		return ret;
	}
};


//template <typename T, typename Out_T, typename In_T, size_t Cols_N, size_t Rows_N>
//constexpr Out_T Transpose(In_T const & in) {
//	Out_T retval;
//	for (size_t row_i = 0; row_i < Rows_N; row_i++) {
//		auto& row = in[row_i];
//		for (size_t col_i = 0; col_i < Cols_N; col_i++)
//			retval[col_i][row_i] = row[col_i];
//	}
//	return retval;
//}
//
//
//template <typename T, size_t Cols_N, size_t Rows_N>
//constexpr Vector_t<Vector_t<T, Rows_N>, Cols_N> Transpose(Vector_t<Vector_t<T, Cols_N>, Rows_N> const & in) {
//	return Transpose<T, Vector_t<Vector_t<T, Rows_N>, Cols_N>, Transpose<Vector_t<T, Cols_N>, Rows_N>, Cols_N, Rows_N>;
//}


template <typename Matrix_T>
inline constexpr auto Identity() {
	Matrix_T val;
	for (size_t y = 0; y < Matrix_T::cols(); y++) {
		for (size_t x = 0; x < Matrix_T::cols(); x++)
			val[y][x] = 0;
		val[y][y] = 1;
	}

	return val;
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_T>
inline constexpr auto Scaler(T scale) {
	Matrix_T val;
	for (size_t y = 0; y < Matrix_T::cols(); y++) {
		for (size_t x = 0; x < Matrix_T::cols(); x++)
			val[y][x] = 0;
		val[y][y] = scale;
	}
	val[Matrix_T::cols() - 1][Matrix_T::cols() - 1] = 1;
	return val;
}

template <typename Matrix_T, typename Vector_T = typename Matrix_T::Row_t>
inline constexpr auto Translator(Vector_T offset) {
	auto val = Identity<Matrix_T>();
	auto const count = std::min<size_t>(offset.size(), Matrix_T::cols() - 1);
	for (size_t i = 0; i < count; i++)
		//val[i][Matrix_T::cols() - 1] = offset[i];
		val[Matrix_T::rows() - 1][i] = offset[i];
	return val;
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_T>
inline auto RotationX(T angle) {
    auto tm = Identity<Matrix_T>();
    tm[2][2] = (tm[1][1] = cos(angle));
    tm[1][2] = -(tm[2][1] = sin(angle));
    return tm;
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_T>
inline auto RotationY(T angle) {
    auto tm = Identity<Matrix_T>();
    tm[2][2] = (tm[0][0] = cos(angle));
    tm[2][0] = -(tm[0][2] = sin(angle));
    return tm;
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_T>
inline auto RotationZ(T angle) {
    auto tm = Identity<Matrix_T>();
    tm[1][1] = (tm[0][0] = cos(angle));
    tm[0][1] = -(tm[1][0] = sin(angle));
    return tm;
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_t, typename T2 = t<T>>
inline auto Perspective(typename T2::type fov, typename T2::type aspect, typename T2::type plane_near, typename T2::type plane_far) {
	T D2R = (T)M_PI / (T)180.0;
	T yScale = (T)1.0 / tan(D2R * fov / 2);
	T xScale = (T)yScale / aspect;
	T zf = -plane_far / (plane_far - plane_near);

	return Matrix_T({
		Vector_t<T, Matrix_T::rows()> { xScale, 0, 0, 0 },
		Vector_t<T, Matrix_T::rows()> { 0, yScale, 0, 0 },
		Vector_t<T, Matrix_T::rows()> { 0, 0, zf, -1 },
		Vector_t<T, Matrix_T::rows()> { 0, 0, zf*plane_near, 1 }
	});
}

template <typename Matrix_T,
	typename T = typename Matrix_T::Scaler_t,
	typename Return_T = MatrixHandler_t<T, VectorMatrixAccessor_t<T, Matrix_T::rows(), Matrix_T::cols()>>
	>
inline constexpr auto Transpose(Matrix_T const & m) {
    Return_T tm;
	for (size_t row_i = 0; row_i < m.rows(); row_i++) {
		for (size_t col_i = 0; col_i < m.cols(); col_i++)
			tm[col_i][row_i] = m[row_i][col_i];
	}
    return tm;
}

template <typename Matrix_T>
inline constexpr auto Determinant(Matrix_T const & m) {
	// TODO: generalize

	if (m.rows() != m.cols())
		#if defined(__cpp_exceptions) && __cpp_exceptions==199711
			throw UnsupportedSizeException_t{};
		#else
			abort();//return Identity<Return_T>();
		#endif

	switch (m.cols()) {
	case 2:
		return m[0][0] * m[1][1] - m[1][0] * m[0][1];

	case 3:
		return
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
			+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	case 4:
		return 
			(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
			- (m[0][2] * m[1][3] - m[1][2] * m[0][3]) * (m[2][0] * m[3][1] - m[3][0] * m[2][1]);
		
	default:
		#if defined(__cpp_exceptions) && __cpp_exceptions==199711
			throw UnsupportedSizeException_t{};
		#else
			abort();//return Identity<Return_T>();
		#endif
	}
}

template <typename Matrix_T, typename T = typename Matrix_T::Scaler_t>
inline constexpr auto Inverse(Matrix_T const & m) {
    return Transpose(m) * (T)((T)1 / Determinant(m));
}


} // namespace seLib::Matrix

template <typename T, typename Base_T = Vector_t<T, 4>>
using Quaternion_t = Base_T;

template <typename T, size_t Size, typename Base_T = Matrix::VectorMatrixAccessor_t<T, Size, Size>>
using SquareMatrix_t = Matrix::MatrixHandler_t<T, Base_T>;

template <typename T, size_t Cols_N, size_t Rows_N, typename Base_T = Matrix::VectorMatrixAccessor_t<T, Cols_N, Rows_N>>
using Matrix_t = Matrix::MatrixHandler_t<T, Base_T>;

using Vector2f_t = Vector_t<float, 2>;
using Vector2d_t = Vector_t<double, 2>;
using Vector3f_t = Vector_t<float, 3>;
using Vector3d_t = Vector_t<double, 3>;
using Vector4f_t = Vector_t<float, 4>;
using Vector4d_t = Vector_t<double, 4>;
//template Vector<double, 3>::operator Vector<float, 3><float, 3>()

using Matrix4f_t = Matrix_t<float, 4, 4>;
using Matrix4d_t = Matrix_t<double, 4, 4>;

} // namespace seLib
