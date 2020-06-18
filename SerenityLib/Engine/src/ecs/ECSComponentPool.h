#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

namespace Engine::priv {
    template<typename ENTITY>
    class ECS;
}

#include <core/engine/containers/SparseSet.h>
#include <ecs/EntityDataRequest.h>

namespace Engine::priv {
    template<typename ...> class ECSComponentPool;

    template <typename ENTITY, typename COMPONENT>
    class ECSComponentPool<ENTITY, COMPONENT> : public Engine::sparse_set<COMPONENT>, public Engine::NonCopyable, public Engine::NonMoveable{
        friend class Engine::priv::ECS<ENTITY>;
        using super = Engine::sparse_set<COMPONENT>;
        private:
            //unsigned int m_SortValue = 0;
        public:
            ECSComponentPool() : super(400) {

            }
            ~ECSComponentPool() = default;

            template<typename... ARGS> 
            inline COMPONENT* addComponent(ENTITY entity, ARGS&&... args) {
				const EntityDataRequest dataRequest(entity);
                return super::add(dataRequest.ID, entity, std::forward<ARGS>(args)...);
            }
            template<typename... ARGS> 
            inline COMPONENT* addComponent(const EntityDataRequest& dataRequest, ENTITY entity, ARGS&& ... args) {
                return super::add(dataRequest.ID, entity, std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(ENTITY entity) {
				const EntityDataRequest dataRequest(entity);
                return super::remove(dataRequest.ID);
            }
            inline bool removeComponent(const EntityDataRequest& dataRequest) {
                return super::remove(dataRequest.ID);
            }
            inline bool removeComponent(std::uint32_t entityID) {
                return super::remove(entityID);
            }
            inline COMPONENT* getComponent(ENTITY entity) const {
				const EntityDataRequest dataRequest(entity);
                return super::get(dataRequest.ID);
            }
            inline COMPONENT* getComponent(const EntityDataRequest& dataRequest) const {
                return super::get(dataRequest.ID);
            }
            inline COMPONENT* getComponent(std::uint32_t entityID) const {
                return super::get(entityID);
            }
    };
};
#endif