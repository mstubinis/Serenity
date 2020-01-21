#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include <ecs/EntityDataRequest.h>
#include <core/engine/scene/Scene.h>

namespace Engine {
    namespace priv {
        template<typename TEntity> class ECSEntityPool final{
            friend struct Engine::priv::InternalScenePublicInterface;
            private:
                std::vector<EntityPOD>       _pool;
                std::vector<unsigned int>    _freelist;
            public:
                ECSEntityPool() {}
                ~ECSEntityPool() {}
                ECSEntityPool(const ECSEntityPool&)                      = delete;
                ECSEntityPool& operator=(const ECSEntityPool&)           = delete;
                ECSEntityPool(ECSEntityPool&& other) noexcept            = delete;
                ECSEntityPool& operator=(ECSEntityPool&& other) noexcept = delete;

                void destroyFlaggedEntity(const unsigned int& entityID) {
                    const auto index = entityID - 1;
                    ++_pool[index].versionID;
                    _freelist.emplace_back(index);
                }
                TEntity addEntity(const Scene& scene) {
                    if (_freelist.empty()) {
                        _pool.emplace_back(0, 0);
                        _freelist.emplace_back(static_cast<unsigned int>(_pool.size()) - 1U);
                    }
                    const auto id = _freelist.back();
                    _freelist.pop_back();
                    EntityPOD& element = _pool[id];
                    element.ID = id + 1;
                    element.sceneID = scene.id();
                    TEntity entity = TEntity(element.ID, element.sceneID, element.versionID);
                    return std::move(entity);
                }
                EntityPOD* getEntity(const unsigned int& entityData) {
                    if (entityData == 0) {
                        return nullptr;
                    }
                    const EntityDataRequest dataRequest(entityData);
                    const auto index = dataRequest.ID - 1;
                    if (index < _pool.size() && _pool[index].versionID == dataRequest.versionID) {
                        return &_pool[index];
                    }
                    return nullptr;
                }
                EntityPOD* getEntity(const TEntity& entity) {
                    return getEntity(entity.data);
                }
            };
    };
};

#endif