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

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <array>
#include <math.h>

namespace seLib {

using namespace std;

template <typename T>
void CalcCurve(T* array, size_t count, T minval, T maxval, float gamma) {
	const float fraction = 1.0f / (count - 1);
	T range = maxval - minval;

	array[0] = minval;
	array[count - 1] = maxval;
	for (size_t i = 1; i < (count - 1); i++) {
		float f = pow(fraction * i, 1.0f / gamma);
		array[i] = minval + (T)(f * (float)range);
	}
}

template <typename T, int count>
constexpr array<T, count> CalcCurve(T minval, T maxval, float gamma) {
	array<T, count> retval = { 0 };
	
	const float fraction = 1.0f / (count - 1);
	T range = maxval - minval;

	retval[0] = minval;
	retval[count - 1] = maxval;
	for (size_t i = 1; i < (count - 1); i++) {
		float f = pow(fraction * i, 1.0f / gamma);
		retval[i] = minval + (T)(f * (float)range);
	}
	
	return array<T, count>(retval);
}

template <typename T, int count>
constexpr array<T, count> CalcCurve(array<T, count> minvals, array<T, count> maxvals, float minval, float maxval, float gamma) {
	array<T, count> retval;
	
	const float fraction = 1.0f / (count - 1);
	float range2 = maxval - minval;

	retval[0] = minvals[0];
	retval[count - 1] = maxvals[count - 1];
	for (size_t i = 1; i < (count - 1); i++) {
		T range = maxvals[i] - minvals[i];
		float f = pow(fraction * i, 1.0f / gamma) * range2 + minval;
		retval[i] = minvals[i] + (T)(f * (float)range);
	}
	
	return retval;
}

}