#pragma once
#include <stdint.h>

#ifdef _MSC_VER
#if _M_X64 == 100 || _M_IX86 == 600
#define __ORDER_LITTLE_ENDIAN__ 1
//#define __ORDER_BIG_ENDIAN__ 1
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif
#endif

inline uint32_t LittleEndian_UINT32(const uint8_t* val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return *((uint32_t*)val);
#else
  return (
    ((uint32_t)val[0]) |
    ((uint32_t)val[1]) << 8 |
    ((uint32_t)val[2]) << 16 |
    ((uint32_t)val[3]) << 24
    );
#endif
}

inline void LittleEndian_UINT32(uint32_t val, uint8_t* out) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  *((uint32_t*)out) = val;
#else
  out[0] = (uint8_t)(val & ((uint32_t)0xFF));
  out[1] = (uint8_t)(val & ((uint32_t)0xFF) << 8);
  out[2] = (uint8_t)(val & ((uint32_t)0xFF) << 16);
  out[3] = (uint8_t)(val & ((uint32_t)0xFF) << 24);
#endif
}

inline int32_t LittleEndian_INT32(const uint8_t* val) {
  return (int32_t)LittleEndian_UINT32(val);
}

inline void LittleEndian_INT32(int32_t val, uint8_t* out) {
  LittleEndian_UINT32(*((uint32_t*)&val), out);
}

inline uint16_t LittleEndian_UINT16(const uint8_t* val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return *((uint16_t*)val);
#else
  return (
    ((uint16_t)val[0]) |
    ((uint16_t)val[1]) << 8
    );
#endif
}

inline void LittleEndian_UINT16(uint16_t val, uint8_t* out) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  *((uint16_t*)out) = val;
#else
  out[0] = (uint8_t)(val & ((uint16_t)0xFF));
  out[1] = (uint8_t)(val & ((uint16_t)0xFF) << 8);
#endif
}

inline int16_t LittleEndian_INT16(const uint8_t* val) {
  return (int16_t)LittleEndian_UINT16(val);
}

inline void LittleEndian_INT16(int16_t val, uint8_t* out) {
  LittleEndian_UINT16(*((uint16_t*)&val), out);
}

//============================================================================
// Win32 support

#ifdef WIN32

#ifndef __cplusplus_cli
#define __CLI_PUBLIC
#endif

#include <mutex>

namespace seLib {

//using LockObj = std::mutex;
//
//class LockGuard {
//public:
//  LockGuard(LockObj& lock) {}
//};

}

#endif

//============================================================================
// Emscripten support

#ifdef __EMSCRIPTEN__
#undef _WIN32
#define __CLI_PUBLIC

namespace seLib {

typedef int LockObj;

class LockGuard {
public:
  LockGuard(LockObj& lock) {}
};

}

#endif
