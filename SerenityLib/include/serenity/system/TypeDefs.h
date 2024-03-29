#pragma once
#ifndef ENGINE_SYSTEM_TYPEDEFS_H
#define ENGINE_SYSTEM_TYPEDEFS_H

#include <cstdint>
#include <string>
#include <chrono>

using namespace std::literals;
using namespace std::chrono_literals;
using namespace std::literals::chrono_literals;

using MaskType = int;
//using MaskType = uint32_t;
using uint     = uint32_t;
using uchar    = uint8_t;
using ushort   = uint16_t;
using uint8    = uint8_t;
using uint16   = uint16_t;
using uint32   = uint32_t;
using uint64   = uint64_t;
using int8     = int8_t;
using int16    = int16_t;
using int32    = int32_t;
using int64    = int64_t;
using cstr     = char*;
using ccstr    = const char*;

inline constexpr uint8_t operator "" _uc(uint64_t arg) noexcept { return (uint8_t)arg; }
inline std::string operator "" _str(const char* cStr, std::size_t length) { return std::string(cStr, length); }

#endif