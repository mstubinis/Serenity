#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include "ecs/SparseSet.h"
#include "ecs/EntitySerialization.h"
#include "core/Scene.h"


namespace Engine {
    namespace epriv {

        struct EntityPOD;

        template<typename TEntity> class ECSEntityPool final{

            private:
                std::vector<EntityPOD>    _pool;
                std::vector<uint>         _freelist;
            public:
                ECSEntityPool() = default;
                ~ECSEntityPool() = default;

                TEntity addEntity(Scene& _scene) {
                    if (_freelist.empty()) {
                        _pool.emplace_back(0);
                        _freelist.emplace_back(_pool.size() - 1);
                    }
                    uint _id = _freelist.back();
                    _freelist.pop_back();
                    auto& element = _pool[_id];
                    element.ID = _id + 1;
                    element.sceneID = _scene.id();
                    return TEntity(element.ID, element.sceneID, _pool[_id].versionID);
                }
                bool removeEntity(const uint& _id) {
                    EntitySerialization _s(_id);
                    uint index = _s.ID;
                    ++_pool[index].versionID;
                    _freelist.push_back(index);
                }
                bool removeEntity(const TEntity& _entity) { return removeEntity(_entity.data); }
                EntityPOD* getEntity(const uint& _id) {
                    EntitySerialization _s(_id);
                    if (_s.ID < _pool.size() && _pool[_s.ID].versionID == _s.versionID) {
                        return &_pool[_s.ID];
                    }
                    return nullptr;
                }
                EntityPOD* getEntity(const TEntity& _entity) { return getEntity(_entity.data); }
            };
    };
};

#endif