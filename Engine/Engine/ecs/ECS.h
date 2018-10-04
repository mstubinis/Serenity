#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

#include "ECSRegistry.h"
#include "ECSComponentPool.h"
#include <memory>

#include <iostream>

class Scene;
namespace Engine {
    namespace epriv {
        template<typename TEntity>
        class ECSEntityPool final {
            private:
                uint lastIndex;
                std::vector<TEntity> pool;
            public:
                ECSEntityPool():lastIndex(0) {}
                ~ECSEntityPool(){ lastIndex = 0; pool.clear(); }
                TEntity* getEntity(uint _entityID) {
                    if (_entityID == 0) return nullptr;
                    return &pool[_entityID - 1];
                }
                TEntity* createEntity(Scene& _scene) {
                    TEntity e = TEntity(lastIndex + 1, _scene);
                    addEntity(e);
                    return &pool[pool.size() - 1];
                }
                void addEntity(const TEntity& _entity) {
                    pool.push_back(std::move(_entity));
                    ++lastIndex;
                }
                void removeEntity(uint _entityID) {
                    uint indexToRemove = _entityID - 1;
                    if (indexToRemove != lastIndex) {
                        std::swap(pool[indexToRemove], pool[lastIndex]);
                    }
                    TEntity& e = pool[pool.size()];
                    /*
                    for(auto system : componentSystems){
                        system.removeComponent(this);
                    }
                    */
                    pool.pop_back();
                    --lastIndex;
                }
                void removeEntity(TEntity& _entity) { removeEntity(_entity.ID); }
                void moveEntity(ECSEntityPool<TEntity>& other, uint _entityID) {
                    TEntity& e = pool[_entityID - 1];
                    other.addEntity(e);
                    removeEntity(e);
                }
                void moveEntity(ECSEntityPool<TEntity>& other, TEntity& _entity) { moveEntity(other, _entity.ID); }
        };
        //per scene basis
        //the hub of the ECS system. have a pool for entities here as well as the component pools here too
        //note: component pools have: a sparse vector of entity id's (uint) and a dense vector of components (T)
        //essentially, each component pool has a list of entites mapped to the components of a particular type
        template<typename TEntity>
        class ECS final {
            private:
                ECSEntityPool<TEntity>                                    entityPool;
                std::vector<std::unique_ptr<ECSComponentPool<TEntity>>>   componentPools;

                //builds a component pool for the component type if it is not built already.
                template<typename TComponent> void buildPool(uint type_slot) {
                    if (componentPools[type_slot]) return;
                    if (!(type_slot < componentPools.size()))
                        componentPools.resize(type_slot + 1);
                    if (!componentPools[type_slot])
                        componentPools[type_slot] = std::make_unique<ECSComponentPool<TEntity, TComponent>>();
                }

            public:
                ECS() = default;
                ~ECS() = default;

                TEntity* createEntity(Scene& _scene) { return entityPool.createEntity(_scene); }
                void removeEntity(uint _entityID) { entityPool.removeEntity(_entityID); }
                void removeEntity(TEntity& _entity) { ECS::removeEntity(_entity.ID); }


                template<typename TComponent> TComponent* addComponent(TEntity& _entity) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    return componentPools[type_slot]->addComponent<TComponent>(_entity.ID);
                }
                template<typename TComponent> void removeComponent(TEntity& _entity) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    componentPools[type_slot]->removeComponent<TComponent>(_entity.ID);
                }
                template<typename TComponent> TComponent* getComponent(TEntity& _entity) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    return componentPools[type_slot]->getComponent<TComponent>(_entity.ID);
                }
        };
    };
};

#endif
