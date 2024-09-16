#pragma once

// stl
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

#if defined(__GNUC__) || defined(__clang__)
#ifndef __MINGW32__
#define STRONG_INLINE __attribute__((always_inline)) inline
#else
#define STRONG_INLINE inline
#endif
#elif defined(_MSC_VER)
#define STRONG_INLINE __pragma(warning(suppress : 4714)) inline __forceinline
#else
#define STRONG_INLINE inline
#endif

#define V8_SCOPE(isolate)                     \
    v8::Locker locker(isolate);               \
    v8::Isolate::Scope isolateScope(isolate); \
    v8::HandleScope scope(isolate)
