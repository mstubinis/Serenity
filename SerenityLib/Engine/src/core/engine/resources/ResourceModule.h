#pragma once
#ifndef ENGINE_RESOURCES_RESOURCE_MODULE_H
#define ENGINE_RESOURCES_RESOURCE_MODULE_H

#include <core/engine/containers/TypeRegistry.h>
#include <core/engine/resources/ResourceVector.h>

namespace Engine::priv {
    class ResourceModule final {
        private:
            Engine::type_registry                                        m_ResourceRegistry;
            std::vector<std::unique_ptr<Engine::priv::IResourceVector>>  m_Resources;
            std::mutex                                                   m_Mutex;
        public:
            ResourceModule() = default;
            ResourceModule(const ResourceModule& other)            = delete;
            ResourceModule& operator=(const ResourceModule& other) = delete;
            ResourceModule(ResourceModule&& other) noexcept        = delete;
            ResourceModule& operator=(ResourceModule&& other)      = delete;
            ~ResourceModule() = default;

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

            template<typename TResource>
            std::uint32_t registerResourceType() {
                std::uint32_t index = (const std::uint32_t)m_ResourceRegistry.type_slot<TResource>();
                if (index == m_Resources.size()) {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.emplace_back(std::make_unique<Engine::priv::ResourceVector<TResource>>());
                }
                return index;
            }
            template<typename TResource>
            std::uint32_t registerResourceTypeThreadSafe() {
                std::uint32_t index;
                {
                    std::lock_guard lock{ m_Mutex };
                    index = (const std::uint32_t)m_ResourceRegistry.type_slot<TResource>();
                    if (index == m_Resources.size()) {
                        m_Resources.emplace_back(std::make_unique<Engine::priv::ResourceVector<TResource>>());
                    }
                }
                return index;
            }



            template<typename TResource>
            std::pair<TResource*, Handle> get(const std::string_view sv) noexcept {
                if (sv.empty()) {
                    return std::make_pair(nullptr, Handle{});
                }
                using collectionType                        = Engine::priv::ResourceVector<TResource>*;
                //const std::uint32_t typeIndex               = (const std::uint32_t)registerResourceType<TResource>();
                const std::uint32_t typeIndex               = (const std::uint32_t)m_ResourceRegistry.type_slot_fast<TResource>();
                std::pair<TResource*, Handle> returned_pair = static_cast<collectionType>(m_Resources[typeIndex].get())->get(sv);
                returned_pair.second.m_Type = typeIndex + 1;
                return returned_pair;
            }

            template<typename TResource>
            TResource* get(const Handle inHandle) noexcept {
                using collectionType          = Engine::priv::ResourceVector<TResource>*;
                //const std::uint32_t typeIndex = (const std::uint32_t)registerResourceType<TResource>();
                const std::uint32_t typeIndex = (const std::uint32_t)m_ResourceRegistry.type_slot_fast<TResource>();
                return static_cast<collectionType>(m_Resources[typeIndex].get())->get(inHandle);
            }
            inline void get(void*& out, const Handle inHandle) const noexcept {
                m_Resources[inHandle.type() - 1]->get(out, inHandle);
            }

            template<typename TResource, typename ... ARGS>
            Handle emplace(ARGS&&... args) {
                using collectionType           = Engine::priv::ResourceVector<TResource>*;
                const std::uint32_t typeIndex  = (const std::uint32_t)registerResourceType<TResource>();
                const std::uint32_t index      = (const std::uint32_t)static_cast<collectionType>(m_Resources[typeIndex].get())->emplace_back(std::forward<ARGS>(args)...);
                return Handle( index, 0, typeIndex + 1 );
            }
            template<typename TResource>
            Handle push(TResource&& inResource) {
                using collectionType           = Engine::priv::ResourceVector<TResource>*;
                const std::uint32_t typeIndex  = (const std::uint32_t)registerResourceType<TResource>();
                const std::uint32_t index      = (const std::uint32_t)static_cast<collectionType>(m_Resources[typeIndex].get())->push_back(std::move(inResource));
                return Handle( index, 0, typeIndex + 1 );
            }

            template<typename TResource>
            std::list<TResource*> getAllResourcesOfType() {
                using collectionType           = Engine::priv::ResourceVector<TResource>*;
                const std::uint32_t typeIndex  = (const std::uint32_t)m_ResourceRegistry.type_slot_fast<TResource>();
                return static_cast<collectionType>(m_Resources[typeIndex].get())->getAsList();
            }
    };
};

#endif