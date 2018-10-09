#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include "ecs/SparseSet.h"

class Scene;
namespace Engine {
    namespace epriv {

        struct IDObject {
            uint ID;
            IDObject() :ID(0) {}
            IDObject(uint _id):ID(_id){}
        };

        template<typename TEntity> class ECSEntityPool final : public SparseSet<IDObject, TEntity> {
            using super = SparseSet<IDObject, TEntity>;
            public:
                ECSEntityPool() {}
                ~ECSEntityPool() {}

                TEntity* addEntity(Scene& _scene) {
                    IDObject _id(super::pool().size() + 1);
                    return super::_add(_id, _id.ID, _scene);
                }
                bool removeEntity(const TEntity& _entity) {
                    return super::_remove(_entity);
                }
                TEntity* getEntity(const TEntity& _entity) {
                    return super::_get(_entity);
                }
                void moveEntity(ECSEntityPool<TEntity>& other, uint _entityID) {
                    //TEntity& e = pool[_entityID - 1];
                    //other.addEntity(e);
                    //removeEntity(e);
                }
                void moveEntity(ECSEntityPool<TEntity>& other, TEntity& _entity) { moveEntity(other, _entity.ID); }
        };
    };
};

#endif