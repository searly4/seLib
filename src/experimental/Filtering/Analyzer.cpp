#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>

#include <seLib/experimental/Filtering/Analyzer.h>

namespace seLib {
namespace Filtering {

using namespace seLib;
using namespace std;

TypedRefBufferView<DataSet> MeanAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());

	double sum = 0;
	size_t count = 0;

	for (auto iter = start; iter != end; iter++) {
		sum += *iter;
		count++;
	}

	retval->Value = (float)(sum / count);

	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

TypedRefBufferView<DataSet> StdDevAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	size_t count = 0;

	double sum = 0;
	for (auto iter = start; iter != end; iter++) {
		sum += *iter;
		count++;
	}
	//double mean = calc_mean(start, end);
	float mean = (float)(sum / count);

	sum = 0;
	for (auto iter = start; iter != end; iter++) {
		double diff = *iter - mean;
		sum += diff * diff;
		count++;
	}

	retval->Value = (float)sqrt(sum / count);
	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

TypedRefBufferView<DataSet> PeakMagAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	float peak = *start;
	for (auto iter = start; iter != end; iter++) {
		peak = max(peak, abs(*iter));
	}

	retval->Value = peak;
	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

TypedRefBufferView<DataSet> IdleAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	retval->Value = 0;

	auto search_end = end - window;
	for (auto iter1 = start; iter1 != search_end; iter1++) {
		float min_val = *iter1;
		float max_val = *iter1;
		auto window_end = iter1 + (window - 1);
		for (auto iter2 = iter1 + 1; iter2 != window_end; iter2++) {
			min_val = min(min_val, *iter2);
			max_val = max(max_val, *iter2);
		}
		if ((max_val - min_val) < maxdev) {
			retval->Value = iter1 - start;
			break;
		}
	}

	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

TypedRefBufferView<DataSet> HitAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	retval->Value = 0;

	for (auto iter = start; iter != end; iter++) {
		if (abs(*iter) < Magnitude)
			continue;
		retval->Value = iter - start;
		break;
	}

	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

TypedRefBufferView<DataSet> FreqHitAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	retval->Value = 0;

	vector<float> taps(TapCount);
	for (size_t i = 0; i < TapCount; i++) {
		taps[i] = sin((float)M_PI * 2 * i / TapCount);
	}

	end -= TapCount;

	int hit_index = 0;
	for (auto window_start = start; window_start != end; window_start++) {
		float sum = 0;
		auto window_end = window_start + TapCount;
		auto iter = window_start;
		for (size_t i = 0; i < TapCount; i++, iter++) {
			sum += taps[i] * *iter;
		}
		if (abs(*iter) < Magnitude)
			continue;
		hit_index = window_start - start;
		break;
	}

	retval->Value = hit_index;
	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

//FixedPoint<long, 15>* FixedPoint<long, 15>::LastA;
//FixedPoint<long, 15>* FixedPoint<long, 15>::LastM1;
//FixedPoint<long, 15>* FixedPoint<long, 15>::LastM2;

TypedRefBufferView<DataSet> FFTAnalyzer::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());

	vector<double> in(start, end);

	vector<FFT5::fixed> xr(TapCount);
	vector<FFT5::fixed> xi(TapCount);

	/*auto iter = start;
	for (size_t i; i < TapCount; i++) {
		in[i].Set(*iter);
		iter++;
	}*/

	fft.load(in.data(), (FFT5::fixed*)xr.data(), (FFT5::fixed*)xi.data());
	fft.fft((FFT5::fixed*)xr.data(), (FFT5::fixed*)xi.data());

	auto& output_array = retval->Data;
	output_array.resize(TapCount);
	for (size_t i = 0; i < TapCount; i++) {
		output_array[i] = (float)(double)xr[i];
	}

	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}

/*
template <int DCT_SIZE>
TypedRefBufferView<DataSet> DCTAnalyzer<DCT_SIZE>::Process(vector<float>::const_iterator start, vector<float>::const_iterator end) {
	TypedRefBufferView<ReturnValue_t> retval(Name());
	auto& output_array = retval->Data;
	output_array.resize(size());

	dct.Process(start, end, output_array);

	LastDataSet = retval.cast<DataSet>();
	return LastDataSet;
}
*/

}
}
