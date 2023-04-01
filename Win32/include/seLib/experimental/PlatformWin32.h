#pragma once
/*
Copyright (c) 2016 Scott Early

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef WIN32
#error Attempted to include Win32 platform support in a non-Win32 build.
#endif // !_WIN32

#include <stdint.h>
#include <mutex>
#include "Win32Trace.h"

//#include <gl\glew.h>
//#include <gl\glu.h>

//using namespace std;
//using namespace SE;

namespace seLib {
namespace PlatformSupport{

typedef float HGRfloat; // apply correct GL float type to HGraph generated vectors
//typedef uint32_t GLuint;

class MS_Timer {
protected:
	double _time;
public:
	MS_Timer();

	inline MS_Timer& AddMilliseconds(int ms) {
		_time += (double)ms / 1000;
		return *this;
	}

	inline int DiffMilliseconds(const MS_Timer& right) const {
		return (int)((_time - right._time) * 1000);
	}

	inline bool operator<(const MS_Timer& right) const {
		return _time < right._time;
	}

	inline bool operator>(const MS_Timer& right) const {
		return _time > right._time;
	}

	inline bool operator==(const MS_Timer& right) const {
		return _time == right._time;
	}

	/*static inline MS_Timer Now() {
	return MS_Timer();
	}//*/
};

}
}
