#pragma once
#ifndef ENGINE_UTILS_PRECOMPILED_HEADER_H
#define ENGINE_UTILS_PRECOMPILED_HEADER_H

#include <limits>
#include <bitset>
#include <algorithm>
#include <utility>
#include <chrono>

#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <memory_resource>

#include <functional>
#include <memory>
#include <regex>
#include <random>
#include <type_traits>

#include <condition_variable>
#include <optional>
#include <atomic>
#include <future>
#include <execution>
#include <mutex>
#include <string>
#include <cstring>
#include <stdint.h>
#include <cstdint>
#include <thread>
#include <vector>
#include <array>
#include <tuple>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>
#include <deque>
#include <set>
#include <map>

//windows api
#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#endif

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/bitfield.hpp>
#include <glm/detail/type_half.hpp>

#ifdef ENGINE_USE_INLINE
    #define INLINE inline
#else
    #define INLINE
#endif

#ifdef ENGINE_USE_NOEXCEPT
    #define NOEXCEPT noexcept
#else
    #define NOEXCEPT
#endif

#ifdef ENGINE_USE_CONSTEXPR
    #define CONSTEXPR constexpr
#else
    #define CONSTEXPR
#endif

#ifdef ENGINE_USE_LIKELY
    #define LIKELY [[likely]]
    #define UNLIKELY [[unlikely]]
#else
    #define LIKELY
    #define UNLIKELY
#endif

#if defined(ENGINE_PRODUCTION)
    #define ENGINE_PRODUCTION_LOG(x)
#else
    #define ENGINE_PRODUCTION_LOG(x) std::cout << x << '\n';
#endif


//#define ENGINE_FORCE_DISABLE_THREAD_WINDOW_EVENTS //this will force the engine and window to use the same thread, which will prevent the engine logic from executing if the user modifies the window. this can cause some flickering on resize however
//#define ENGINE_FORCE_PHYSICS_DEBUG_DRAW //this will force the renderer to output physics debugging info regardless if the build is debug or release

#if defined(__AVX2__) || defined( __AVX__ ) || (defined(_M_AMD64) || defined(_M_X64)) || _M_IX86_FP || defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__) || defined(__SSE4_1__)
    #define ENGINE_SIMD_SUPPORTED
#endif

#if defined(ENGINE_PRODUCTION)
    #define ASSERT(condition, message) ((void)0)
#else
    #define ASSERT(condition, message) \
        do { \
            if (! (condition)) { \
                std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message << '\n'; \
                std::cin.get(); \
                std::terminate(); \
            } \
        } while (false)
#endif

using uint   = std::uint32_t;
using uchar  = std::uint8_t;
using ushort = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

#ifndef ENGINE_HIGH_PRECISION
typedef float       decimal;
typedef glm::vec2   glm_vec2;
typedef glm::vec3   glm_vec3;
typedef glm::vec4   glm_vec4;
typedef glm::quat   glm_quat;
typedef glm::mat4   glm_mat3;
typedef glm::mat4   glm_mat4;
#else
#define BT_USE_DOUBLE_PRECISION

typedef double      decimal;
typedef glm::dvec2  glm_vec2;
typedef glm::dvec3  glm_vec3;
typedef glm::dvec4  glm_vec4;
typedef glm::dquat  glm_quat;
typedef glm::dmat3  glm_mat3;
typedef glm::dmat4  glm_mat4;
#endif

#pragma region EnvironmentDefines

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#ifdef ENVIRONMENT64
#else
#endif

#pragma endregion

#pragma region MemoryLeakDetection

#if defined(_DEBUG) && !defined(ENVIRONMENT32)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to be of _CLIENT_BLOCK type
#define _CRTDBG_MAP_ALLOC
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
#define MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#define FREE(block) _free_dbg(block, _NORMAL_BLOCK)
#else
#define NEW new
#define MALLOC malloc
#define FREE free
#endif

#pragma endregion

#pragma region DeleteMacros

#define SAFE_DELETE_FUTURE(x){ \
    if (x) { \
        if (x->_Is_ready() || x->_Is_ready_at_thread_exit()) { \
            x->get(); \
        } \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_THREAD(x){ \
    if (x) { \
        if (x->joinable()) { \
            x->join(); \
        } \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_COM(x) { \
    if(x){ \
        x->Release(); \
        x = 0; \
    } \
}
#define SAFE_DELETE(x) { \
    if(x){ \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_VECTOR(x){ \
    for (size_t i = 0; i < x.size(); ++i) { \
        SAFE_DELETE(x[i]); \
    } \
    x.clear(); \
}
#define SAFE_DELETE_ARRAY(x){ \
    for (size_t i = 0; i < x.size(); ++i) { \
        SAFE_DELETE(x[i]); \
    } \
}
#define SAFE_DELETE_QUEUE(x){ \
    while (x.size() > 0) { \
        auto& val = x.front(); \
        SAFE_DELETE(val); \
        x.pop(); \
    } \
}
#define SAFE_DELETE_MAP(x){ \
    if(x.size() > 0){ \
        for(auto& it : x){ \
            SAFE_DELETE(it.second); \
        } \
        x.clear(); \
    } \
}

///
#define DELETE_FUTURE(x){ \
    if (x->_Is_ready() || x->_Is_ready_at_thread_exit()) { \
        x->get(); \
    } \
    delete x; \
}
#define DELETE_THREAD(x){ \
    if (x->joinable()) { \
        x->join(); \
    } \
    delete x; \
}
#define DELETE_COM(x) { \
    x->Release(); \
    x = 0; \
}
#define DELETE_VECTOR(x){ \
    for (size_t i = 0; i < x.size(); ++i) { \
        delete(x[i]); \
    } \
    x.clear(); \
}
#define DELETE_QUEUE(x){ \
    while (x.size() > 0) { \
        auto& val = x.front(); \
        delete(val); \
        x.pop(); \
    } \
}
#define DELETE_MAP(x){ \
    for(auto& it : x){ \
        delete(it.second); \
    } \
    x.clear(); \
}

#pragma endregion

inline constexpr unsigned char operator "" _uc(unsigned long long arg) noexcept { return (unsigned char)arg; }
inline std::string operator "" _str(const char* cStr, std::size_t length) { return std::string(cStr, length); }

#include <core/engine/types/SmallMap.h>
#include <core/engine/types/StaticString.h>
#include <core/engine/types/ColorVector.h>
#include <core/engine/types/Flag.h>
#include <core/engine/utils/BlockProfiler.h>

namespace Engine {
    class UserPointer {
        protected:
            void* m_UserPointer = nullptr;
        public:
            UserPointer() = default;
            ~UserPointer() = default;

            inline void setUserPointer(void* userPointer) noexcept { m_UserPointer = userPointer; }
            inline CONSTEXPR void* getUserPointer() const noexcept { return m_UserPointer; }
    };
    class NonCopyable {
        public: 
            NonCopyable() = default;
            ~NonCopyable() = default;
        private:
            NonCopyable(const NonCopyable& other) = delete;
            NonCopyable& operator=(const NonCopyable& other) = delete;
    };
    class NonMoveable {
        public:
            NonMoveable() = default;
            ~NonMoveable() = default;
        private:
            NonMoveable(NonMoveable&&) noexcept = delete;
            NonMoveable& operator=(NonMoveable&&) noexcept = delete;
    };

    void printEndianness() noexcept {
        std::uint32_t data;
        std::uint8_t* cptr;
        data = 1; //Assign data
        cptr = (std::uint8_t*)&data; //Type cast
        if (*cptr == 1) {
            ENGINE_PRODUCTION_LOG("little-endiann")
        }else if (*cptr == 0) {
            ENGINE_PRODUCTION_LOG("big-endiann")
        }
    }
};

#endif