#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include "ecs/SparseSet.h"

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
                    return super::_add(_entity, const_cast<TEntity&>(_entity), std::forward<ARGS>(_args)...);
                }
                bool removeComponent(const TEntity& _entity) {
                    return super::_remove(_entity);
                }
                TComponent* getComponent(const TEntity& _entity) {
                    return super::_get(_entity);
                }
        };
    };
};
#endif