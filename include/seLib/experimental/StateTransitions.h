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

template <typename Enum_T>
class StateTransitions {
public:
	Enum_T PendingValue;
	Enum_T CurrentValue;

	constexpr StateTransitions(Enum_T initial_value) : CurrentValue(initial_value), PendingValue(initial_value) {}
	constexpr StateTransitions(Enum_T pending, Enum_T current) : CurrentValue(current), PendingValue(pending) {}

	// Return true if the specified value matches the pending value but not the current value.
	inline bool IsPending(Enum_T value) const {
		return PendingValue == value && CurrentValue != value;
	}

	// Return true if the specified value matches the current value but not the pending value.
	inline bool IsExpiring(Enum_T value) const {
		return PendingValue != value && CurrentValue == value;
	}

	// Return true if the specified value is current.
	inline bool IsCurrent(Enum_T value) const {
		return CurrentValue == value;
	}

	// Return true if the specified value is current.
	inline bool IsTransition(Enum_T from, Enum_T to) const {
		return CurrentValue == from && PendingValue == to;
	}

	// Set the specified value as pending and return the current value.
	inline Enum_T Assign(Enum_T value) {
		return PendingValue = value;
		return CurrentValue;
	}

	// Set the current value to the specified value return its previous value.
	inline Enum_T AssignImmediate(Enum_T value) {
		auto lastval = CurrentValue;
		PendingValue = CurrentValue = value;
		return lastval;
	}

	inline bool IsChangePending() const {
		return !(CurrentValue == PendingValue);
	}

	// Copy the pending value to the current value.
	inline Enum_T Update() {
		auto lastval = CurrentValue;
		CurrentValue = PendingValue;
		return lastval;
	}

	inline StateTransitions<Enum_T>& operator=(Enum_T value) {
		PendingValue = value;
		return *this;
	}

	inline bool operator==(Enum_T value) const {
		return CurrentValue == value;
	}

	inline bool operator!=(Enum_T value) const {
		return CurrentValue != value;
	}
};


}
