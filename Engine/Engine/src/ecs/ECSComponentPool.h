#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include "ecs/SparseSet.h"
#include "ecs/EntitySerialization.h"

namespace Engine {
    namespace epriv {

        template<typename ...> class ECSComponentPool;

        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public SparseSet<TEntity, TComponent>{
            using super  = SparseSet<TEntity, TComponent>;
            public:
                ECSComponentPool() = default;
                ECSComponentPool(const ECSComponentPool& other) noexcept = delete;
                ECSComponentPool& operator=(const ECSComponentPool& other) noexcept = delete;
                ECSComponentPool(ECSComponentPool&& other) noexcept = delete;
                ECSComponentPool& operator=(ECSComponentPool&& other) noexcept = delete;

                ~ECSComponentPool() = default;

                template<typename... ARGS> TComponent* addComponent(const TEntity& _entity, ARGS&&... _args) {
                    EntitySerialization _s(_entity);
                    return super::_add(_s.ID, const_cast<TEntity&>(_entity), std::forward<ARGS>(_args)...);
                }
                bool removeComponent(const TEntity& _entity) {
                    EntitySerialization _s(_entity);
                    return super::_remove(_s.ID);
                }
                TComponent* getComponent(const TEntity& _entity) {
                    EntitySerialization _s(_entity);
                    return super::_get(_s.ID);
                }
        };
    };
};
#endif