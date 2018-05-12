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

#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <array>

#include <seLib/FixedPoint.h>

namespace seLib {
namespace Filtering {

using namespace std;
using namespace seLib;

class FFT5 {
public:
	const double pi = 3.1415926535897932384626433832795;
	//typedef double fixed;
	typedef FixedPoint<1, int16_t, int32_t> fixed;
	//typedef tfixed16<15> fixed;
	//typedef int fixed;

	// Wave Table parameters
	static const int WTC = 256;
	static const int WTBC = 8;
	static const int WTMask = 255;
	// Data Maximum parameters
	static const int DMC = 128;
	static const int DMBC = 7;

	// Data parameters
	const int DC;
	const int DBC;

	// table access parameters
	const int BCDiff;
	const int CDiff;

	vector<int> BR;
	const int C1;
	const int m;

	vector<fixed> WR; // wr[n*2]; wr[i] = cos(pin*i)
	vector<fixed> WI; // wi[n*2]; wi[i] = -sin(pin*i)

	FFT5(size_t count) :
		DC((int)count >> 1), DBC(GenBC(DC)),
		BCDiff(DMBC - DBC), CDiff(1 << BCDiff),
		BR(GenBR()), WR(GenWR()), WI(GenWI()),
		m(16383), C1(DC-1) {}

	~FFT5() {
		//delete []const_cast<fixed*>(WR);
		//delete []const_cast<fixed*>(WI);
		//delete []const_cast<int*>(BR);
	}

	void operator()(double* data) {
		fixed *xr = new fixed[DC];
		fixed *xi = new fixed[DC];
		load(data, xr, xi);
		fft(xr, xi);
		rfft(data, xr, xi);
		delete []xr; delete []xi;
	}

	void load(const double* data, fixed *xr, fixed *xi) {
		int i = 0, j = 0;
		while (i < DC) {
			xr[BR[j]] = data[i << 1];
			xi[BR[j]] = data[(i << 1) + 1];
			i++;
			j += CDiff;
		}
	}

	void fft(fixed *xr, fixed *xi) {
		int i = DMBC, bfsize = 1, pinc = DMC;
		do {
			int j = 0;
			do {
				int wp = 0,//WTMask & (j << i),
					m = j + bfsize,
					jl = m;
				do {
					fixed t0, t1;
					t0 = (xr[j] >> 1) - WR[wp] * xr[m] + WI[wp] * xi[m];
					t1 = (xi[j] >> 1) - WR[wp] * xi[m] - WI[wp] * xr[m];
					xr[j] = (xr[j] >> 1) + WR[wp] * xr[m] - WI[wp] * xi[m];
					xi[j] = (xi[j] >> 1) + WR[wp] * xi[m] + WI[wp] * xr[m];
					xr[m] = t0;
					xi[m] = t1;
					j++, m++, wp = WTMask & (wp + pinc);
				} while (j < jl);
				j += bfsize;
			} while (j < DC);
			bfsize <<= 1; pinc >>= 1;// i--;
		} while (bfsize < DC);
	}

	void rfft(double* data, fixed *xr, fixed *xi) {
		data[0] = pow((double)xr[0] + (double)xi[0], 2);
		int i = 1, wt1 = CDiff, wt2 = DMC >> 1;
		while (i < DC) {
			fixed xrt = (xr[i] + xr[DC - i]) >> 2,
				xit = (xi[i] - xi[DC - i]) >> 2,
				tr = (xr[i] - xr[DC - i]) >> 1,
				ti = (xi[i] + xi[DC - i]) >> 1;
			fixed
				outr = xrt + (ti * WR[wt1]) + (tr * WI[wt1]),
				outi = xit + (ti * WI[wt1]) - (tr * WR[wt1]);
			data[i] = scale(outr, outi);
			i++;
			wt1 += CDiff;
			wt2 += CDiff;
//			wt2 &= WTMask;
		}
	}

	fixed scale(const fixed& rv, const fixed& iv) {
		fixed magsq, mag, dbfact, decibel;
		magsq = rv * rv + iv * iv << 1;
		mag = (rv * rv + iv * iv << 1).Sqrt();
		decibel = 0;
		if (((double)magsq * m) >= .5) {
//			dbfact = m * m / 0x10000;
			decibel = .5 * log10((double)magsq * 4096) * 0.434294481;
		}
//		return decibel;
		return mag;
	}

	//int getwr(int i) const {return (int)(WR[i]());}
	//int getwi(int i) const {return (int)(WI[i]());}
	//int getbr(int i) const {return BR[i];}

private:
	constexpr unsigned GenBC(int val) { // ret = log2(val)
		for (unsigned i = 0; ; i++)
			if (val & (1 << i))
				return i;
	}

	vector<fixed> GenWR() {
		vector<fixed> t(WTC);
		for (unsigned i = 0; i < WTC; i++) {
			t[i] = cos(pi * 2 * i / WTC);
		}
		return t;
	}

	vector<fixed> GenWI() {
		vector<fixed> t(WTC);
		for (unsigned i = 0; i < WTC; i++) {
			t[i] = -sin(pi * 2 * i / WTC);
		}
		return t;
	}

	vector<int> GenBR() {
		vector<int> t(DMC);
		for (unsigned i = 0; i < DMC; i++) {
			unsigned i2, rev, index = i;
			for (i2=rev=0; i2 < DMBC; i2++) {
				rev = (rev << 1) | (index & 1);
				index >>= 1;
			}
			t[i] = rev;
		}
		return t;
	}
};


	// FFT
	// -- n*log2(n) total operations (896 ops at n=128, 2048 at n=256)
	// -- inner loop: 1 AND, 4 MACS, 4 adds, 4 rshifts(?), 1 lshift
	//   -- 

}
}
