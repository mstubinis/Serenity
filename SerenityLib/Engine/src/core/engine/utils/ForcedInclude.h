#pragma once
#ifndef ENGINE_FORCED_INCLUDE_H
#define ENGINE_FORCED_INCLUDE_H

//#include <locale>
#include <stdint.h>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

//#define ENGINE_FORCE_DISABLE_THREAD_WINDOW_EVENTS //this will force the engine and window to use the same thread, which will prevent the engine logic from executing if the user modifies the window. this can cause some flickering on resize however
//#define ENGINE_FORCE_PHYSICS_DEBUG_DRAW //this will force the renderer to output physics debugging info regardless if the build is debug or release


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

#ifdef _DEBUG
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
    if(x.size() > 0) { \
        for (size_t i = 0; i < x.size(); ++i) { \
            SAFE_DELETE(x[i]); \
        } \
        x.clear(); \
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

inline constexpr unsigned char operator "" _uc(unsigned long long arg) noexcept{
    return static_cast<unsigned char>(arg);
}

namespace Engine {
    class NonCopyable {
        public:
            NonCopyable()  = default;
            ~NonCopyable() = default;
        private:
            NonCopyable(const NonCopyable&)                = delete;
            NonCopyable& operator=(const NonCopyable&)     = delete;
    };
    class NonMoveable {
        public:
            NonMoveable()  = default;
            ~NonMoveable() = default;
        private:
            NonMoveable(NonMoveable&&) noexcept            = delete;
            NonMoveable& operator=(NonMoveable&&) noexcept = delete;
    };

    template<typename T> class Flag final {
        private:
            T m_Flags;
        public:
            Flag() : m_Flags(0) {}
            ~Flag() {}

            Flag& operator=(const T& other) {
                m_Flags = other;
                return *this;
            }
            const T& get() const {
                return m_Flags;
            }
            T operator&(const T& other) {
                return m_Flags & other;
            }
            T operator|(const T& other) {
                return m_Flags | other;
            }
            T operator&=(const T& other) {
                return m_Flags &= other;
            }
            T operator|=(const T& other) {
                return m_Flags |= other;
            }

            void add(const T& flag) {
                if (m_Flags != (m_Flags | flag)) {
                    m_Flags = m_Flags | flag;
                }
            }
            void remove(const T& flag) {
                if (m_Flags != (m_Flags & ~flag)) {
                    m_Flags = m_Flags & ~flag;
                }
            }
            const bool has(const T& flag) const {
                return (m_Flags & flag) ? true : false;
            }
    };


    struct color_vector_4 {
        glm::vec<4, unsigned char, glm::packed_highp> color = glm::vec<4, unsigned char, glm::packed_highp>(0);


        color_vector_4() = default;
        explicit color_vector_4(const float color);
        explicit color_vector_4(const float r, const float g, const float b, const float a);
        explicit color_vector_4(const glm::vec4& color);
        explicit color_vector_4(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a);
        explicit color_vector_4(const unsigned char color);

        const uint32_t toPackedInt() const;
        glm::vec4 unpackInt(uint32_t i) const;

        inline float r() const { return static_cast<float>(color.r) * 0.003921568627451f; }
        inline float g() const { return static_cast<float>(color.g) * 0.003921568627451f; }
        inline float b() const { return static_cast<float>(color.b) * 0.003921568627451f; }
        inline float a() const { return static_cast<float>(color.a) * 0.003921568627451f; }
    };

};

#endif