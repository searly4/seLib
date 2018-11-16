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
#include <seLib/FixedPoint.h>

namespace seLib {

template <typename T, int bits>
class Interpolator {
protected:
	T interp_table[(1 << bits) + 1];
	
public:
	Interpolator() {}

	static constexpr size_t size() {
		return (1 << bits) + 1;
	}

	void Set(size_t index, T value) {
		assert(index < size());
		interp_table[index] = value;
	}

	T Interpolate(T input) const {
		size_t interp_proportion_bits = sizeof(T) * 8 - bits;
		size_t curve_steps = size() - 1;
		T step_proportion_mask = (T)(1u << bits) - 1;
		T interp_proportion_mask = (T)(1u << interp_proportion_bits) - 1;

		size_t curve_step_n = (input >> interp_proportion_bits) & step_proportion_mask; // which interval in the curve table to use
		T interp_proportion = input & interp_proportion_mask;
		T interp_low = interp_table[curve_step_n];
		T interp_high = interp_table[curve_step_n + 1];
		T step_width = interp_high - interp_low;
		T output = interp_low + (((int)step_width * interp_proportion) >> interp_proportion_bits);

		return output;
	}

	T const * Data() const {
		return interp_table;
	}
};

template <typename T, int bits, typename Storage_T = typename T::Storage_t>
class FixedPoint_Interpolator : protected Interpolator<Storage_T, bits> {
public:
	typedef Interpolator<Storage_T, bits> Base_t;
	static_assert(sizeof(T) == sizeof(Storage_T));

	T Interpolate(T input) const {
		Storage_T raw_val = input.GetRaw();
		int lshift = sizeof(Storage_T) * 8 - T::Radix();
		raw_val = (raw_val << lshift) | (raw_val >> T::Radix());
		return T::fromRaw(Base_t::Interpolate(raw_val));
		//return T::fromRaw(Base_t::Interpolate(input.GetRaw()));
	}

	T const * Data() const {
		return (T*)(void*)Base_t::Data();
	}

	static constexpr size_t size() {
		return Base_t::size();
	}
};

}
