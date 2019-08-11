#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include <ecs/SparseSet.h>
#include <ecs/EntityDataRequest.h>
#include <core/engine/scene/Scene.h>

namespace Engine {
namespace epriv {
    template<typename E> class ECSEntityPool final{
        friend struct Engine::epriv::InternalScenePublicInterface;
        private:
            std::vector<EntityPOD>    _pool;
            std::vector<uint>         _freelist;
        public:
            ECSEntityPool() = default;
            ~ECSEntityPool() = default;

            void destroyFlaggedEntity(const uint& entityID) {
                const uint index = entityID - 1;
                ++_pool[index].versionID;
                _freelist.emplace_back(index);
            }
            E addEntity(Scene& scene) {
                if (_freelist.empty()) {
                    _pool.emplace_back(0,0);
                    _freelist.emplace_back(_pool.size() - 1);
                }
                const uint id = _freelist.back();
                _freelist.pop_back();
                auto& element = _pool[id];
                element.ID = id + 1;
                element.sceneID = scene.id();
                return E(element.ID, element.sceneID, element.versionID);
            }
            EntityPOD* getEntity(const uint& entityData) {
                if (entityData == 0) 
                    return nullptr;
                EntityDataRequest dataRequest(entityData);
                const uint index = dataRequest.ID - 1;
                if (index < _pool.size() && _pool[index].versionID == dataRequest.versionID) {
                    return &_pool[index];
                }
                return nullptr;
            }
            EntityPOD* getEntity(const E& entity) {
                return getEntity(entity.data);
            }
        };
};
};

#endif