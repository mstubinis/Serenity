#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include "ecs/SparseSet.h"
#include "ecs/EntitySerialization.h"
#include "core/Scene.h"

namespace Engine {
    namespace epriv {
        template<typename TEntity> class ECSEntityPool final{
            friend struct Engine::epriv::InternalScenePublicInterface;
            private:
                std::vector<EntityPOD>    _pool;
                std::vector<uint>         _freelist;
            public:
                ECSEntityPool() = default;
                ~ECSEntityPool() = default;

                void destroyFlaggedEntity(uint& i) {
                    const uint& index = i - 1;
                    ++_pool[index].versionID;
                    _freelist.emplace_back(index);
                }
                TEntity addEntity(Scene& _scene) {
                    if (_freelist.empty()) {
                        _pool.emplace_back(0,0);
                        _freelist.emplace_back(_pool.size() - 1);
                    }
                    uint _id = _freelist.back();
                    _freelist.pop_back();
                    auto& element = _pool[_id];
                    element.ID = _id + 1;
                    element.sceneID = _scene.id();
                    return TEntity(element.ID, element.sceneID, element.versionID);
                }
                EntityPOD* getEntity(const uint& _entityData) {
                    if (_entityData == 0) return nullptr;
                    EntitySerialization _s(_entityData);
                    const uint& index = _s.ID - 1;
                    if (index < _pool.size() && _pool[index].versionID == _s.versionID) {
                        return &_pool[index];
                    }
                    return nullptr;
                }
                EntityPOD* getEntity(TEntity& _entity) { return getEntity(_entity.data); }
            };
    };
};

#endif