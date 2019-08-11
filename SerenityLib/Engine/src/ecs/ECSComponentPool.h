#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <ecs/SparseSet.h>
#include <ecs/EntityDataRequest.h>

namespace Engine {
namespace epriv {
    template<typename ...> class ECSComponentPool;
    template <typename E,typename T> class ECSComponentPool<E,T> : public SparseSet<E, T>{
        using super = SparseSet<E, T>;
        public:
            ECSComponentPool()                                                  = default;
            ECSComponentPool(const ECSComponentPool& other) noexcept            = delete;
            ECSComponentPool& operator=(const ECSComponentPool& other) noexcept = delete;
            ECSComponentPool(ECSComponentPool&& other) noexcept                 = delete;
            ECSComponentPool& operator=(ECSComponentPool&& other) noexcept      = delete;
            ~ECSComponentPool()                                                 = default;

            template<typename... ARGS> inline T* addComponent(const E& entity, ARGS&&... args) {
				const EntityDataRequest dataRequest(entity);
                return super::_add(dataRequest.ID, const_cast<E&>(entity), std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(const E& entity) {
				const EntityDataRequest dataRequest(entity);
                return super::_remove(dataRequest.ID);
            }
            inline bool removeComponent(const EntityDataRequest& dataRequest) {
                return super::_remove(dataRequest.ID);
            }
            inline bool removeComponent(const uint& entityID) {
                return super::_remove(entityID);
            }
            inline T* getComponent(const E& entity) {
				const EntityDataRequest dataRequest(entity);
                return super::_get(dataRequest.ID);
            }
            inline T* getComponent(const EntityDataRequest& dataRequest) {
                return super::_get(dataRequest.ID);
            }
            inline T* getComponent(const uint& entityID) { 
                return super::_get(entityID);
            }
    };
};
};
#endif