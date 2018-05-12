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

#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

namespace seLib {
namespace Filtering {

using namespace std;
using namespace seLib;

class Filter {
protected:
public:
	virtual void Process(vector<float>& data) = 0;
	virtual void Process(const vector<float>& input, vector<float>& output) = 0;
};

class FIR_Filter : public Filter {
protected:
public:
	vector<float> m_taps;
	vector<float> m_sr;

	FIR_Filter(int tapcount);
	FIR_Filter(const FIR_Filter&) = default;
	FIR_Filter(FIR_Filter&&) = default;
	FIR_Filter& operator=(const FIR_Filter&) = default;
	FIR_Filter& operator=(FIR_Filter&&) = default;

	float Process(float input);
	void Process(vector<float>& data) override;
	void Process(const vector<float>& input, vector<float>& output) override;

	void HanningWindow();

	static FIR_Filter HighPass(int tapcount, float sample_freq, float cutoff_freq);

	static FIR_Filter LowPass(int tapcount, float sample_freq, float cutoff_freq);

	static FIR_Filter BandPass(int tapcount, float sample_freq, float lower_freq, float upper_freq);
};

class Offset_Filter : public Filter {
protected:
public:
	float Offset;
	float Scale;

	inline Offset_Filter(float offset, float scale) : Offset(offset), Scale(scale) {}
	Offset_Filter(const Offset_Filter&) = default;
	Offset_Filter(Offset_Filter&&) = default;
	Offset_Filter& operator=(const Offset_Filter&) = default;
	Offset_Filter& operator=(Offset_Filter&&) = default;

	inline float Process(float input) {
		return (input + Offset) * Scale;
	}
	void Process(vector<float>& data) override;
	void Process(const vector<float>& input, vector<float>& output) override;
};

class Window_Filter : public Filter {
protected:
public:
	vector<float> m_taps;

	Window_Filter(int tapcount);
	Window_Filter(const Window_Filter&) = default;
	Window_Filter(Window_Filter&&) = default;
	Window_Filter& operator=(const Window_Filter&) = default;
	Window_Filter& operator=(Window_Filter&&) = default;

	void Process(vector<float>& data) override;
	void Process(const vector<float>& input, vector<float>& output) override;


};

}
}
