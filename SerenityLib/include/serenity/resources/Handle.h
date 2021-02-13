#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_H
#define ENGINE_RESOURCE_HANDLE_H

#include <serenity/resources/ResourcesIncludes.h>
#include <serenity/system/TypeDefs.h>
#include <serenity/types/ViewPointer.h>
#include <serenity/system/Macros.h>
#include <mutex>

class SoundQueue;
namespace Engine::priv {
    class ResourceModule;
};

class Handle final {
    friend class ::SoundQueue;
    friend class Engine::priv::ResourceModule;
    private:
        void* internal_get_base() const noexcept;
        void* internal_get_base_thread_safe() noexcept;

        uint32_t m_Index   : 12;
        uint32_t m_Version : 15;
        uint32_t m_Type    : 5;
    public:
        constexpr Handle()
            : m_Index{ 0 }
            , m_Version{ 0 }
            , m_Type{ 0 }
        {}
        constexpr Handle(const uint32_t index, const uint32_t version, const uint32_t type)
            : m_Index{ index }
            , m_Version{ version }
            , m_Type { type }
        {}

        [[nodiscard]] inline constexpr uint32_t index() const noexcept { return m_Index; }
        [[nodiscard]] inline constexpr uint32_t version() const noexcept { return m_Version; }
        [[nodiscard]] inline constexpr uint32_t type() const noexcept { return m_Type; }
        inline explicit constexpr operator uint32_t() const noexcept { return m_Type << 27 | m_Version << 12 | m_Index; }
        inline explicit constexpr operator bool() const noexcept { return !null(); }

        inline constexpr bool operator==(const Handle& other) const noexcept { 
            return (m_Index == other.m_Index && m_Version == other.m_Version && m_Type == other.m_Type);
        }

        [[nodiscard]] inline constexpr bool null() const noexcept { return (m_Index == 0 && m_Version == 0 && m_Type == 0); }

        template<typename TResource> [[nodiscard]] inline operator TResource*() const noexcept { return get<TResource>(); }

        template<typename TResource> 
        [[nodiscard]] inline TResource* get() const noexcept {
            TResource* ret = static_cast<TResource*>(internal_get_base());
            ASSERT((ret != nullptr && !null()) || (ret == nullptr && null()), __FUNCTION__ << "(): a non-null handle returned a null resource!");
            return ret;
        }
        template<typename TResource>
        [[nodiscard]] inline TResource* getThreadSafe() noexcept {
            TResource* ret = static_cast<TResource*>(internal_get_base_thread_safe());
            ASSERT((ret != nullptr && !null()) || (ret == nullptr && null()), __FUNCTION__ << "(): a non-null handle returned a null resource!");
            return ret;
        }

        [[nodiscard]] Engine::view_ptr<std::mutex> getMutex() noexcept;
};

template<class TResource>
struct LoadedResource final {
    Engine::view_ptr<TResource> m_Resource = nullptr;
    Handle                      m_Handle   = Handle{};
};

#endif