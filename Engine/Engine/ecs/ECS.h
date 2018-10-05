#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

#include "ECSRegistry.h"
#include "ECSEntityPool.h"
#include "ECSComponentPool.h"
#include <memory>

namespace Engine {
    namespace epriv {
        template<typename TEntity> class ECS final {
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
                void removeEntity(TEntity& _entity) { removeEntity(_entity.ID); }


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
