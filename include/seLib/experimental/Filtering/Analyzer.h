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
#include <string>
#include <string_view>

#include <seLib/RefObj.h>
#include <seLib/experimental/DataSet.h>
#include <seLib/experimental/fft5.h>
#include <seLib/experimental/DCT.h>

namespace seLib {
namespace Filtering {

using namespace std;
using namespace seLib;

/*double calc_mean(vector<float>::const_iterator start, vector<float>::const_iterator end);
double calc_stddev(vector<float>::const_iterator start, vector<float>::const_iterator end);
double calc_peak_magnitude(vector<float>::const_iterator start, vector<float>::const_iterator end);
size_t find_idle(float maxdev, size_t window, const vector<float>& data);*/

class Analyzer {
public:
	TypedRefBufferView<DataSet> LastDataSet = TypedRefBufferView<DataSet>::from_RefBuffer(nullptr);

	virtual TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) = 0;
	virtual TypedRefBufferView<DataSet> Process2(const vector<float>& data) {
		return Process(data.begin(), data.end());
	}

	virtual string Name() = 0;
};

template <typename T>
class SimpleAnalyzer : public Analyzer {
protected:
	string _Name;
public:
	typedef ScalarDataSet<T> ReturnValue_t;

	SimpleAnalyzer(string_view name) : _Name(name) {}

	string Name() override {
		return _Name;
	}
};

class MeanAnalyzer : public SimpleAnalyzer<float> {
public:
	MeanAnalyzer() : SimpleAnalyzer<float>("Mean") {}
	MeanAnalyzer(string_view name) : SimpleAnalyzer<float>(name) {}
	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
};

class StdDevAnalyzer : public SimpleAnalyzer<float> {
public:
	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
	StdDevAnalyzer() : SimpleAnalyzer<float>("StdDev") {}
	StdDevAnalyzer(string name) : SimpleAnalyzer<float>(name) {}
};

class PeakMagAnalyzer : public SimpleAnalyzer<float> {
public:
	PeakMagAnalyzer() : SimpleAnalyzer<float>("PeakMag") {}
	PeakMagAnalyzer(string name) : SimpleAnalyzer<float>(name) {}
	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
};

class IdleAnalyzer : public SimpleAnalyzer<size_t> {
public:
	size_t window = 100;
	float maxdev = 5;

	IdleAnalyzer() : SimpleAnalyzer<size_t>("FindIdle") {}
	IdleAnalyzer(string name) : SimpleAnalyzer<size_t>(name) {}
	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
};

class HitAnalyzer : public SimpleAnalyzer<size_t> {
public:
	typedef ScalarDataSet<size_t> ReturnValue_t;

	float Magnitude;

	HitAnalyzer(float magnitude) : Magnitude(magnitude), SimpleAnalyzer<size_t>("FindHit") {}
	HitAnalyzer(float magnitude, string name) : Magnitude(magnitude), SimpleAnalyzer<size_t>(name) {}
	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
};

class FreqHitAnalyzer : public SimpleAnalyzer<size_t> {
public:
	const size_t TapCount;
	float Magnitude;

	FreqHitAnalyzer(size_t tapcount, float magnitude) : TapCount(tapcount), Magnitude(magnitude), SimpleAnalyzer<size_t>("FindHit") {}
	FreqHitAnalyzer(size_t tapcount, float magnitude, string_view name) : TapCount(tapcount), Magnitude(magnitude), SimpleAnalyzer<size_t>("FindHit") {}

	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;
};

class FFTAnalyzer : public Analyzer {
protected:
	string _Name;
	FFT5 fft;

public:
	typedef ArrayDataSet<float> ReturnValue_t;

	const size_t TapCount;

	FFTAnalyzer(size_t tapcount) : TapCount(tapcount), fft(tapcount), _Name("FFT") {}
	FFTAnalyzer(size_t tapcount, string_view name) : TapCount(tapcount), fft(tapcount), _Name(name) {}

	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override;

	string Name() override {
		return _Name;
	}
};

template <int DCT_SIZE>
class DCTAnalyzer : public Analyzer {
protected:
	string _Name;
	DCT<DCT_SIZE, float> dct;

public:
	typedef ArrayDataSet<float> ReturnValue_t;

	DCTAnalyzer() : _Name("DCT") {}
	DCTAnalyzer(string_view name) : _Name(name) {}

	TypedRefBufferView<DataSet> Process(vector<float>::const_iterator start, vector<float>::const_iterator end) override {
		TypedRefBufferView<ReturnValue_t> retval(Name());
		auto& output_array = retval->Data;
		output_array.resize(dct.size());

		dct.Process(start, end, output_array);

		LastDataSet = retval.cast<DataSet>();
		return LastDataSet;
	}


	string Name() override {
		return _Name;
	}
};

}
}
