#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include <ecs/EntityDataRequest.h>
#include <core/engine/scene/Scene.h>

namespace Engine::priv {
    template<typename TEntity> 
    class ECSEntityPool final{
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            std::vector<EntityPOD>       m_Pool;
            std::vector<unsigned int>    m_Freelist;
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

            void destroyFlaggedEntity(const unsigned int& entityID) {
                const auto index = entityID - 1;
                ++m_Pool[index].versionID;
                m_Freelist.emplace_back(index);
            }
            TEntity addEntity(const Scene& scene) {
                if (m_Freelist.empty()) {
                    m_Pool.emplace_back(0, 0);
                    m_Freelist.emplace_back(static_cast<unsigned int>(m_Pool.size()) - 1U);
                }
                const auto id          = m_Freelist.back();
                m_Freelist.pop_back();
                EntityPOD& entityPOD   = m_Pool[id];
                entityPOD.ID           = id + 1;
                entityPOD.sceneID      = scene.id();
                TEntity entity         = TEntity(entityPOD.ID, entityPOD.sceneID, entityPOD.versionID);
                return std::move(entity);
            }
            EntityPOD* getEntity(const unsigned int& entityData) const {
                if (entityData == 0) {
                    return nullptr;
                }
                const EntityDataRequest dataRequest(entityData);
                const auto index = dataRequest.ID - 1;
                if (index < m_Pool.size() && m_Pool[index].versionID == dataRequest.versionID) {
                    EntityPOD* ret = &m_Pool[index];
                    return const_cast<EntityPOD*>(ret);
                }
                return nullptr;
            }
            EntityPOD* getEntity(const TEntity& entity) {
                return getEntity(entity.data);
            }
        };
};

#endif