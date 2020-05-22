#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <core/engine/containers/SparseSet.h>
#include <ecs/EntityDataRequest.h> //ok

namespace Engine::priv {
    template<typename ...> class ECSComponentPool;

    template <typename TEntity,typename TComponent> 
    class ECSComponentPool<TEntity, TComponent> : public Engine::sparse_set<TComponent>, public Engine::NonCopyable, public Engine::NonMoveable{
        using super = Engine::sparse_set<TComponent>;
        public:
            ECSComponentPool() : super(400) {

            }
            ~ECSComponentPool() = default;

            template<typename... ARGS> 
            inline TComponent* addComponent(const TEntity& entity, ARGS&&... args) {
				const EntityDataRequest dataRequest(entity);
                return super::add(dataRequest.ID, const_cast<TEntity&>(entity), std::forward<ARGS>(args)...);
            }
            template<typename... ARGS> 
            inline TComponent* addComponent(const EntityDataRequest& dataRequest, const TEntity& entity, ARGS&& ... args) {
                return super::add(dataRequest.ID, const_cast<TEntity&>(entity), std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(const TEntity& entity) {
				const EntityDataRequest dataRequest(entity);
                return super::remove(dataRequest.ID);
            }
            inline bool removeComponent(const EntityDataRequest& dataRequest) {
                return super::remove(dataRequest.ID);
            }
            inline bool removeComponent(const unsigned int entityID) {
                return super::remove(entityID);
            }
            inline TComponent* getComponent(const TEntity& entity) const {
				const EntityDataRequest dataRequest(entity);
                return super::get(dataRequest.ID);
            }
            inline TComponent* getComponent(const EntityDataRequest& dataRequest) const {
                return super::get(dataRequest.ID);
            }
            inline TComponent* getComponent(const unsigned int entityID) const {
                return super::get(entityID);
            }
    };
};
#endif