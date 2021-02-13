#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

struct SceneOptions;

#include <serenity/containers/SparseSet.h>

namespace Engine::priv {
    template <typename COMPONENT>
    class ECSComponentPool : public Engine::sparse_set<COMPONENT> {
        private:
            ECSComponentPool() = delete;
        public:
            ECSComponentPool(const SceneOptions& options)
                : Engine::sparse_set<COMPONENT>{ 400U }
            {}

            ECSComponentPool(const ECSComponentPool&)                = default;
            ECSComponentPool& operator=(const ECSComponentPool&)     = default;
            ECSComponentPool(ECSComponentPool&&) noexcept            = default;
            ECSComponentPool& operator=(ECSComponentPool&&) noexcept = default;

            ~ECSComponentPool() = default;

            template<typename... ARGS> 
            inline constexpr COMPONENT* addComponent(Entity entity, ARGS&&... args) noexcept {
                return Engine::sparse_set<COMPONENT>::add(entity.id(), entity, std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(Entity entity) noexcept {
                return Engine::sparse_set<COMPONENT>::remove(entity.id());
            }
            inline bool removeComponent(uint32_t entityID) noexcept {
                return Engine::sparse_set<COMPONENT>::remove(entityID);
            }
            [[nodiscard]] inline constexpr COMPONENT* getComponent(Entity entity) const noexcept {
                return Engine::sparse_set<COMPONENT>::get(entity.id());
            }
            [[nodiscard]] inline constexpr COMPONENT* getComponent(uint32_t entityID) const noexcept {
                return Engine::sparse_set<COMPONENT>::get(entityID);
            }
    };
};
#endif