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

#include "BitFlags.h"
#include "seLib_Util.h"
#include <functional>

namespace seLib {

template <typename Flags_T, typename Storage_T = uint32_t>
class BitTransitions {
public:
	typedef Flags_T Flags;
	typedef BitFlags<Flags_T, Storage_T> BitFlags_T;

	BitFlags_T PendingFlags;
	BitFlags_T CurrentFlags;

	constexpr BitTransitions() {}

	// Return true if the specified flag is pending but not yet current.
	inline bool IsSetPending(Flags_T flag) const {
		return PendingFlags[flag] && !CurrentFlags[flag];
	}

	// Return true if the specified flag is current but a clear is pending.
	inline bool IsClearPending(Flags_T flag) const {
		return !PendingFlags[flag] && CurrentFlags[flag];
	}

	// Return true if the specified flag is pending.
	inline bool IsPending(Flags_T flag) const {
		return PendingFlags[flag];
	}

	// Return true if the specified flag is pending.
	inline bool IsCurrent(Flags_T flag) const {
		return CurrentFlags[flag];
	}

	// Return true if the specified flag is set.
	inline bool operator[](Flags_T flag) const {
		return CurrentFlags.Get(flag);
	}

	// Set the specified flag and return its previous value.
	inline bool Set(Flags_T flag) {
		return PendingFlags.Set(flag);
	}

	// Clear the specified flag and return its previous value.
	inline bool Clear(Flags_T flag) {
		return PendingFlags.Clear(flag);
	}

	// Set the specified flag and return its previous value.
	inline bool SetImmediate(Flags_T flag) {
		PendingFlags.Set(flag);
		return CurrentFlags.Set(flag);
	}

	// Clear the specified flag and return its previous value.
	inline bool ClearImmediate(Flags_T flag) {
		PendingFlags.Clear(flag);
		return CurrentFlags.Clear(flag);
	}

	// Assign a value to the specified flag and return its previous value.
	inline bool Assign(Flags_T flag, bool value) {
		return value ? Set(flag) : Clear(flag);
	}

	// Assign a value to the specified flag and return its previous value.
	inline bool AssignImmediate(Flags_T flag, bool value) {
		PendingFlags.Assign(flag, value);
		return CurrentFlags.Assign(flag, value);
	}

	inline bool IsChangePending() const {
		return !(CurrentFlags == PendingFlags);
	}

	// Copy the pending status to the current status for the specified flag.
	inline bool Update(Flags_T flag) {
		return CurrentFlags.Assign(flag, PendingFlags[flag]);
	}

	// Copy the pending status to the current status for the masked flags.
	inline void UpdateAll(BitFlags_T mask = ~BitFlags_T()) {
		CurrentFlags = (CurrentFlags & ~mask) | (PendingFlags & mask);
	}
};

template <typename Flags_T, typename Storage_T = uint32_t>
class BitTransitionNotifier {
public:
	typedef BitTransitionNotifier<Flags_T, Storage_T> Self_T;
	typedef BitTransitions<Flags_T, Storage_T> Data_T;
	//typedef void(*BitTransitionCallback)(Self_T const & notifier, void* ref);
	typedef void(*BitTransitionCallback)(Self_T const & notifier);
	struct NotifierTarget {
		//std::function<void(Self_T const &)> callback;
		//BitTransitionCallback callback;
		//BitTransitionCallback callback;
		//void* ref;
		call_forwarder<Self_T const &> callback;
		BitFlags<Flags_T, Storage_T> mask;
	};

	Data_T const & Data;
    size_t ReceiverCount;
    NotifierTarget const * Receivers;

	constexpr BitTransitionNotifier(Data_T const & data_ref, NotifierTarget const * receivers, size_t receiver_count) :
		Data(data_ref), ReceiverCount(receiver_count), Receivers(receivers)
	{}
	
    void Notify() const {
        for (size_t i = 0; i < ReceiverCount; i++) {
            auto t = Receivers[i];
	        if ((t.mask & (Data.PendingFlags ^ Data.CurrentFlags)).Get())
				t.callback(*this, t.ref);
        }
    }
};

template <typename Struct_T>
class BitFieldTransitions {
protected:
	Struct_T Pending;
	Struct_T Current;
	
public:
	//typedef Struct_T Flags;
	class Updater_t : public Struct_T {
	protected:
		BitFieldTransitions<Struct_T> const & _parent;
	public:
		Updater_t(BitFieldTransitions<Struct_T>& parent) : _parent(parent) {}
		~Updater_t() {
		}
	};

	inline Struct_T const & operator*() const {
		return Current;
	}

	inline Struct_T const & Get() {
		return Current;
	}

	inline Struct_T const & GetPending() {
		return Pending;
	}

	inline Struct_T GetChanged() {
		Struct_T retval;
		for (size_t i = 0; i < sizeof(Struct_T); i++)
			((uint8_t*)retval)[i] = ((uint8_t*)Pending)[i] ^ ((uint8_t*)Current)[i];
		return retval;
	}

	inline Struct_T GetRising() {
		Struct_T retval;
		for (size_t i = 0; i < sizeof(Struct_T); i++)
			((uint8_t*)retval)[i] = (((uint8_t*)Pending)[i] ^ ((uint8_t*)Current)[i]) & ((uint8_t*)Pending)[i];
		return retval;
	}

	inline Struct_T GetFalling() {
		Struct_T retval;
		for (size_t i = 0; i < sizeof(Struct_T); i++)
			((uint8_t*)retval)[i] = (((uint8_t*)Pending)[i] ^ ((uint8_t*)Current)[i]) & ((uint8_t*)Current)[i];
		return retval;
	}

	inline Struct_T & Set() {
		return Pending;
	}
	inline void UpdateAll() {
		Current = Pending;
		//for (size_t i = 0; i < sizeof(Struct_T); i++)
			//((uint8_t*)retval)[i] = ((uint8_t*)Pending)[i] ^ ((uint8_t*)Current)[i];
	}
	inline void UpdateMasked(Struct_T mask) {
		for (size_t i = 0; i < sizeof(Struct_T); i++)
			((uint8_t*)Current)[i] = (((uint8_t*)Pending)[i] & ((uint8_t*)mask)[i]) | (((uint8_t*)Current)[i] & ~((uint8_t*)mask)[i]);
	}
};

}
