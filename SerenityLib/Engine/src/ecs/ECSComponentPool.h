#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <ecs/SparseSet.h> //ok
#include <ecs/EntityDataRequest.h> //ok

namespace Engine {
    namespace epriv {
        template<typename ...> class ECSComponentPool;
        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity, TComponent> : public SparseSet<TEntity, TComponent>{
            using super = SparseSet<TEntity, TComponent>;
            public:
                ECSComponentPool()                                                  = default;
                ECSComponentPool(const ECSComponentPool& other) noexcept            = delete;
                ECSComponentPool& operator=(const ECSComponentPool& other) noexcept = delete;
                ECSComponentPool(ECSComponentPool&& other) noexcept                 = delete;
                ECSComponentPool& operator=(ECSComponentPool&& other) noexcept      = delete;
                ~ECSComponentPool() {
                }

                template<typename... ARGS> inline TComponent* addComponent(const TEntity& entity, ARGS&&... args) {
				    const EntityDataRequest dataRequest(entity);
                    return super::_add(dataRequest.ID, const_cast<TEntity&>(entity), std::forward<ARGS>(args)...);
                }
                template<typename... ARGS> inline TComponent* addComponent(const EntityDataRequest& dataRequest, const TEntity& entity, ARGS&& ... args) {
                    return super::_add(dataRequest.ID, const_cast<TEntity&>(entity), std::forward<ARGS>(args)...);
                }
                inline const bool removeComponent(const TEntity& entity) {
				    const EntityDataRequest dataRequest(entity);
                    return super::_remove(dataRequest.ID);
                }
                inline const bool removeComponent(const EntityDataRequest& dataRequest) {
                    return super::_remove(dataRequest.ID);
                }
                inline const bool removeComponent(const unsigned int& entityID) {
                    return super::_remove(entityID);
                }
                inline TComponent* getComponent(const TEntity& entity) {
				    const EntityDataRequest dataRequest(entity);
                    return super::_get(dataRequest.ID);
                }
                inline TComponent* getComponent(const EntityDataRequest& dataRequest) {
                    return super::_get(dataRequest.ID);
                }
                inline TComponent* getComponent(const unsigned int& entityID) {
                    return super::_get(entityID);
                }
                inline void reserveMore(const unsigned int amount = 500) {
                    super::reserveMore(amount);
                }
        };
    };
};
#endif