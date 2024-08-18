#include <Windows.h>
#ifdef NATIVE_UNIT_TEST
#include <CppUnitTest.h>
#endif
#include <seLib/PlatformCore.h>
#include <seLib/Win32Trace.h>
#include <seLib/Debug.h>

using namespace std;

#ifdef NATIVE_UNIT_TEST
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#endif


namespace seLib {
namespace Debug {

void SETraceLog(const char* message, int level, const char* file, int line) {
  Trace::Default.Log(message, level, file, line);
}

void SETraceLog(const char* message, int level) {
  Trace::Default.Log(message, level);
}

Trace Trace::Default;

int64_t _GetCurrentTime() {
  FILETIME time;
  GetSystemTimeAsFileTime(&time);
  return (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
}

Trace::Trace() : StartTime(_GetCurrentTime()) {
	//LogPrinter = PrintfLogPrinter;
}

Trace::~Trace() {

}

void Trace::Log(string message) {
	if (IncludeTime) {
		int64_t time = (_GetCurrentTime() - StartTime) / 10000LL; // convert to milliseconds
#ifdef NATIVE_UNIT_TEST
		Logger::WriteMessage((to_string(time) + " - " + message + "\n").data());
#else
		OutputDebugStringA((to_string(time) + " - " + message + "\n").data());
#endif
	} else {
#ifdef NATIVE_UNIT_TEST
		Logger::WriteMessage((message + "\n").data());
#else
		OutputDebugStringA((message + "\n").data());
#endif
	}
}

void Trace::Log(std::string message, const char* file, int line) {
	if (IncludeTime) {
		int64_t time = (_GetCurrentTime() - StartTime) / 10000LL; // convert to milliseconds
		//OutputDebugStringA((to_string(time) + " - " + message + "\n").data());
		//LogPrinter(std::to_string(time) + " - " + message + " @" + std::string(file) + ":" + std::to_string(line));
		auto output = std::to_string(time) + " - " + message + " @" + std::string(file) + ":" + std::to_string(line);
#ifdef NATIVE_UNIT_TEST
		Logger::WriteMessage(output.data());
#else
		OutputDebugStringA(output.data());
#endif
	} else {
		//OutputDebugStringA((message + "\n").data());
		//LogPrinter(message + " @" + std::string(file) + ":" + std::to_string(line));
		auto output = message + " @" + std::string(file) + ":" + std::to_string(line);
#ifdef NATIVE_UNIT_TEST
		Logger::WriteMessage(output.data());
#else
		OutputDebugStringA(output.data());
#endif
	}
}

}
}
