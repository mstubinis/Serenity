#pragma once
#ifndef ENGINE_RESOURCE_POOL_H
#define ENGINE_RESOURCE_POOL_H

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/containers/Freelist.h>

namespace Engine::priv{
    template<typename RESOURCE> 
    struct HandleEntry final {
        std::uint32_t             m_Version{ 1 };
        std::unique_ptr<RESOURCE> m_Resource;

        HandleEntry() = default;
        HandleEntry(RESOURCE* r) 
            : m_Resource{ r } 
        {}
        HandleEntry(const HandleEntry& other)                = delete;
        HandleEntry& operator=(const HandleEntry& other)     = delete;
        HandleEntry(HandleEntry&& other) noexcept            = default;
        HandleEntry& operator=(HandleEntry&& other) noexcept = default;
        ~HandleEntry() = default;
    };

    template<typename RESOURCE_INTERFACE> 
    class ResourcePool final: public freelist<HandleEntry<RESOURCE_INTERFACE>>{
        using super = freelist<HandleEntry<RESOURCE_INTERFACE>>;
        private:
            std::mutex m_Mutex;
        public:
            ResourcePool(unsigned int numEntries) 
                : freelist<HandleEntry<RESOURCE_INTERFACE>>{ numEntries } 
            {}
            ~ResourcePool() = default;
            Handle add(RESOURCE_INTERFACE* ptr, unsigned int type) {
                int used_index{ -1 };
                {
                    std::lock_guard lock{ m_Mutex };
                    used_index = super::emplace_back(ptr);
                }
                if (used_index == -1) {
                    return Handle{};
                }
                auto& item = super::get(used_index);
                ++item.m_Version;
                if (item.m_Version == 0) {
                    item.m_Version = 1;
                }
                return Handle{ (std::uint32_t)used_index + 1U, item.m_Version, type };
            }
            RESOURCE_INTERFACE* get(Handle handle) noexcept {
                RESOURCE_INTERFACE* outPtr{ nullptr };
                return get(handle, outPtr);
            }
            bool get(Handle handle, RESOURCE_INTERFACE*& outPtr) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                if (item.m_Version != handle.version()){
                    outPtr = nullptr;
                    return false;
                }
                outPtr = item.m_Resource.get();
                return true;
            }
            template<typename RESOURCE> 
            inline bool getAs(Handle handle, RESOURCE*& outPtr) noexcept {
                RESOURCE_INTERFACE* void_ = nullptr;
                bool rv = get(handle, void_);
                outPtr = reinterpret_cast<RESOURCE*>(void_);
                return rv;
            }
            template<typename RESOURCE>
            inline void getAsFast(Handle handle, RESOURCE*& outPtr) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                outPtr = reinterpret_cast<RESOURCE*>(item.m_Resource.get());
            }
            template<typename RESOURCE>
            inline RESOURCE* getAsFast(Handle handle) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                return reinterpret_cast<RESOURCE*>(item.m_Resource.get());
            }
            template<typename RESOURCE>
            inline RESOURCE* getAsFast(unsigned int index) noexcept {
                const auto& item = super::get(index - 1U);
                return reinterpret_cast<RESOURCE*>(item.m_Resource.get());
            }
    };
};


#endif