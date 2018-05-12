#include <seLib/experimental/Filtering/Filter.h>

namespace seLib {
namespace Filtering {

using namespace seLib;

//== FIR filter ========================================================================

FIR_Filter::FIR_Filter(int tapcount) {
	m_taps.resize(tapcount);
	m_sr.resize(tapcount, 0);

	for (int i = 1; i < tapcount; i++) {
		m_taps[i] = 0;
	}
	m_taps[0] = 1.0;
}

inline float FIR_Filter::Process(float input) {
	size_t m_num_taps = m_taps.size();

	double result = 0;

	for (size_t i = m_num_taps - 1; i >= 1; i--) {
		m_sr[i] = m_sr[i - 1];
		result += m_sr[i] * m_taps[i];
	}
	m_sr[0] = input;
	result += m_sr[0] * m_taps[0];

	return (float)result;
}

void FIR_Filter::Process(vector<float>& data) {
	Process(data, data); // this implementation can read and write the same array
}

void FIR_Filter::Process(const vector<float>& input, vector<float>& output) {
	output.resize(input.size());
	size_t m_num_taps = m_taps.size();

	// reset window
	for (int i = 0; i < m_num_taps; i++)
		m_sr[i] = input[0];

	for (int i = 0; i < input.size(); i++)
		output[i] = Process(input[i]);
}

void FIR_Filter::HanningWindow() {
	size_t m_num_taps = m_taps.size();
	size_t m = m_num_taps - 1;
	size_t halfLength = m_num_taps / 2;
	float fac = 2.0f * (float)M_PI / m;

	for (int n = 0; n <= halfLength; n++) {
		float val = 0.5f - 0.5f * cos(fac * n);
		m_taps[n] *= val;
		m_taps[m_num_taps - n - 1] *= val;
	}
}

FIR_Filter FIR_Filter::HighPass(int tapcount, float sample_freq, float cutoff_freq) {
	FIR_Filter filter(tapcount);

	float ft = cutoff_freq / sample_freq; // normalized cutoff freq
	float m_lambda = (float)M_PI * 2 * ft;
	int M = tapcount - 1;
	float M2 = (float)M / 2.0f;

	for (int i = 0; i < tapcount; i++) {
		float mm = i - M2;
		if (i == M2)
			filter.m_taps[i] = 1.0f - 2.0f * ft;
		else
			filter.m_taps[i] = -sin(mm * m_lambda) / (mm * (float)M_PI);
	}

	return filter;//*/
	//return BandPass(tapcount, sample_freq, cutoff_freq, 0.5 * sample_freq);
}

FIR_Filter FIR_Filter::LowPass(int tapcount, float sample_freq, float cutoff_freq) {
	FIR_Filter filter(tapcount);

	float ft = cutoff_freq / sample_freq; // normalized cutoff freq
	float m_lambda = (float)M_PI * 2.0f * ft;
	int M = tapcount - 1;
	float M2 = (float)M / 2.0f;

	for (int i = 0; i < tapcount; i++) {
		float mm = i - M2;
		if (mm == 0.0f)
			filter.m_taps[i] = 2.0f * ft;
		else
			filter.m_taps[i] = sin(mm * m_lambda) / (mm * (float)M_PI);
	}

	return filter;
	//return BandPass(tapcount, sample_freq, 0, cutoff_freq);
}

FIR_Filter FIR_Filter::BandPass(int tapcount, float sample_freq, float lower_freq, float upper_freq) {
	FIR_Filter filter(tapcount);

	float ftl = lower_freq / sample_freq; // normalized lower freq
	float ftu = upper_freq / sample_freq; // normalized upper freq
	float m_lambda = (float)M_PI * 2.0f * ftl;
	float m_phi = (float)M_PI * 2.0f * ftu;
	int M = tapcount - 1;
	float M2 = (float)M / 2.0f;

	for (int i = 0; i < tapcount; i++) {
		float mm = i - M2;
		if (i == M2)
			filter.m_taps[i] = 2.0f * (ftu - ftl);
		else
			filter.m_taps[i] = (sin(mm * m_phi) - sin(mm * m_lambda)) / (mm * (float)M_PI);
	}

	return filter;
}


//== Offset filter ========================================================================

void Offset_Filter::Process(vector<float>& data) {
	Process(data, data); // this implementation can read and write the same array
}

void Offset_Filter::Process(const vector<float>& input, vector<float>& output) {
	output.resize(input.size());

	for (int i = 0; i < input.size(); i++)
		output[i] = Process(input[i]);
}


//== Window filter ========================================================================

Window_Filter::Window_Filter(int tapcount) {
	int half = tapcount >> 1;
	tapcount = (half << 1) + 1; // make sure there's an odd number of taps

	m_taps.resize(tapcount);

	float sum = 0;
	for (int i = 0; i < tapcount; i++) {
		m_taps[i] = half - labs(half - i) + 1;
		sum += m_taps[i];
	}

	for (int i = 0; i < tapcount; i++) {
		m_taps[i] /= sum;
	}
}

void Window_Filter::Process(vector<float>& data) {
	vector<float> return_array;
	Process(data, return_array);
	return_array.swap(data);
}

void Window_Filter::Process(const vector<float>& input, vector<float>& output) {
	size_t datacount = input.size();
	size_t tapcount = m_taps.size();
	size_t half = tapcount >> 1;

	output.resize(datacount);

	// can't use all taps at start of data set
	for (int i = 0; i < half; i++) {
		float frac = 0;
		float sum = 0;
		int samplecount = half + i;
		int tapstart = tapcount - samplecount;
		for (int i2 = 0; i2 < samplecount; i2++) {
			int tap = i2 + tapstart;
			frac += m_taps[tap];
			sum += m_taps[tap] * input[i2];
		}
		output[i] = sum / frac;
	}

	// middle of data set
	for (int i = half; i < datacount - half; i++) {
		float sum = 0;
		for (int tap = 0; tap < tapcount; tap++) {
			sum += m_taps[tap] * input[i - half + tap];
		}
		output[i] = sum;
	}

	// can't use all taps at end of data set
	for (int i = datacount - half; i < datacount; i++) {
		float frac = 0;
		float sum = 0;
		int windowstart = i - half;
		for (int i2 = windowstart; i2 < datacount; i2++) {
			int tap = i2 - windowstart;
			frac += m_taps[tap];
			sum += m_taps[tap] * input[i2];
		}
		output[i] = sum / frac;
	}
}

}
}
