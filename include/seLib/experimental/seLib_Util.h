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

//#include "ProjectConfig.h"

#pragma GCC diagnostic ignored "-Wpmf-conversions"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
#include <utility>
#endif

#define Q(_x_) # _x_
#define QUOTE(_x_) Q(_x_)

#define sizeofarray(array) (sizeof(array) / sizeof(array[0]))

#define CriticalSection(code) \
	CRITICAL_REGION_ENTER(); \
	code \
	CRITICAL_REGION_EXIT();

#ifdef __cplusplus
extern "C" {
#endif

typedef enum seLib_TraceVal_e {
	TraceVal_Error,
	TraceVal_Misc,
	TraceVal_Misc0,
	TraceVal_Misc1,
	TraceVal_Misc2,
	TraceVal_Misc3,
	TraceVal_Misc4,
	TraceVal_Misc5,
	TraceVal_Misc6,
	TraceVal_Misc7,
	TraceVal_EnterFunction,
	TraceVal_Init,
	TraceVal_BoardInit,
	TraceVal_ModeStandby,
	TraceVal_ModeStreaming,
} seLib_TraceVal_t;

typedef enum seLib_TraceLevel_e {
    TraceLevel_Error,
    TraceLevel_Info,
    TraceLevel_Debug,
    TraceLevel_Verbose,
} seLib_TraceLevel_t;

typedef struct seLib_TracePacket_s {
	seLib_TraceVal_t val;
	const void* loc;
	const char* msg;
} seLib_TracePacket_t;

inline uint8_t seLib_HexLow(uint8_t val) {
    val &= 0x0F;
    return (val < 0x0A) ? (val + '0') : (val + 'A' - 0x0A);
}

inline uint8_t seLib_HexHigh(uint8_t val) {
    val >>= 4;
    val &= 0x0F;
    return (val < 0x0A) ? (val + '0') : (val + 'A' - 0x0A);
}

extern seLib_TraceLevel_t seLib_LogLevel;
void seLib_Trace(seLib_TraceVal_t enumval, const void* loc, const char* message);
void seLib_Trace2(seLib_TraceVal_t enumval, const void* loc, const char* message, uint32_t val);
void seLib_TraceErr(seLib_TraceVal_t enumval, const void* loc, const char* message, uint32_t val, uint32_t errcode);
void seLib_Assert(seLib_TraceVal_t enumval, const void* loc, const char* message, uint32_t val, uint32_t errcode);
void seLib_DebugString(const char* string);
inline void seLib_Log(seLib_TraceLevel_t level, const char* message) {
    if (level <= seLib_LogLevel)
        seLib_DebugString(message);
}

// for release builds, allow function and message content to optimize out
inline void seLib_DebugLog(seLib_TraceLevel_t level, const char* message) {
#ifdef DEBUG
    seLib_Log(level, message);
#endif
}


#ifdef __cplusplus
}

namespace seLib {

// returns constant of type T with upper <count> bits set to 1
template <typename T>
constexpr inline T MaskUpper(unsigned count) {
	return (~(T)0) << ((sizeof(T) * 8) - count);
}

// returns constant of type T with lower <count> bits set to 1
template <typename T>
constexpr inline T MaskLower(unsigned count) {
	return ~((~(T)0) << count);
}

namespace Trace {

	using TraceVal_t = seLib_TraceVal_t;
	using TraceLevel_t = seLib_TraceLevel_t;
	using TracePacket_t = seLib_TracePacket_t;
	template <typename... Args> auto Trace(Args&&... args) -> decltype(seLib_Trace(std::forward<Args>(args)...)) { return seLib_Trace(std::forward<Args>(args)...); }
	template <typename... Args> auto Trace2(Args&&... args) -> decltype(seLib_Trace2(std::forward<Args>(args)...)) { return seLib_Trace2(std::forward<Args>(args)...); }
	template <typename... Args> auto TraceErr(Args&&... args) -> decltype(seLib_TraceErr(std::forward<Args>(args)...)) { return seLib_TraceErr(std::forward<Args>(args)...); }
	template <typename... Args> auto Assert(Args&&... args) -> decltype(seLib_Assert(std::forward<Args>(args)...)) { return seLib_Assert(std::forward<Args>(args)...); }
	template <typename... Args> auto DebugString(Args&&... args) -> decltype(seLib_DebugString(std::forward<Args>(args)...)) { return seLib_DebugString(std::forward<Args>(args)...); }
	template <typename... Args> auto Log(Args&&... args) -> decltype(seLib_Log(std::forward<Args>(args)...)) { return seLib_Log(std::forward<Args>(args)...); }
	template <typename... Args> auto DebugLog(Args&&... args) -> decltype(seLib_DebugLog(std::forward<Args>(args)...)) { return seLib_DebugLog(std::forward<Args>(args)...); }
	template <typename... Args> auto HexLow(Args&&... args) -> decltype(seLib_HexLow(std::forward<Args>(args)...)) { return seLib_HexLow(std::forward<Args>(args)...); }
	template <typename... Args> auto HexHigh(Args&&... args) -> decltype(seLib_HexHigh(std::forward<Args>(args)...)) { return seLib_HexHigh(std::forward<Args>(args)...); }

}

template <typename Self_T, typename Storage_T = uint32_t>
class EnumClass {
public:
	Storage_T Value;

	EnumClass(const Self_T& value) : Value(value.Value) {}
	
	EnumClass(const Storage_T& value) : Value(value) {}
	EnumClass(int value) : Value((Storage_T)value) {}
	
	Self_T operator|(const Self_T& b) const {
		return Self_T(Value | b.Value);
	}

	Self_T operator&(const Self_T& b) const {
		return Self_T(Value & b.Value);
	}

	bool operator==(const Self_T& b) const {
		return (Value & b.Value) == b.Value;
	}
	
	Self_T& operator=(const Storage_T& value) {
		Value = value;
		return *this;
	}
};

template <typename ...Args>
struct base_forwarder {
	virtual void call(Args && ...args) const = 0;
};

template <typename ...Args>
struct class_forwarder : base_forwarder<Args...> {
	void* const f;
	void* const ref;
	constexpr class_forwarder(void* f_ptr = nullptr, void* obj_ptr = nullptr) : f(f_ptr), ref(obj_ptr) {}
	constexpr class_forwarder(class_forwarder const &) = default;
	constexpr class_forwarder(class_forwarder &&) = default;
	virtual void call(Args && ...args) const override {
	  ref.*f(std::forward<Args>(args)...);
	}
};

template <typename ...Args>
struct function_forwarder : base_forwarder<Args...> {
	void* const f;
	constexpr function_forwarder(void* f_ptr) : f(f_ptr) {}
	constexpr function_forwarder(function_forwarder const &) = default;
	constexpr function_forwarder(function_forwarder &&) = default;
	virtual void call(Args && ...args) const override {
	  *f(std::forward<Args>(args)...);
	}
};

template <typename ...Args>
union call_forwarder {
	class_forwarder<Args...> class_t;
	function_forwarder<Args...> function_t;
};


}

#endif

