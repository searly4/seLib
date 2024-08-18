//#include <Windows.h>
//#include <CppUnitTest.h>
//#include <seLib/PlatformCore.h>
//#include <seLib/Win32Trace.h>
//#include <seLib/Debug.h>
//
//using namespace std;
//
//#ifdef NATIVE_UNIT_TEST
//using namespace Microsoft::VisualStudio::CppUnitTestFramework;
//#endif
//
//namespace seLib {
//namespace Debug {
//
//void SETraceLog(const char* message, int level, const char* file, int line) {
//  Trace::Default.Log(message, level, file, line);
//}
//
//void SETraceLog(const char* message, int level) {
//  Trace::Default.Log(message, level);
//}
//
//Trace Trace::Default;
//
//int64_t _GetCurrentTime() {
//  FILETIME time;
//  GetSystemTimeAsFileTime(&time);
//  return (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
//}
//
//Trace::Trace() : StartTime(_GetCurrentTime()) {
//}
//
//Trace::~Trace() {
//
//}
//
//void Trace::Log(string message) {
//  if (IncludeTime) {
//    int64_t time = (_GetCurrentTime() - StartTime) / 10000LL; // convert to milliseconds
//    Logger::WriteMessage((to_string(time) + " - " + message + "\n").data());
//  } else {
//    Logger::WriteMessage((message + "\n").data());
//  }
//}
//
//}
//}
