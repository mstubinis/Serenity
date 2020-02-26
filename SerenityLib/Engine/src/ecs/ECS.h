#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

struct SceneOptions;
namespace Engine::priv {
    struct InternalScenePublicInterface;
}

#include <core/engine/utils/Utils.h>
#include <ecs/ECSRegistry.h>
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSSystem.h>
#include <mutex>

namespace Engine::priv {
    template<typename TEntity> 
    class ECS final {
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            ECSEntityPool<TEntity>                       m_EntityPool;
            std::vector<TEntity>                         m_JustAddedEntities;
            std::vector<TEntity>                         m_DestroyedEntities;
            std::vector<Engine::priv::sparse_set_base*>  m_ComponentPools;
            std::vector<ECSSystem<TEntity>*>             m_Systems;
            std::mutex                                   m_Mutex;

            //builds a component pool and system for the component type if it is not built already.
            template<typename TComponent> 
            void buildPool(const unsigned int type_slot) {
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
                    m_ComponentPools[type_slot] = NEW ECSComponentPool<TEntity, TComponent>();
                }
                if (!m_Systems[type_slot]) {
                    ECSSystemCI _ci;
                    m_Systems[type_slot] = NEW ECSSystem<TEntity, TComponent>(_ci, *this);
                }
            }
        public:
            ECS(/*const SceneOptions& options*/) {
            }
            ~ECS() {
                SAFE_DELETE_VECTOR(m_Systems);
                SAFE_DELETE_VECTOR(m_ComponentPools);
            }
            ECS(const ECS&)                      = delete;
            ECS& operator=(const ECS&)           = delete;
            ECS(ECS&& other) noexcept            = delete;
            ECS& operator=(ECS&& other) noexcept = delete;


            //"event handlers"
            template<typename TComponent> 
            void onResize(const unsigned int width, const unsigned int height) {
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                const auto& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                auto& components      = (*static_cast<CPoolType*>(m_ComponentPools[type_slot])).data();
                for (auto& camera : components) {
                    camera.resize(width, height);
                }
            }
            void update(const float dt, Scene& scene) {
                for (size_t i = 0; i < m_Systems.size(); ++i) { 
                    m_Systems[i]->onUpdate(dt, scene);
                }
            }
            void onComponentAddedToEntity(void* component, TEntity& entity, const unsigned int type_slot) {
                m_Systems[type_slot]->onComponentAddedToEntity(component, entity);
            }
            void onComponentRemovedFromEntity(TEntity& entity, const unsigned int type_slot) {
                m_Systems[type_slot]->onComponentRemovedFromEntity(entity);
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
            void preUpdate(Scene& scene, const float dt) {
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
            void postUpdate(Scene& scene, const float dt) {
                /*
                if (m_DestroyedEntities.size() > 0) {
                    for (auto& entityID : m_DestroyedEntities) {
                        m_EntityPool.destroyFlaggedEntity(entityID);
                    }
                    for (auto& component_pool : m_ComponentPools) {
                        for (auto& entityID : m_DestroyedEntities) {
                            component_pool->remove(entityID);
                            onComponentRemovedFromEntity(entityID, type_slot);
                        }
                    }
                    m_DestroyedEntities.clear();
                }
                */
                for (auto& entity : m_DestroyedEntities) {
                    EntityDataRequest request(entity);

                    const auto id = request.ID;
                    m_EntityPool.destroyFlaggedEntity(id);
                    for (size_t i = 0; i < m_ComponentPools.size(); ++i) {
                        m_ComponentPools[i]->remove(id);
                        m_Systems[i]->onComponentRemovedFromEntity(entity);
                    }
                    m_DestroyedEntities.clear();
                }
                for (auto& component_pool : m_ComponentPools) {
                    component_pool->reserve(150);
                }
            }

            template<class TComponent>
            ECSComponentPool<TEntity, TComponent>& getPool() const {
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                const auto& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                return *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
            }

            template<class TComponent>
            inline ECSSystem<TEntity, TComponent>& getSystem() const {
                const auto& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                return *static_cast<ECSSystem<TEntity, TComponent>*>(m_Systems[type_slot]);
            }



            template<class TComponent>
            void assignSystem(const ECSSystemCI& systemCI) {
                const auto& type_slot = ECSRegistry::type_slot<TComponent>();
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                using CSystemType     = ECSSystem<TEntity, TComponent>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPoolType();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW CSystemType(systemCI, *this);
            }
            template<class TComponent, typename TSystem>
            void assignSystem(const ECSSystemCI& systemCI) {
                const auto& type_slot = ECSRegistry::type_slot<TComponent>();
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPoolType();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW TSystem(systemCI, *this);
            }





            TEntity createEntity(Scene& scene) { 
                const TEntity res = m_EntityPool.addEntity(scene);
                m_JustAddedEntities.push_back(res);
                return std::move(res);
            }
            void removeEntity(const TEntity& entity) { 
                m_DestroyedEntities.push_back(entity);
            }
            priv::EntityPOD* getEntity(const unsigned int entityID) const { 
                return m_EntityPool.getEntity(entityID); 
            }
            template<class T, typename... ARGS> void addComponent(TEntity& entity, ARGS&&... args) {
                const auto& type_slot  = ECSRegistry::type_slot_fast<T>();
                auto& cPool            = *static_cast<ECSComponentPool<TEntity, T>*>(m_ComponentPools[type_slot]);
                T* res                 = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
            }
            template<class T, typename... ARGS> void addComponent(const EntityDataRequest& request, TEntity& entity, ARGS&& ... args) {
                const auto& type_slot  = ECSRegistry::type_slot_fast<T>();
                auto& cPool            = *static_cast<ECSComponentPool<TEntity, T>*>(m_ComponentPools[type_slot]);
                T* res                 = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(request, entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
            }
            template<class T> const bool removeComponent(TEntity& entity) {
                const auto& type_slot  = ECSRegistry::type_slot_fast<T>();
                auto& cPool            = *static_cast<ECSComponentPool<TEntity, T>*>(m_ComponentPools[type_slot]);

                bool ret_val = false;
                {
                    std::lock_guard lock(m_Mutex);
                    ret_val = cPool.removeComponent(entity);
                    if (ret_val) {
                        onComponentRemovedFromEntity(entity, type_slot);
                    }
                }
                return ret_val;
            }
            template<class T> 
            inline T* getComponent(TEntity& entity) const {
                using CPoolType = ECSComponentPool<TEntity, T>;
                return static_cast<CPoolType*>(m_ComponentPools[ECSRegistry::type_slot_fast<T>()])->getComponent(entity);
            }
            template<class T> 
            inline T* getComponent(const EntityDataRequest& dataRequest) const {
                using CPoolType = ECSComponentPool<TEntity, T>;
                return static_cast<CPoolType*>(m_ComponentPools[ECSRegistry::type_slot_fast<T>()])->getComponent(dataRequest);
            }
    };
};

#endif
