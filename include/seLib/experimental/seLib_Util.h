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
	TraceVal_t val;
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
    if (level <= LogLevel)
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
namespace Trace {

  using TraceVal_t = seLib_TraceVal_t;
  using TraceLevel_t = seLib_TraceLevel_t;
  using TracePacket_t = seLib_TracePacket_t;
  using Trace = seLib_Trace;
  using Trace2 = seLib_Trace2;
  using TraceErr = seLib_TraceErr;
  using Assert = seLib_Assert;
  using DebugString = seLib_DebugString;
  using Log = seLib_Log;
  using DebugLog = seLib_DebugLog;
  using HexLow = seLib_HexLow;
  using HexHigh = seLib_HexHigh;

}
}

#endif

