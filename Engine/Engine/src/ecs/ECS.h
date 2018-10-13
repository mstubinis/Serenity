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
        template<typename TEntity> class ECS{
            private:
                ECSEntityPool<TEntity>                                    entityPool;
                std::vector<std::unique_ptr<SparseSet<TEntity>>>          componentPools;
                std::vector<ECSSystem<TEntity>*>                          systems;

                //builds a component pool and system for the component type if it is not built already.
                template<typename TComponent> void buildPool(uint type_slot) {
                    using CSystemType = ECSSystem<TEntity, TComponent>;
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (componentPools[type_slot]) return;
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1,nullptr);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<CPoolType>();
                    }
                    if (!systems[type_slot]) {
                        ECSSystemCI _ci;
                        systems[type_slot] = new CSystemType(_ci,*this);
                    }
                }
            public:
                ECS() = default;
                virtual ~ECS() {
                    SAFE_DELETE_VECTOR(systems);
                }
                ECS(const ECS&) = delete;                      // non construction-copyable
                ECS& operator=(const ECS&) = delete;           // non copyable
                ECS(ECS&& other) noexcept = delete;            // non construction-moveable
                ECS& operator=(ECS&& other) noexcept = delete; // non moveable


                //"event handlers"
                void update(const float& dt) { 
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i]; system.update(dt);
                    }
                }
                void onComponentAddedToEntity(void* _component, Entity& _entity) {
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i]; system.onComponentAddedToEntity(_component, _entity);
                    }
                }
                void onEntityAddedToScene(Entity& _entity, Scene& _scene) {
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i]; system.onEntityAddedToScene(_entity, _scene);
                    }
                }
                void onSceneEntered(Scene& _Scene) { 
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i]; system.onSceneEntered(_Scene);
                    }
                }
                void onSceneLeft(Scene& _Scene) { 
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i]; system.onSceneLeft(_Scene);
                    }
                }



                template<typename TComponent> ECSComponentPool<TEntity, TComponent>& getPool() {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    return *(CPoolType*)componentPools[type_slot].get();
                }
                template<typename TComponent> void assignSystem(const ECSSystemCI& _systemCI) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    using CSystemType = ECSSystem<TEntity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<CPoolType>();
                    }
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1);
                    }
                    if (systems[type_slot]) {
                        SAFE_DELETE(systems[type_slot]);
                    }
                    systems[type_slot] = new CSystemType(_systemCI,*this);
                }

                //we may or may not need these...
                TEntity createEntity(Scene& _scene) { return entityPool.addEntity(_scene); }
                void removeEntity(uint _entityID) { entityPool.removeEntity(_entityID); }
                void removeEntity(TEntity& _entity) { entityPool.removeEntity(_entity.ID); }
                TEntity* getEntity(uint _entityID) { return entityPool.getEntity(_entityID); }


                template<typename TComponent, typename... ARGS> TComponent* addComponent(const TEntity& _entity, ARGS&&... _args) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.addComponent(_entity, std::forward<ARGS>(_args)...);
                }
                template<typename TComponent> bool removeComponent(const TEntity& _entity) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.removeComponent(_entity);
                }
                template<typename TComponent> TComponent* getComponent(const TEntity& _entity) {
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
