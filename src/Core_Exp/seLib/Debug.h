#pragma once
#include <exception>

#if defined(__cpp_exceptions) && __cpp_exceptions==199711
#define seLib_THROW_ASSERT(assert_condition, assert_exception) { if (!(assert_condition)) { throw assert_exception; } }
#else
#define seLib_THROW_ASSERT(assert_condition, assert_exception) assert(assert_condition);
#endif


namespace seLib {
namespace Debug {

class Trace;

}
}



#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif

#define SETraceLevelErr -1

#ifdef _SETraceDisable
#define seLibTrace(message, level) {}
#elif defined(__cplusplus)
#define seLibTrace(message, level) seLib::Debug::Trace::Default.Log(message, level, __FILE__, __LINE__)
#else
#define seLibTrace(message, level) seLibTraceLog(message, level, __FILE__, __LINE__)
#endif

extern "C" {

	//void seLibTraceLog(const char* message, int level, const char* file, int line);

}

#include <string>


namespace seLib {
namespace Debug {

class Trace {
public:
	static const int CriticalError = -20;
	static const int Error = -10;
	static const int Exception = -1;
	static const int Warn = 0;
	static const int Info = 5;
	static const int Debug = 10;
	static const int Verbose = 20;
	static Trace Default;
	int LogThreshold = 0;
	bool IncludeLocation = true;
	bool IncludeTime = true;
	const int64_t StartTime;
	//void (*LogPrinter)(std::string message);

public:
	Trace();
	~Trace();

	inline void Log(const std::exception& e, int level, const char* file, int line) {
		if (level <= LogThreshold)
			Log(e.what(), file, line);
	}
	inline void Log(const std::exception& e, int level) {
		if (level <= LogThreshold)
			Log(e.what());
	}
	inline void Log(std::string message, int level, const char* file, int line) {
		if (level <= LogThreshold)
			Log(message, file, line);
	}
	inline void Log(std::string message, int level) {
		if (level <= LogThreshold)
			Log(message);
	}
	void Log(std::string message);
	void Log(std::string message, const char* file, int line);

	//static void PrintfLogPrinter(std::string message);
};

class MessageException : public std::exception {
public:
  //const char* const Message;
	std::string Message;
	inline MessageException(const std::string& msg, int level = Trace::Debug) : Message(msg) {
		seLib::Debug::Trace::Default.Log(*this, level);
	}
	inline MessageException(std::string&& msg, int level = Trace::Debug) : Message(msg) {
		seLib::Debug::Trace::Default.Log(*this, level);
	}
	inline MessageException(const char* msg, int level = Trace::Debug) : Message(msg) {
		seLib::Debug::Trace::Default.Log(*this, level);
	}
	virtual const char* what() const _NOEXCEPT override {
		return Message.data();
	}
};

}
}

