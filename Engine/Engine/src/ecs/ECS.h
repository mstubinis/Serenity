#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

#include "ecs/ECSRegistry.h"
#include "ecs/ECSEntityPool.h"
#include "ecs/ECSComponentPool.h"
#include "ecs/ECSSystem.h"
#include <memory>

namespace Engine {
    namespace epriv {
        template<typename TEntity> class ECS final {
            private:
                ECSEntityPool<TEntity>                                    entityPool;
                std::vector<std::unique_ptr<ECSComponentPool<TEntity>>>   componentPools;
                std::vector<std::unique_ptr<ECSSystem<TEntity>>>          systems;

                //builds a component pool and system for the component type if it is not built already.
                template<typename TComponent> void buildPool(uint type_slot) {
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (componentPools[type_slot]) return;
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<ECSComponentPool<TEntity, TComponent>>();
                    }
                    if (!systems[type_slot]) {
                        using CPoolType = ECSComponentPool<TEntity, TComponent>;
                        auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                        systems[type_slot] = std::make_unique<ECSSystem<TEntity, TComponent>>(cPool);
                    }
                }

            public:
                ECS() = default;
                ~ECS() = default;
                ECS(const ECS&) = delete;            // non construction-copyable
                ECS& operator=(const ECS&) = delete; // non copyable

                template<typename TComponent> void assignSystem(ECSSystem<TEntity>& _system) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    if (!(type_slot < componentPools.size())) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<ECSComponentPool<TEntity, TComponent>>();
                    }
                    if (!(type_slot < systems.size())) {
                        systems.resize(type_slot + 1);
                    }
                    if (systems[type_slot]) {
                        systems[type_slot].reset();
                    }
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    systems[type_slot] = std::unique_ptr<ECSSystem<TEntity, TComponent>>(_system);
                }


                //we may or may not need these...
                TEntity* createEntity(Scene& _scene) { return entityPool.createEntity(_scene); }
                void removeEntity(uint _entityID) { entityPool.removeEntity(_entityID); }
                void removeEntity(TEntity& _entity) { entityPool.removeEntity(_entity.ID); }
                TEntity* getEntity(uint _entityID) { return entityPool.getEntity(_entityID); }
                void moveEntity(ECSEntityPool<TEntity>& other, uint _entityID) { entityPool.moveEntity(other, _entityID); }
                void moveEntity(ECSEntityPool<TEntity>& other, TEntity& _entity) { entityPool.moveEntity(other, _entity.ID); }


                template<typename TComponent, typename... ARGS> TComponent* addComponent(TEntity& _entity, ARGS&&... _args) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.addComponent(_entity, std::forward<ARGS>(_args)...);
                }
                template<typename TComponent> bool removeComponent(TEntity& _entity) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.removeComponent(_entity);
                }
                template<typename TComponent> TComponent* getComponent(TEntity& _entity) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.getComponent(_entity);
                }
        };
    };
};

#endif
