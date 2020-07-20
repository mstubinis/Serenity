#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include <core/engine/scene/Scene.h>

namespace Engine::priv {
    template<typename ENTITY> 
    class ECSEntityPool final{
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            std::vector<ENTITY>          m_Pool;
            std::vector<std::uint32_t>   m_Freelist;
        public:
            ECSEntityPool() {
                m_Pool.reserve(5000);
                m_Freelist.reserve(5000);
            }
            ~ECSEntityPool() {}
            ECSEntityPool(const ECSEntityPool&)                      = delete;
            ECSEntityPool& operator=(const ECSEntityPool&)           = delete;
            ECSEntityPool(ECSEntityPool&& other) noexcept            = delete;
            ECSEntityPool& operator=(ECSEntityPool&& other) noexcept = delete;

            constexpr bool isEntityVersionDifferent(ENTITY entity) const {
                auto index = entity.id() - 1U;
                ENTITY storedEntity = m_Pool[index];
                return (storedEntity.versionID() != entity.versionID());
            }

            void destroyFlaggedEntity(std::uint32_t entityID) {
                auto index = entityID - 1U;
                ENTITY storedEntity = m_Pool[index];
                std::uint32_t storedVersion = storedEntity.versionID();
                ++storedVersion;
                ENTITY updatedEntity(storedEntity.id(), storedEntity.sceneID(), storedVersion);
                m_Pool[index] = std::move(updatedEntity);
                m_Freelist.emplace_back(index);
            }
            constexpr ENTITY addEntity(const Scene& scene) {
                if (m_Freelist.empty()) {
                    m_Pool.emplace_back(0U, 0U, 0U);
                    m_Freelist.emplace_back(static_cast<std::uint32_t>(m_Pool.size()) - 1U);
                }
                auto id                 = m_Freelist.back();
                ENTITY old              = m_Pool[id];
                ENTITY entity           = ENTITY(id + 1U, scene.id(), old.versionID());
                m_Freelist.pop_back();
                m_Pool[id] = entity;
                return std::move(entity);
            }
            constexpr ENTITY getEntity(std::uint32_t entityData) const {
                if (entityData == 0) {
                    return nullptr;
                }
                auto index = ENTITY::id(entityData) - 1U;
                if (index < m_Pool.size()) {
                    ENTITY e = m_Pool[index];
                    if (e.versionID() == ENTITY::versionID(entityData)) {
                        return e;
                    }
                    //else {

                    //}
                }
                return ENTITY();
            }
        };
};

#endif