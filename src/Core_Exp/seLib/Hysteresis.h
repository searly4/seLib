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

namespace seLib {

template <typename T>
class Hysteresis {
protected:
	T _MinChange;
	T _LastVal;
	
public:
	Hysteresis(T min_change, T init_val) : _MinChange(min_change), _LastVal(init_val) {}

	Hysteresis(T min_change) : _MinChange(min_change) {}

	T Set(T input) {
		if (input > (_LastVal + _MinChange))
			return _LastVal = input - _MinChange;
		if (input < (_LastVal - _MinChange))
			return _LastVal = input + _MinChange;
		return _LastVal;
	}

	inline T SetBypass(T input) {
		return _LastVal = input;
	}

	inline T Get() const {
		return _LastVal;
	}
};

template <typename T, T _MinValue, T _MaxValue>
class StateHysteresis2 {
protected:
	bool _LastState;
	
public:
	StateHysteresis2(T init_val) : _LastState(init_val) {}

	bool Set(T input) {
		if (input > _MaxValue)
			return _LastState = true;
		if (input < _MinValue)
			return _LastState = false;
		return _LastState;
	}

	inline bool SetBypass(bool state) {
		return _LastState = state;
	}

	inline bool Get() const {
		return _LastState;
	}
};

template <typename T>
class StateHysteresis {
protected:
	bool _LastState;
	
public:
	T MinValue;
	T MaxValue;

	StateHysteresis(T min_val, T max_val, bool init_val) : MinValue(min_val), MaxValue(max_val), _LastState(init_val) {}

	bool Set(T input) {
		if (input > MaxValue)
			return _LastState = true;
		if (input < MinValue)
			return _LastState = false;
		return _LastState;
	}

	inline bool SetBypass(bool state) {
		return _LastState = state;
	}

	inline bool Get() const {
		return _LastState;
	}
};

}
