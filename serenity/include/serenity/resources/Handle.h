#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_H
#define ENGINE_RESOURCE_HANDLE_H

#include <serenity/system/EngineIncludes.h>
#include <serenity/types/ViewPointer.h>
#include <cassert>
#include <string>
#include <mutex>

namespace Engine::priv {
    class ResourceModule;
};

class Handle final {
    friend class Engine::priv::ResourceModule;
    private:
        [[nodiscard]] void* internal_get_base(uint32_t resourceTypeID) const noexcept;
        [[nodiscard]] void* internal_get_base_thread_safe(uint32_t resourceTypeID) const noexcept;


        uint32_t m_Index   : 23 = 0;
        uint32_t m_Type    : 9  = 0;
    public:
        constexpr Handle() noexcept = default;
        constexpr Handle(const uint32_t index, const uint32_t type) noexcept
            : m_Index{ index }
            , m_Type { type }
        {}
        Handle(const Handle&) = default;
        Handle& operator=(const Handle&) = default;
        Handle(Handle&& other) noexcept
            : m_Index{ other.m_Index }
            , m_Type{ other.m_Type }
        {
            other.m_Index = 0;
            other.m_Type  = 0;
        }
        Handle& operator=(Handle&& other) noexcept {
            if (this != &other) {
                m_Index = other.m_Index;
                m_Type  = other.m_Type;
                other.m_Index = 0;
                other.m_Type  = 0;
            }
            return *this;
        }

        //Don't use these, they are only here to make it compatible to std locks
        bool try_lock() const;

        //Don't use these, they are only here to make it compatible to std locks
        void lock() const;

        //Don't use these, they are only here to make it compatible to std locks
        void unlock() const noexcept;


        [[nodiscard]] inline constexpr uint32_t index() const { return m_Index; }
        [[nodiscard]] inline constexpr uint32_t type() const { return m_Type; }
        [[nodiscard]] inline std::string toString() const { return std::to_string(m_Index) + ", " + std::to_string(m_Type); }
        //inline explicit constexpr operator uint32_t() const noexcept { return m_Type << 23 | m_Index; }
        inline constexpr operator uint32_t() const noexcept { return m_Index; }
        inline constexpr operator size_t() const noexcept { return size_t(m_Index); }
        inline constexpr operator bool() const noexcept { return !null(); }
        
        inline constexpr bool operator<(const Handle& other) const noexcept { return m_Index < other.m_Index; }
        inline constexpr bool operator>(const Handle& other) const noexcept { return m_Index > other.m_Index; }
        inline constexpr bool operator<=(const Handle& other) const noexcept { return m_Index <= other.m_Index; }
        inline constexpr bool operator>=(const Handle& other) const noexcept { return m_Index >= other.m_Index; }
        inline constexpr bool operator==(const Handle& other) const noexcept {  return (m_Index == other.m_Index && m_Type == other.m_Type); }
        
        [[nodiscard]] inline constexpr bool null() const noexcept { return (m_Index == 0 && m_Type == 0); }

        template<class RESOURCE> 
        [[nodiscard]] inline operator RESOURCE*() const noexcept { return get<RESOURCE>(); }

        template<class RESOURCE>
        [[nodiscard]] inline RESOURCE* get() const noexcept {
            RESOURCE* ret = static_cast<RESOURCE*>(this->internal_get_base(RESOURCE::TYPE_ID));
            assert((ret != nullptr && !null()) || (ret == nullptr && null()));
            return ret;
        }
        template<class RESOURCE>
        [[nodiscard]] inline RESOURCE* getThreadSafe() noexcept {
            RESOURCE* ret = static_cast<RESOURCE*>(this->internal_get_base_thread_safe(RESOURCE::TYPE_ID));
            assert((ret != nullptr && !null()) || (ret == nullptr && null()));
            return ret;
        }

        [[nodiscard]] Engine::view_ptr<std::mutex> getMutex() const noexcept;
};

namespace std {
    template <>
    struct hash<Handle> {
        std::size_t operator()(const Handle& handle) const noexcept {
            using std::hash;
            return ((hash<uint32_t>()(handle.index()) ^ (hash<uint32_t>()(handle.type()) << 1)) >> 1);
        }
    };
};

template<class RESOURCE>
struct LoadedResource final {
    Engine::view_ptr<RESOURCE> m_Resource = nullptr;
    Handle                     m_Handle;

    inline operator Handle() const noexcept { return m_Handle; }
    inline operator RESOURCE&() const noexcept { return *m_Resource; }
    inline operator RESOURCE*() const noexcept { return m_Resource; }

    void operator=(const Handle otherHandle) {
        m_Handle   = otherHandle;
        m_Resource = m_Handle.get<RESOURCE>();
    }
};

#endif