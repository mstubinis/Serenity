#pragma once
#ifndef ENGINE_RESOURCE_POOL_H
#define ENGINE_RESOURCE_POOL_H

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/containers/Freelist.h>

namespace Engine::priv{
    template<typename RESOURCE> 
    struct HandleEntry final {
        std::uint32_t   m_Version  = 1;
        RESOURCE*       m_Resource = nullptr;

        HandleEntry() = default;
        HandleEntry(RESOURCE* r) 
            : m_Resource(r) {
        }
        ~HandleEntry() {
            SAFE_DELETE(m_Resource);
        }

        HandleEntry(const HandleEntry& other)              = delete;
        HandleEntry& operator=(const HandleEntry& other)   = delete;
        HandleEntry(HandleEntry&& other) noexcept {
            m_Version  = std::move(other.m_Version);
            m_Resource = std::exchange(other.m_Resource, nullptr);
        }
        HandleEntry& operator=(HandleEntry&& other) noexcept {
            if (&other != this) {
                m_Version  = std::move(other.m_Version);
                m_Resource = std::exchange(other.m_Resource, nullptr);
            }
            return *this;
        }

    };

    template<typename T> 
    class ResourcePool final: public freelist<HandleEntry<T>>{
        using super = freelist<HandleEntry<T>>;
        private:
            std::mutex m_Mutex;
        public:
            ResourcePool(unsigned int numEntries) 
                : freelist<HandleEntry<T>>(numEntries){
            }
            ~ResourcePool() = default;
            Handle add(T* ptr, unsigned int type) {
                int used_index = -1;
                {
                    std::lock_guard lock(m_Mutex);
                    used_index = super::emplace_back(ptr);
                }
                if (used_index == -1) {
                    return Handle();
                }
                auto& item = super::get(used_index);
                ++item.m_Version;
                if (item.m_Version == 0) {
                    item.m_Version = 1;
                }
                return Handle(used_index + 1, item.m_Version, type);
            }
            T* get(Handle handle) noexcept {
                T* outPtr = nullptr;
                return get(handle, outPtr);
            }
            bool get(Handle handle, T*& outPtr) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                if (item.m_Version != handle.version()){
                    outPtr = nullptr;
                    return false;
                }
                outPtr = item.m_Resource;
                return true;
            }
            template<typename RESOURCE> 
            inline bool getAs(Handle handle, RESOURCE*& outPtr) noexcept {
                T* void_ = nullptr;
                bool rv = get(handle, void_);
                outPtr = reinterpret_cast<RESOURCE*>(void_);
                return rv;
            }
            template<typename RESOURCE>
            inline void getAsFast(Handle handle, RESOURCE*& outPtr) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                outPtr = reinterpret_cast<RESOURCE*>(item.m_Resource);
            }
            template<typename RESOURCE>
            inline RESOURCE* getAsFast(Handle handle) noexcept {
                const auto& item = super::get(handle.index() - 1U);
                return reinterpret_cast<RESOURCE*>(item.m_Resource);
            }
            template<typename RESOURCE>
            inline RESOURCE* getAsFast(unsigned int index) noexcept {
                const auto& item = super::get(index - 1U);
                return reinterpret_cast<RESOURCE*>(item.m_Resource);
            }
    };
};


#endif