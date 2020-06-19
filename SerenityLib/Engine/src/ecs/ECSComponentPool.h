#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

namespace Engine::priv {
    template<typename ENTITY>
    class ECS;
}

#include <core/engine/containers/SparseSet.h>

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
                return super::add(entity.id(), entity, std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(ENTITY entity) {
                return super::remove(entity.id());
            }
            inline bool removeComponent(std::uint32_t entityID) {
                return super::remove(entityID);
            }
            inline COMPONENT* getComponent(ENTITY entity) const {
                return super::get(entity.id());
            }
            inline COMPONENT* getComponent(std::uint32_t entityID) const {
                return super::get(entityID);
            }
    };
};
#endif