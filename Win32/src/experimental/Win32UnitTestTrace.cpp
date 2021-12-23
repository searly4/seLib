#include "CppUnitTest.h"
#include <PlatformCore.h>
#include "Win32Trace.h"
#include <Windows.h>
#include <SEDebug.h>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SE {
namespace Debug {

void SETraceLog(const char* message, int level, const char* file, int line) {
  Trace::Default.Log(message, level, file, line);
}

void SETraceLog(const char* message, int level) {
  Trace::Default.Log(message, level);
}

int64_t _GetCurrentTime() {
  FILETIME time;
  GetSystemTimeAsFileTime(&time);
  return (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
}

Trace Trace::Default;

Trace::Trace() : StartTime(_GetCurrentTime()) {
}

Trace::~Trace() {

}

void Trace::Log(string message) {
  if (IncludeTime) {
    int64_t time = (_GetCurrentTime() - StartTime) / 10000LL; // convert to milliseconds
    Logger::WriteMessage((to_string(time) + " - " + message + "\n").data());
  } else {
    Logger::WriteMessage((message + "\n").data());
  }
}

}
}
