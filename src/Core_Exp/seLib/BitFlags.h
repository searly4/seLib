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

#include <stdint.h>
#include <initializer_list>

#define SELIB_FLAGSENUM_OPERATORS(ENUM_TYPE, STORAGE_TYPE) \
	inline STORAGE_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b) { return (STORAGE_TYPE)a | (STORAGE_TYPE)b; } \
	inline STORAGE_TYPE operator|(STORAGE_TYPE a, ENUM_TYPE b) { return (STORAGE_TYPE)a | (STORAGE_TYPE)b; } \
	inline STORAGE_TYPE operator|(ENUM_TYPE a, STORAGE_TYPE b) { return (STORAGE_TYPE)a | (STORAGE_TYPE)b; }

namespace seLib {

template <typename Flags_T, typename Storage_T = uint32_t>
class BitFlags {
protected:
	Storage_T Bitmap;

	static inline constexpr Storage_T MaskOf(Flags_T flag) {
		return ((Storage_T)1) << (Storage_T)flag;
	}

	constexpr BitFlags(Storage_T init_vals) : Bitmap(init_vals) {}

public:
	typedef BitFlags<Flags_T, Storage_T> Self_T;
	typedef Flags_T Flags;
	constexpr BitFlags() : Bitmap(0) {}

	constexpr BitFlags(std::initializer_list<Flags_T> set_list) : Bitmap(0) {
		auto iter = set_list.begin();
		auto iter_end = set_list.end();
		while (iter != iter_end) {
			Bitmap |= MaskOf(*iter);
			iter++;
		}
	}

	// Return the flags bitmap.
	inline Storage_T Get() const {
		return Bitmap;
	}

	// Return true if the specified flag is set.
	inline constexpr bool Get(Flags_T flag) const {
		Storage_T flagmask = MaskOf(flag);
		return (Bitmap & flagmask) != 0;
	}

	// Return true if the specified flag is set.
	inline constexpr bool operator[](Flags_T flag) const {
		return Get(flag);
	}

	// Set the specified flag and return its previous value.
	inline bool Set(Flags_T flag) {
		Storage_T flagmask = MaskOf(flag);
		bool lastval = (Bitmap & flagmask) != 0;
		Bitmap |= flagmask;
		return lastval;
	}

	// Clear the specified flag and return its previous value.
	inline bool Clear(Flags_T flag) {
		Storage_T flagmask = MaskOf(flag);
		bool lastval = (Bitmap & flagmask) != 0;
		Bitmap &= ~flagmask;
		return lastval;
	}

	// Assign a value to the specified flag and return its previous value.
	inline bool Assign(Flags_T flag, bool value) {
		return value ? Set(flag) : Clear(flag);
	}

	inline bool operator==(const BitFlags<Flags_T, Storage_T>& b) const {
		return Bitmap == b.Bitmap;
	}

	Self_T ChangedFlags(const Self_T& ref) const {
		return Self_T(Bitmap ^ ref.Bitmap);
	}

	inline bool FallingEdge(Flags_T flag, const Self_T& ref) const {
		return Get(flag) && !ref.Get(flag);
	}

	inline bool RisingEdge(Flags_T flag, const Self_T& ref) const {
		return !Get(flag) && ref.Get(flag);
	}

	inline bool Update(Flags_T flag, const Self_T& ref) {
		bool old_val = Get(flag);
		Assign(flag, ref.Get(flag));
		return old_val;
	}
	
	inline Self_T& operator=(Self_T const & b) {
		Bitmap = b.Bitmap;
		return *this;
	}
	
	inline constexpr Self_T operator&(Self_T const & b) const {
		return { Bitmap & b.Bitmap };
	}
	
	inline constexpr Self_T operator|(Self_T const & b) const {
		return { Bitmap | b.Bitmap };
	}
	
	inline constexpr Self_T operator^(Self_T const & b) const {
		return { Bitmap ^ b.Bitmap };
	}

	inline constexpr Self_T operator~() const {
		return { ~Bitmap };
	}
};


}
