#pragma once
/*
   Copyright 2018 by Scott Early

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


#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <array>
#include <complex>

#include <seLib/FixedPoint.h>

namespace seLib {
namespace Filtering {

using namespace std;
using namespace seLib;

template <int DCT_SIZE, typename T = float>
class DCT {
public:
	typedef complex<T> Complex_T;

	void Process(const vector<T>& input, vector<T>& output) const {
		vector<Complex_T> input_c(size());
		vector<Complex_T> output_c(size());
		for (size_t i = 0; i < size(); i++)
			input_c[i] = input[i];
		Process(input_c.data(), output_c.data());
		output.resize(size() >> 1);
		for (size_t i = 0; i < output.size(); i++)
			output[i] = sqrt(norm(output_c[i]));
	}

	void Process(typename vector<T>::const_iterator input_start, typename vector<T>::const_iterator input_end, vector<T>& output) const {
		vector<Complex_T> output_c(size());
		vector<Complex_T> input_c(size());

		auto iter = input_start;
		for (size_t i = 0; i < size(); i++) {
			assert(iter != input_end);
			input_c[i] = *iter;
			iter++;
		}
		Process(input_c, output_c);
		output.resize(size() >> 1);
		for (size_t i = 0; i < output.size(); i++)
			output[i] = sqrt(norm(output_c[i]));
	}

	void Process(const vector<Complex_T>& input, vector<Complex_T>& output) const {
		auto halfsize = size() >> 1;

		/*
		for (size_t i = 0; i < halfsize; i++)
			output[size() - i] = output[i] = input[i];

		for (size_t i = 1; i < halfsize; i++)
			swap(buffer[i], buffer[BitReverse(i)]);
		*/

		for (size_t i = 0; i < size(); i++)
			output[BitReverse(i)] = input[i];

		for (int N = 2; N <= size(); N <<= 1) {
			for (int i = 0; i < size(); i += N) {
				for (int k = 0; k < N / 2; k++) {

					int evenIndex = i + k;
					int oddIndex = i + k + (N / 2);
					auto even = output[evenIndex];
					auto odd = output[oddIndex];

					double term = -2.0 * M_PI * k / (double)N;
					Complex_T exp;
					exp.real((float)cos(term));
					exp.imag((float)sin(term));
					exp *= odd;

					output[evenIndex] = even + exp;
					output[oddIndex] = even - exp;

				}
			}
		}
	}

	template <typename F_T>
	inline static constexpr void swap(F_T& a, F_T& b) {
		F_T temp = a;
		a = b;
		b = temp;
	}

	static constexpr size_t size() {
		size_t val = DCT_SIZE;
		return (size_t)1 << (DCT_SIZE);
	}

	static constexpr size_t BitReverse(size_t in) {
		size_t out = 0;
		for (size_t i = 0; i < DCT_SIZE; i++) {
			out <<= 1;
			out |= in & 1;
			in >>= 1;
		}
		return out;
	}
};

}
}

