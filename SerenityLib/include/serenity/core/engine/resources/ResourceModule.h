#pragma once
#ifndef ENGINE_RESOURCES_RESOURCE_MODULE_H
#define ENGINE_RESOURCES_RESOURCE_MODULE_H

#include <serenity/core/engine/resources/ResourceVector.h>
#include <serenity/core/engine/containers/TypeRegistry.h>
#include <mutex>
#include <vector>
#include <memory>

namespace Engine::priv {
    class ResourceModule final {
        private:
            Engine::type_registry                                        m_ResourceRegistry;
            std::vector<std::unique_ptr<Engine::priv::IResourceVector>>  m_Resources;
            mutable std::mutex                                           m_Mutex;
        public:
            ResourceModule() = default;
            ResourceModule(const ResourceModule&)            = delete;
            ResourceModule& operator=(const ResourceModule&) = delete;
            ResourceModule(ResourceModule&&) noexcept        = delete;
            ResourceModule& operator=(ResourceModule&&)      = delete;

            // Locks the resource module from modifying the underlying resource containers. 
            // It will also reduce the memory footprint of them by calling shrink_to_fit().
            void lock() noexcept {
                for (auto& itr : m_Resources) {
                    itr->lock();
                }
            }
            // Unlocks the resource module so the underlying resource containers can be modified again.
            void unlock() noexcept {
                for (auto& itr : m_Resources) {
                    itr->unlock();
                }
            }

            inline Engine::view_ptr<std::mutex> getMutex(const Handle inHandle) noexcept { 
                return m_Resources[inHandle.type() - 1]->getMutex(); 
            }

            template<typename TResource>
            uint32_t registerResourceType() {
                const uint32_t index = m_ResourceRegistry.type_slot<TResource>();
                if (index == m_Resources.size()) {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.emplace_back(std::make_unique<Engine::priv::ResourceVector<TResource>>());
                }
                return index;
            }
            template<typename TResource>
            uint32_t registerResourceTypeThreadSafe() {
                uint32_t index;
                {
                    std::lock_guard lock{ m_Mutex };
                    index = m_ResourceRegistry.type_slot<TResource>();
                    if (index == m_Resources.size()) {
                        m_Resources.emplace_back(std::make_unique<Engine::priv::ResourceVector<TResource>>());
                    }
                }
                return index;
            }



            template<typename TResource>
            std::pair<Engine::view_ptr<TResource>, Handle> get(const std::string_view sv) noexcept {
                if (sv.empty()) {
                    return std::make_pair(nullptr, Handle{});
                }
                using collectionType     = Engine::priv::ResourceVector<TResource>*;
                const uint32_t typeIndex = m_ResourceRegistry.type_slot_fast<TResource>();
                std::pair<Engine::view_ptr<TResource>, Handle> returned_pair = static_cast<collectionType>(m_Resources[typeIndex].get())->get(sv);
                returned_pair.second.m_Type = typeIndex + 1;
                return returned_pair;
            }

            template<typename TResource>
            Engine::view_ptr<TResource> get(const Handle inHandle) noexcept {
                using collectionType     = Engine::priv::ResourceVector<TResource>*;
                const uint32_t typeIndex = m_ResourceRegistry.type_slot_fast<TResource>();
                return static_cast<collectionType>(m_Resources[typeIndex].get())->get(inHandle);
            }
            inline void get(void*& out, const Handle inHandle) const noexcept {
                if (inHandle.null()) {
                    out = nullptr;
                    return;
                }
                m_Resources[inHandle.type() - 1]->get(out, inHandle);
            }

            template<typename TResource, typename ... ARGS>
            Handle emplace(ARGS&&... args) {
                using collectionType     = Engine::priv::ResourceVector<TResource>*;
                const uint32_t typeIndex = registerResourceType<TResource>();
                const uint32_t index     = (const uint32_t)static_cast<collectionType>(m_Resources[typeIndex].get())->emplace_back(std::forward<ARGS>(args)...);
                return Handle( index, 0, typeIndex + 1 );
            }

            template<typename TResource>
            Handle push(TResource&& inResource) {
                using collectionType     = Engine::priv::ResourceVector<TResource>*;
                const uint32_t typeIndex = registerResourceType<TResource>();
                const uint32_t index     = (const uint32_t)static_cast<collectionType>(m_Resources[typeIndex].get())->push_back(std::move(inResource));
                return Handle( index, 0, typeIndex + 1 );
            }

            template<typename TResource>
            std::list<Engine::view_ptr<TResource>> getAllResourcesOfType() {
                using collectionType     = Engine::priv::ResourceVector<TResource>*;
                const uint32_t typeIndex = m_ResourceRegistry.type_slot_fast<TResource>();
                return static_cast<collectionType>(m_Resources[typeIndex].get())->getAsList();
            }
    };
};

#endif