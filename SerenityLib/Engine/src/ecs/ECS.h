#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

#include <ecs/ECSRegistry.h>
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSComponentPool.h>
#include <ecs/ECSSystem.h>
#include <ecs/Entity.h>
#include <memory>

namespace Engine {
    namespace epriv {

        template<typename TEntity> class ECS{
            friend struct Engine::epriv::InternalScenePublicInterface;
            private:
                ECSEntityPool<TEntity>                                    entityPool;
                std::vector<TEntity>                                      justAddedEntities;
                std::vector<uint>                                         destroyedEntities;
                std::vector<SparseSet<TEntity>*>                          componentPools;
                std::vector<ECSSystem<TEntity>*>                          systems;

                //builds a component pool and system for the component type if it is not built already.
                template<typename TComponent> void buildPool(uint type_slot) {
                    using CSystemType = ECSSystem<TEntity, TComponent>;
                    using CPoolType   = ECSComponentPool<TEntity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1, nullptr);
                    }
                    if (componentPools[type_slot]) return;
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1, nullptr);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = new CPoolType();
                    }
                    if (!systems[type_slot]) {
                        ECSSystemCI _ci;
                        systems[type_slot] = new CSystemType(_ci, *this);
                    }
                }
            public:
                ECS() = default;
                virtual ~ECS() {
                    SAFE_DELETE_VECTOR(systems);
                    SAFE_DELETE_VECTOR(componentPools);
                }
                ECS(const ECS&) = delete;                      // non construction-copyable
                ECS& operator=(const ECS&) = delete;           // non copyable
                ECS(ECS&& other) noexcept = delete;            // non construction-moveable
                ECS& operator=(ECS&& other) noexcept = delete; // non moveable


                //"event handlers"
                template<typename T> void onResize(uint& w, uint& h) {
                    using CPoolType = ECSComponentPool<TEntity, T>;
                    uint type_slot  = ECSRegistry::type_slot<T>();
                    std::vector<T>& components = (*(CPoolType*)componentPools[type_slot]).pool();
                    for (auto& camera : components) {
                        camera.resize(w, h);
                    }
                }
                void update(const float& dt, Scene& _Scene) {
                    for (uint i = 0; i < systems.size(); ++i) { 
                        auto& system = *systems[i];
                        system.update(dt, _Scene); 
                    }
                }
                void onComponentAddedToEntity(void* _component, TEntity& _entity, const uint& type_slot) {
                    auto& system = *systems[type_slot];
                    system.onComponentAddedToEntity(_component, _entity);
                }
                void onSceneEntered(Scene& _Scene) { 
                    for (uint i = 0; i < systems.size(); ++i) { 
                        auto& system = *systems[i];
                        system.onSceneEntered(_Scene); 
                    }
                }
                void onSceneLeft(Scene& _Scene) { 
                    for (uint i = 0; i < systems.size(); ++i) { 
                        auto& system = *systems[i];
                        system.onSceneLeft(_Scene); 
                    }
                }
                void preUpdate(Scene& _scene, const float& dt) {
                    if (justAddedEntities.size() > 0) {
                        for (uint i = 0; i < systems.size(); ++i) {
                            auto& system = *systems[i];
                            for (auto& _entity : justAddedEntities) {
                                system.onEntityAddedToScene(_entity, _scene);
                            }
                        }
                        vector_clear(justAddedEntities);
                    }
                }
                void postUpdate(Scene& _scene,const float& dt) {
                    if (destroyedEntities.size() > 0) {
                        for (uint i = 0; i < componentPools.size(); ++i) {
                            auto& pool = *componentPools[i];
                            for (auto& _index1 : destroyedEntities) {
                                pool._remove(_index1);
                            }
                        }
                        for (auto& _index : destroyedEntities) {
                            entityPool.destroyFlaggedEntity(_index);
                        }
                        vector_clear(destroyedEntities);
                    }
                }

                template<typename TComponent> ECSComponentPool<TEntity, TComponent>& getPool() {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot  = ECSRegistry::type_slot<TComponent>();
                    return *(CPoolType*)componentPools[type_slot];
                }
                template<typename TComponent> void assignSystem(const ECSSystemCI& _systemCI) {
                    uint type_slot    = ECSRegistry::type_slot<TComponent>();
                    using CPoolType   = ECSComponentPool<TEntity, TComponent>;
                    using CSystemType = ECSSystem<TEntity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1, nullptr);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = new CPoolType();
                    }
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1, nullptr);
                    }
                    if (systems[type_slot]) {
                        SAFE_DELETE(systems[type_slot]);
                    }
                    systems[type_slot] = new CSystemType(_systemCI, *this);
                }

                TEntity createEntity(Scene& _scene) { 
                    TEntity res = entityPool.addEntity(_scene);
                    justAddedEntities.push_back(res);
                    return res;
                }
                void removeEntity(const uint& _index) { 
                    destroyedEntities.push_back(_index); 
                }
                void removeEntity(TEntity& _entity) { 
                    EntityDataRequest dataRequest(_entity);
                    destroyedEntities.push_back(dataRequest.ID);
                }
                epriv::EntityPOD* getEntity(const uint& _entityID) { 
                    return entityPool.getEntity(_entityID); 
                }
                template<typename TComponent, typename... ARGS> TComponent* addComponent(TEntity& _entity, ARGS&&... _args) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot];
                    TComponent* res = cPool.addComponent(_entity, std::forward<ARGS>(_args)...);
                    if (res) {
                        uint type_slot = ECSRegistry::type_slot(res);
                        onComponentAddedToEntity(res, _entity, type_slot);
                    }
                    return res;
                }
                template<typename TComponent> bool removeComponent(TEntity& _entity) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot  = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot];
                    return cPool.removeComponent(_entity);
                }
                template<typename TComponent> TComponent* getComponent(TEntity& _entity) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot  = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot];
                    return cPool.getComponent(_entity);
                }
                template<typename TComponent> TComponent* getComponent(const EntityDataRequest& dataRequest) {
                    using CPoolType = ECSComponentPool<TEntity, TComponent>;
                    uint type_slot  = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot];
                    return cPool.getComponent(dataRequest);
                }
        };
    };
};

#endif
