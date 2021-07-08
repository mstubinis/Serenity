#pragma once
#ifndef ENGINE_RESOURCES_RESOURCE_MODULE_H
#define ENGINE_RESOURCES_RESOURCE_MODULE_H

namespace Engine::priv {
    class ResourceManager;
};

#include <serenity/resources/ResourceVector.h>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>

namespace Engine::priv {
    class ResourceModule final {
        friend class Engine::priv::ResourceManager;
        private:
            static std::atomic<uint32_t>                   m_RegisteredResources;
            std::vector<std::unique_ptr<IResourceVector>>  m_Resources;
            mutable std::mutex                             m_Mutex;
        public:
            ResourceModule() = default;
            ResourceModule(const ResourceModule&)            = delete;
            ResourceModule& operator=(const ResourceModule&) = delete;
            ResourceModule(ResourceModule&&) noexcept        = delete;
            ResourceModule& operator=(ResourceModule&&)      = delete;

            // Locks the resource module from modifying the underlying resource containers. 
            // It will also reduce the memory footprint of them by calling shrink_to_fit().
            void lock() noexcept { for (auto& itr : m_Resources) { itr->lock(); } }
            // Unlocks the resource module so the underlying resource containers can be modified again.
            void unlock() noexcept { for (auto& itr : m_Resources) { itr->unlock(); } }

            [[nodiscard]] inline std::mutex& getMutex(const Handle inHandle) noexcept { return m_Resources[inHandle.type() - 1]->getMutex(); }

            template<class TResource>
            uint32_t registerResourceTypeID() noexcept {
                if (TResource::TYPE_ID == 0) {
                    TResource::TYPE_ID = ++m_RegisteredResources;
                    m_Resources.emplace_back(std::make_unique<Engine::priv::ResourceVector<TResource>>());
                }
                return TResource::TYPE_ID - 1;
            }
            template<class TResource> [[nodiscard]] inline uint32_t getResourceTypeID() const noexcept {
                return TResource::TYPE_ID - 1;
            }
            template<typename TResource>
            [[nodiscard]] LoadedResource<TResource> get(const std::string_view sv) noexcept {
                if (sv.empty()) {
                    return {};
                }
                const uint32_t typeIndex           = getResourceTypeID<TResource>();
                LoadedResource<TResource> ret_data = static_cast<ResourceVector<TResource>*>(m_Resources[typeIndex].get())->get(sv);
                return ret_data;
            }
            template<typename TResource>
            [[nodiscard]] Engine::view_ptr<TResource> get(const Handle inHandle) noexcept {
                const uint32_t typeIndex = getResourceTypeID<TResource>();
                return static_cast<ResourceVector<TResource>*>(m_Resources[typeIndex].get())->get(inHandle);
            }
            [[nodiscard]] inline void get(void*& out, const Handle inHandle) const noexcept {
                if (inHandle.null()) {
                    out = nullptr;
                    return;
                }
                m_Resources[inHandle.type() - 1]->get(out, inHandle);
            }
            template<typename TResource, typename ... ARGS>
            [[nodiscard]] Handle emplace(ARGS&&... args) {
                const uint32_t typeIndex = registerResourceTypeID<TResource>();
                const uint32_t index     = static_cast<const uint32_t>(static_cast<ResourceVector<TResource>*>(m_Resources[typeIndex].get())->emplace_back(std::forward<ARGS>(args)...));
                return Handle( index, 0, typeIndex + 1 );
            }
            template<typename TResource>
            [[nodiscard]] Handle push(TResource&& inResource) {
                const uint32_t typeIndex = registerResourceTypeID<TResource>();
                const uint32_t index     = (const uint32_t)static_cast<ResourceVector<TResource>*>(m_Resources[typeIndex].get())->push_back(std::move(inResource));
                return Handle( index, 0, typeIndex + 1 );
            }
            template<typename TResource>
            [[nodiscard]] std::list<Engine::view_ptr<TResource>> getAllResourcesOfType() {
                const uint32_t typeIndex = getResourceTypeID<TResource>();
                return static_cast<ResourceVector<TResource>*>(m_Resources[typeIndex].get())->getAsList();
            }
    };
};

#endif