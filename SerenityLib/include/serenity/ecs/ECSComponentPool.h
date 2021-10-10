#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

struct SceneOptions;

#include <serenity/containers/SparseSet.h>

namespace Engine::priv {
    template <class COMPONENT>
    class ECSComponentPool : public Engine::sparse_set<COMPONENT> {
        using sparse_base = Engine::sparse_set<COMPONENT>;
        private:
            ECSComponentPool() = delete;
        public:
            ECSComponentPool(const SceneOptions& options)
                : sparse_base{ 200U }
            {}

            ECSComponentPool(const ECSComponentPool&)                = default;
            ECSComponentPool& operator=(const ECSComponentPool&)     = default;
            ECSComponentPool(ECSComponentPool&&) noexcept            = default;
            ECSComponentPool& operator=(ECSComponentPool&&) noexcept = default;

            template<class ... ARGS>
            inline constexpr COMPONENT* addComponent(Entity entity, ARGS&&... args) noexcept { return sparse_base::add(entity.id(), entity, std::forward<ARGS>(args)...); }
            inline bool removeComponent(Entity entity) noexcept { return sparse_base::remove(entity.id()); }
            inline bool removeComponent(uint32_t entityID) noexcept { return sparse_base::remove(entityID); }
            [[nodiscard]] inline COMPONENT* getComponent(Entity entity) noexcept { return sparse_base::get(entity.id()); }
            [[nodiscard]] inline COMPONENT* getComponent(uint32_t entityID) noexcept { return sparse_base::get(entityID); }
    };
};
#endif