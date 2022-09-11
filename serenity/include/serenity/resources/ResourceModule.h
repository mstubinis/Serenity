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
            std::vector<IResourceVector*>  m_ResourceVectors;
            static std::atomic<uint32_t>   m_RegisteredResources;
            mutable std::mutex             m_Mutex;
        private:
            template<class RESOURCE>
            [[nodiscard]] inline uint32_t getResourceTypeID() const noexcept {
                return RESOURCE::TYPE_ID - 1;
            }
            template<class RESOURCE>
            [[nodiscard]] inline ResourceVector<RESOURCE>* getResourceVector() noexcept {
                return static_cast<ResourceVector<RESOURCE>*>(m_ResourceVectors[getResourceTypeID<RESOURCE>()]);
            }
        public:
            ResourceModule() = default;
            ResourceModule(const ResourceModule&)            = delete;
            ResourceModule& operator=(const ResourceModule&) = delete;
            ResourceModule(ResourceModule&&) noexcept        = delete;
            ResourceModule& operator=(ResourceModule&&)      = delete;

            ~ResourceModule() {
                SAFE_DELETE_VECTOR(m_ResourceVectors);
            }

            [[nodiscard]] inline std::mutex& getMutex(const Handle inHandle) noexcept { return m_ResourceVectors[inHandle.type() - 1]->getMutex(); }

            template<class RESOURCE>
            uint32_t registerResource() noexcept {
                if (RESOURCE::TYPE_ID == 0) {
                    RESOURCE::TYPE_ID = ++m_RegisteredResources;
                    m_ResourceVectors.push_back(NEW Engine::priv::ResourceVector<RESOURCE>{});
                }
                return RESOURCE::TYPE_ID - 1;
            }
            template<class RESOURCE>
            [[nodiscard]] LoadedResource<RESOURCE> get(const std::string_view sv) noexcept {
                if (sv.empty()) {
                    return {};
                }
                LoadedResource<RESOURCE> ret_data = getResourceVector<RESOURCE>()->get(sv);
                return ret_data;
            }
            template<class RESOURCE, class INTEGRAL>
            requires (std::is_integral_v<INTEGRAL>)
            [[nodiscard]] Engine::view_ptr<RESOURCE> get(const INTEGRAL index) noexcept {
                return getResourceVector<RESOURCE>()->get(index);
            }
            template<class INTEGRAL>
            requires (std::is_integral_v<INTEGRAL>)
            [[nodiscard]] void* getVoid(const INTEGRAL resourceTypeID, const INTEGRAL index) noexcept {
                return m_ResourceVectors[resourceTypeID - 1]->getVoid(index);
            }
            [[nodiscard]] inline void* getVoid(const Handle inHandle) noexcept {
                return !inHandle.null() ? m_ResourceVectors[inHandle.type() - 1]->getVoid(inHandle.index()) : nullptr;
            }

            template<class RESOURCE, class ... ARGS>
            [[nodiscard]] Handle emplace(ARGS&&... args) {
                const uint32_t typeIndex = registerResource<RESOURCE>();
                const uint32_t index     = uint32_t(getResourceVector<RESOURCE>()->emplace_back(std::forward<ARGS>(args)...));
                return Handle( index, typeIndex + 1 );
            }
            template<class RESOURCE>
            [[nodiscard]] Handle push(RESOURCE&& inResource) {
                const uint32_t typeIndex = registerResource<RESOURCE>();
                const uint32_t index     = uint32_t(getResourceVector<RESOURCE>()->push_back(std::move(inResource)));
                return Handle( index, typeIndex + 1 );
            }
            template<class RESOURCE>
            [[nodiscard]] inline std::vector<Engine::view_ptr<RESOURCE>> getAllResourcesOfType() noexcept {
                return getResourceVector<RESOURCE>()->getAsVector();
            }
    };
};

#endif