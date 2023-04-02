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

namespace seLib {
namespace Embedded {

static const uint8_t _t1 __attribute__((used)) = ~(uint8_t)0;
static const uint8_t _t2 __attribute__((used)) = (~(uint8_t)0) >> 1;
static const uint8_t _t2b __attribute__((used)) = ((uint16_t)(uint8_t)~(uint8_t)0) >> 6;
static const uint8_t _t3 __attribute__((used)) = ~((~(uint8_t)0) >> 1);
static const uint8_t _t4 __attribute__((used)) = ~((uint16_t)((uint8_t)~(uint8_t)0) >> 1);

bool lastshit __attribute__((used));
template <typename storage_t>
class Debounce1 {
public:
	static const storage_t Mask = ~(((storage_t)~(storage_t)0) >> 1);
	storage_t History;

	Debounce1(bool initialstate = false) : History(!initialstate ? (storage_t)0 : ~(storage_t)0) {}

	bool Update(bool currentstate) {
		lastshit = currentstate;
		storage_t iMask = (storage_t)~Mask;
		storage_t acc = (History << 1) & iMask;
		acc |= currentstate ? 1 : 0;
		if (acc == iMask) {
			acc |= Mask;
		} else if (acc != 0) {
			acc |= (History & Mask);
		}
		History = acc;

		return State();
	}

	inline bool State() {
		return (History & Mask) != 0;
	}
};

template <typename timestamp_t>
class Debounce2 {
public:
	timestamp_t LowStart;
	timestamp_t HighStart;
	timestamp_t LowDuration = 0;
	timestamp_t HighDuration = 0;
	timestamp_t ChangeDuration = 0;
	bool State = false;

	Debounce2(timestamp_t changeduration) : ChangeDuration(changeduration) {}

	void Reset(bool initialstate, timestamp_t initialtime) {
		State = initialstate;
		LowDuration = 0;
		HighDuration = 0;
		LowStart = !initialstate ? initialtime : (initialtime - 1);
		HighStart = initialstate ? initialtime : (initialtime - 1);
	}

	bool Update(bool currentstate, timestamp_t time) {
		if (currentstate)
			HighStart = time;
		else
			LowStart = time;


		return State;
	}
};

}
}
