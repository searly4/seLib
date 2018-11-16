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
#include <functional>

namespace seLib {

using namespace std;

template <typename Counter_T, typename Callback_T, int count>
class Timer_Comparator {
public:
	typedef Timer_Comparator<Counter_T, Callback_T, count> Self_T;
	array<Counter_T, count> Comparators;
	array<Callback_T, count> Callbacks;

	constexpr Timer_Comparator(const array<Counter_T, count>& comparators, const array<Callback_T, count>& callbacks) :
		Comparators(comparators), Callbacks(callbacks)
	{}
	constexpr Timer_Comparator(const Self_T&) = default;
	constexpr Timer_Comparator(Self_T&&) = default;

	void Compare(Counter_T value) const {
		for (size_t i = 0; i < count; i++) {
			if (value == Comparators[i])
				Callbacks[i]();
		}
	}
};

template <typename T>
class Counter {
protected:
	T _Counter;

public:
	typedef Counter<T> Self_T;

	Counter() : _Counter(0) {}
	
	inline Counter& operator++() {
		_Counter++;
		return *this;
	}

	inline Counter& operator++(int) {
		_Counter++;
		return *this;
	}
	
	inline T Value() const {
		return _Counter;
	}
	
	inline void WaitTick() const {
		T last_counter = _Counter;
		while (last_counter == *((volatile T *)&_Counter)) {}
	}
};

template <typename T, T max>
class MaxCounter : public Counter<T> {
public:
	typedef MaxCounter<T, max> Self_T;

	inline MaxCounter& operator++(int) {
		this->_Counter++;
		if (this->_Counter >= max)
			this->_Counter = 0;
		return *this;
	}
};

template <typename T, int maskbits>
class MaskCounter : public Counter<T> {
private:
	static const T _Mask = (T)~((T)((T)~(T)0) << maskbits);
public:
	typedef MaskCounter<T, maskbits> Self_T;

	MaskCounter() : Counter<T>() {}
	
	inline MaskCounter& operator++(int) {
		this->_Counter = (this->_Counter + (T)1) & _Mask;
		return *this;
	}

	inline void WaitTicks(size_t ticks) const {
		// todo: verify mult-wrap
		T start_counter = this->_Counter;
		T last_counter = start_counter;
		T counter_match = (ticks + last_counter) & _Mask;
		size_t wrap = _Mask;
		size_t wrap_counter = 0;
		size_t wrap_counter_match = ((ticks + last_counter) <= wrap) ? 0 : ((ticks + last_counter) >> maskbits);
		while (true) {
			T current_counter = *((volatile T *)&(this->_Counter));
			if (last_counter > current_counter)
				wrap_counter++;
			if (wrap_counter >= wrap_counter_match && current_counter >= counter_match)
				break;
			last_counter = current_counter;
		}
	}
};

template <typename Counter_T, typename Comparator_T>
class Timer {
private:
	Counter_T * _Counter;
	Comparator_T _Comp;

public:
	typedef Timer<Counter_T, Comparator_T> Self_T;

	constexpr Timer(Counter_T * counter, const Comparator_T& comparator) : _Counter(counter), _Comp(comparator) {}

	inline const Self_T& operator++(int) const {
		(*_Counter)++;
		_Comp.Compare(_Counter->Value());
		return *this;
	}
};


}
