#pragma once
#ifndef ENGINE_RESOURCE_POOL_H
#define ENGINE_RESOURCE_POOL_H

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/containers/Freelist.h>

namespace Engine::priv{
    template<typename TResource> 
    struct HandleEntry final{
        std::uint32_t   version  = 1;
        TResource*      resource = nullptr;

        HandleEntry() = default;
        HandleEntry(TResource* r){
            resource   = r;
        }
        ~HandleEntry() {
            SAFE_DELETE(resource);
        }

        HandleEntry(const HandleEntry&)                      = delete;
        HandleEntry& operator=(const HandleEntry&)           = delete;
        HandleEntry(HandleEntry&& other) noexcept {
            version  = std::move(other.version);
            resource = std::exchange(other.resource, nullptr);
        }
        HandleEntry& operator=(HandleEntry&& other) noexcept {
            if (&other != this) {
                version  = std::move(other.version);
                resource = std::exchange(other.resource, nullptr);
            }
            return *this;
        }

    };

    template<typename T> 
    class ResourcePool final: public freelist<HandleEntry<T>>{
        using super = freelist<HandleEntry<T>>;
        public:
            ResourcePool(const unsigned int numEntries) : freelist<HandleEntry<T>>(numEntries){

            }
            ~ResourcePool(){
            }
            Handle add(T* ptr, const unsigned int type){
                const int used_index = super::emplace_back(ptr);
                if (used_index == -1) {
                    return Handle();
                }
                auto& item = super::get(used_index);
                ++item.version;
                if (item.version == 0) {
                    item.version = 1;
                }
                return Handle(used_index + 1, item.version, type);
            }
            T* get(const Handle handle){
                T* outPtr = nullptr;
                return get(handle, outPtr);
            }
            const bool get(const Handle handle, T*& outPtr){
                const auto& item = super::get(handle.index - 1U);
                if (item.version != handle.version){
                    outPtr = nullptr;
                    return false;
                }
                outPtr = item.resource;
                return true;
            }
            template<typename TResource> 
            inline const bool getAs(const Handle handle, TResource*& outPtr){
                T* _void = nullptr;
                const bool rv = get(handle,_void);
                outPtr = reinterpret_cast<TResource*>(_void);
                return rv;
            }
            template<typename TResource> 
            inline void getAsFast(const Handle handle, TResource*& outPtr){
                const auto& item = super::get(handle.index - 1U);
                outPtr = reinterpret_cast<TResource*>(item.resource);
            }
            template<typename TResource> 
            inline TResource* getAsFast(const Handle handle){
                const auto& item = super::get(handle.index - 1U);
                return reinterpret_cast<TResource*>(item.resource);
            }
            template<typename TResource> 
            inline TResource* getAsFast(const unsigned int index){
                const auto& item = super::get(index - 1U);
                return reinterpret_cast<TResource*>(item.resource);
            }
    };
};


#endif