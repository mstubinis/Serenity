#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

struct SceneOptions;

#include <core/engine/utils/Utils.h> //ok
#include <ecs/ECSRegistry.h> //ok
#include <ecs/ECSEntityPool.h> //has scene.h in it
#include <ecs/ECSSystem.h>

namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
        template<typename TEntity> class ECS{
            friend struct Engine::epriv::InternalScenePublicInterface;
            private:
                ECSEntityPool<TEntity>              m_EntityPool;
                std::vector<TEntity>                m_JustAddedEntities;
                std::vector<unsigned int>           m_DestroyedEntities;
                std::vector<SparseSet<TEntity>*>    m_ComponentPools;
                std::vector<ECSSystem<TEntity>*>    m_Systems;

                //builds a component pool and system for the component type if it is not built already.
                template<typename TComponent> void buildPool(const unsigned int& type_slot) {
                    using CSystemType = ECSSystem<TEntity, TComponent>;
                    using CPoolType   = ECSComponentPool<TEntity, TComponent>;
                    if (type_slot >= m_ComponentPools.size()) {
                        m_ComponentPools.resize(type_slot + 1, nullptr);
                    }
                    if (m_ComponentPools[type_slot]) {
                        return;
                    }
                    if (type_slot >= m_Systems.size()) {
                        m_Systems.resize(type_slot + 1, nullptr);
                    }
                    if (!m_ComponentPools[type_slot]) {
                        m_ComponentPools[type_slot] = ALLOC CPoolType();
                    }
                    if (!m_Systems[type_slot]) {
                        ECSSystemCI _ci;
                        m_Systems[type_slot] = ALLOC CSystemType(_ci, *this);
                    }
                }
            public:
                ECS(/*const SceneOptions& options*/) {
                }
                virtual ~ECS() {
                    SAFE_DELETE_VECTOR(m_Systems);
                    SAFE_DELETE_VECTOR(m_ComponentPools);
                }
                ECS(const ECS&)                      = delete;
                ECS& operator=(const ECS&)           = delete;
                ECS(ECS&& other) noexcept            = delete;
                ECS& operator=(ECS&& other) noexcept = delete;


                //"event handlers"
                template<typename T> void onResize(const unsigned int& width, const unsigned int& height) {
                    using CPoolType       = ECSComponentPool<TEntity, T>;
                    const auto& type_slot = ECSRegistry::type_slot_fast<T>();
                    auto& components      = (*static_cast<CPoolType*>(m_ComponentPools[type_slot])).pool();
                    for (auto& camera : components) {
                        camera.resize(width, height);
                    }
                }
                void update(const double& dt, Scene& scene) {
                    for (size_t i = 0; i < m_Systems.size(); ++i) { 
                        m_Systems[i]->onUpdate(dt, scene);
                    }
                }
                void onComponentAddedToEntity(void* component, TEntity& entity, const unsigned int& type_slot) {
                    m_Systems[type_slot]->onComponentAddedToEntity(component, entity);
                }
                void onSceneEntered(Scene& scene) { 
                    for (size_t i = 0; i < m_Systems.size(); ++i) { 
                        m_Systems[i]->onSceneEntered(scene);
                    }
                }
                void onSceneLeft(Scene& scene) { 
                    for (size_t i = 0; i < m_Systems.size(); ++i) { 
                        m_Systems[i]->onSceneLeft(scene);
                    }
                }
                //add newly created entities to the scene with their components as defined in their m_Systems, etc
                void preUpdate(Scene& scene, const double& dt) {
                    if (m_JustAddedEntities.size() > 0) {
                        for (size_t i = 0; i < m_Systems.size(); ++i) {
                            for (size_t j = 0; j < m_JustAddedEntities.size(); ++j) {
                                m_Systems[i]->onEntityAddedToScene(m_JustAddedEntities[j], scene);
                            }
                        }
                        m_JustAddedEntities.clear();
                    }
                }
                //destroy flagged entities & their components, if any
                void postUpdate(Scene& scene, const double& dt) {
                    if (m_DestroyedEntities.size() > 0) {
                        for (auto& entityID : m_DestroyedEntities) {
                            m_EntityPool.destroyFlaggedEntity(entityID);
                        }
                        for (auto& pool : m_ComponentPools) {
                            for (auto& entityID : m_DestroyedEntities) {
                                pool->_remove(entityID);
                            }
                        }
                        m_DestroyedEntities.clear();
                    }
                    for (auto& pool : m_ComponentPools) {
                        pool->reserveMore(1500);
                    }
                }

                template<typename TComponent> ECSComponentPool<TEntity, TComponent>& getPool() {
                    using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                    return *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                }
                template<typename TComponent> void assignSystem(const ECSSystemCI& systemCI) {
                    const auto& type_slot = ECSRegistry::type_slot<TComponent>();
                    using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                    using CSystemType     = ECSSystem<TEntity, TComponent>;
                    if (type_slot >= m_ComponentPools.size()) {
                        m_ComponentPools.resize(type_slot + 1, nullptr);
                    }
                    if (!m_ComponentPools[type_slot]) {
                        m_ComponentPools[type_slot] = ALLOC CPoolType();
                    }
                    if (type_slot >= m_Systems.size()) {
                        m_Systems.resize(type_slot + 1, nullptr);
                    }
                    if (m_Systems[type_slot]) {
                        SAFE_DELETE(m_Systems[type_slot]);
                    }
                    m_Systems[type_slot] = ALLOC CSystemType(systemCI, *this);
                }
                TEntity createEntity(Scene& scene) { 
                    const TEntity res = m_EntityPool.addEntity(scene);
                    m_JustAddedEntities.push_back(res);
                    return std::move(res);
                }
                void removeEntity(const unsigned int& entityID) {
                    m_DestroyedEntities.push_back(entityID);
                }
                void removeEntity(const TEntity& entity) { 
                    const EntityDataRequest dataRequest(entity);
                    m_DestroyedEntities.push_back(dataRequest.ID);
                }
                epriv::EntityPOD* getEntity(const unsigned int& entityID) { 
                    return m_EntityPool.getEntity(entityID); 
                }
                template<typename TComponent, typename... ARGS> TComponent* addComponent(TEntity& entity, ARGS&&... args) {
                    using CPoolType        = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                    auto& cPool            = *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                    TComponent* res        = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                    if (res) {
                        onComponentAddedToEntity(res, entity, type_slot);
                    }
                    return res;
                }
                template<typename TComponent, typename... ARGS> TComponent* addComponent(const EntityDataRequest& request, TEntity& entity, ARGS&& ... args) {
                    using CPoolType        = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                    auto& cPool            = *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                    TComponent* res        = cPool.addComponent(request, entity, std::forward<ARGS>(args)...);
                    if (res) {
                        onComponentAddedToEntity(res, entity, type_slot);
                    }
                    return res;
                }
                template<typename TComponent> const bool removeComponent(TEntity& entity) {
                    using CPoolType        = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                    auto& cPool            = *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                    return cPool.removeComponent(entity);
                }
                template<typename TComponent> TComponent* getComponent(TEntity& entity) {
                    using CPoolType        = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                    auto& cPool            = *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                    return cPool.getComponent(entity);
                }
                template<typename TComponent> TComponent* getComponent(const EntityDataRequest& dataRequest) {
                    using CPoolType        = ECSComponentPool<TEntity, TComponent>;
                    const auto& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                    auto& cPool            = *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
                    return cPool.getComponent(dataRequest);
                }
        };
    };
};

#endif
