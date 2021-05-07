#pragma once
#ifndef ENGINE_SYSTEM_MACROS_H
#define ENGINE_SYSTEM_MACROS_H

#include <iostream>

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
#define constexpr constexpr
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



#if defined(__clang__)
#define DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) || defined(__GNUG__)
#define DEBUG_BREAK() __builtin_trap()
#elif defined(_MSC_VER) || defined(_MSC_FULL_VER)
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#if defined(SIGTRAP)
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK() raise(SIGABRT)
#endif
#endif


#if defined(ENGINE_PRODUCTION)
#define ASSERT(condition, message) ((void)0)
#else
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message << '\n'; \
            DEBUG_BREAK(); \
            std::terminate(); \
        } \
    } while (false)
#endif

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

#if (defined(_DEBUG) && !defined(ENVIRONMENT32)) || defined(_CRTDBG_MAP_ALLOC)

#ifndef BT_DEBUG_MEMORY_ALLOCATIONS
#define BT_DEBUG_MEMORY_ALLOCATIONS
#endif

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif

// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to be of _CLIENT_BLOCK type
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

#define BUILD_BEGIN_END_ITR_CLASS_MEMBERS(TYPE, VAR) \
    inline TYPE::iterator begin() noexcept { return VAR.begin(); } \
    inline TYPE::const_iterator begin() const noexcept { return VAR.begin(); } \
    inline TYPE::iterator end() noexcept { return VAR.end(); } \
    inline TYPE::const_iterator end() const noexcept { return VAR.end(); } \
    inline const TYPE::const_iterator cbegin() const noexcept { return VAR.cbegin(); } \
    inline const TYPE::const_iterator cend() const noexcept { return VAR.cend(); }

#define BUILD_TEMPLATE_BEGIN_END_ITR_CLASS_MEMBERS(TYPE, VAR) \
    inline typename TYPE::iterator begin() noexcept { return VAR.begin(); } \
    inline typename TYPE::const_iterator begin() const noexcept { return VAR.begin(); } \
    inline typename TYPE::iterator end() noexcept { return VAR.end(); } \
    inline typename TYPE::const_iterator end() const noexcept { return VAR.end(); } \
    inline typename const TYPE::const_iterator cbegin() const noexcept { return VAR.cbegin(); } \
    inline typename const TYPE::const_iterator cend() const noexcept { return VAR.cend(); }

#pragma endregion