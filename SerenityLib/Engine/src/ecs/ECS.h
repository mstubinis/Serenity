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
            ECSEntityPool<TEntity>              entityPool;
            std::vector<TEntity>                justAddedEntities;
            std::vector<uint>                   destroyedEntities;
            std::vector<SparseSet<TEntity>*>    componentPools;
            std::vector<ECSSystem<TEntity>*>    systems;

            //builds a component pool and system for the component type if it is not built already.
            template<typename TComponent> void buildPool(const uint& type_slot) {
                using CSystemType = ECSSystem<TEntity, TComponent>;
                using CPoolType   = ECSComponentPool<TEntity, TComponent>;
                if (type_slot >= componentPools.size()) {
                    componentPools.resize(type_slot + 1, nullptr);
                }
                if (componentPools[type_slot]) 
                    return;
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
            template<typename T> void onResize(const uint& w, const uint& h) {
                using CPoolType       = ECSComponentPool<TEntity, T>;
                const uint& type_slot = ECSRegistry::type_slot_fast<T>();
                auto& components = (*(CPoolType*)componentPools[type_slot]).pool();
                for (auto& camera : components) {
                    camera.resize(w, h);
                }
            }
            void update(const double& dt, Scene& scene) {
                for (uint i = 0; i < systems.size(); ++i) { 
                    auto& system = *systems[i];
                    system.update(dt, scene); 
                }
            }
            void onComponentAddedToEntity(void* component, TEntity& entity, const uint& type_slot) {
                auto& system = *systems[type_slot];
                system.onComponentAddedToEntity(component, entity);
            }
            void onSceneEntered(Scene& scene) { 
                for (uint i = 0; i < systems.size(); ++i) { 
                    auto& system = *systems[i];
                    system.onSceneEntered(scene); 
                }
            }
            void onSceneLeft(Scene& scene) { 
                for (uint i = 0; i < systems.size(); ++i) { 
                    auto& system = *systems[i];
                    system.onSceneLeft(scene); 
                }
            }
            void preUpdate(Scene& scene, const double& dt) {
                if (justAddedEntities.size() > 0) {
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i];
						for (uint j = 0; j < justAddedEntities.size(); ++j) {
                            system.onEntityAddedToScene(justAddedEntities[j], scene);
                        }
                    }
                    vector_clear(justAddedEntities);
                }
				
            }
            void postUpdate(Scene& scene, const double& dt) {
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
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                const uint& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                return *(CPoolType*)componentPools[type_slot];
            }
            template<typename TComponent> void assignSystem(const ECSSystemCI& systemCI) {
                const uint& type_slot = ECSRegistry::type_slot<TComponent>();
                using CPoolType       = ECSComponentPool<TEntity, TComponent>;
                using CSystemType     = ECSSystem<TEntity, TComponent>;
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
                systems[type_slot] = new CSystemType(systemCI, *this);
            }

            TEntity createEntity(Scene& scene) { 
                TEntity res = entityPool.addEntity(scene);
                justAddedEntities.push_back(res);
                return res;
            }
            void removeEntity(const uint& entityID) {
                destroyedEntities.push_back(entityID);
            }
            void removeEntity(const TEntity& entity) { 
                const EntityDataRequest dataRequest(entity);
                destroyedEntities.push_back(dataRequest.ID);
            }
            epriv::EntityPOD* getEntity(const uint& entityID) { 
                return entityPool.getEntity(entityID); 
            }
            template<typename TComponent, typename... ARGS> TComponent* addComponent(TEntity& entity, ARGS&&... args) {
                using CPoolType = ECSComponentPool<TEntity, TComponent>;
                const uint& type_slot = ECSRegistry::type_slot_fast<TComponent>();
                //buildPool<TComponent>(type_slot);
                auto& cPool = *(CPoolType*)componentPools[type_slot];
                TComponent* res = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
                return res;
            }
            template<typename TComponent> bool removeComponent(TEntity& entity) {
                using CPoolType = ECSComponentPool<TEntity, TComponent>;
                const uint& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                auto& cPool = *(CPoolType*)componentPools[type_slot];
                return cPool.removeComponent(entity);
            }
            template<typename TComponent> TComponent* getComponent(TEntity& entity) {
                using CPoolType = ECSComponentPool<TEntity, TComponent>;
                const uint& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                auto& cPool = *(CPoolType*)componentPools[type_slot];
                return cPool.getComponent(entity);
            }
            template<typename TComponent> TComponent* getComponent(const EntityDataRequest& dataRequest) {
                using CPoolType = ECSComponentPool<TEntity, TComponent>;
                const uint& type_slot  = ECSRegistry::type_slot_fast<TComponent>();
                auto& cPool = *(CPoolType*)componentPools[type_slot];
                return cPool.getComponent(dataRequest);
            }
    };
};
};

#endif
