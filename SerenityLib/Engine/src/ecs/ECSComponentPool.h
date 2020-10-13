#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

struct SceneOptions;

namespace Engine::priv {
    template <typename COMPONENT>
    class ECSComponentPool : public Engine::sparse_set<COMPONENT> {
        private:
            ECSComponentPool() = delete;
        public:
            ECSComponentPool(const SceneOptions& options)
                : Engine::sparse_set<COMPONENT>{ 400U }
            {}

            ECSComponentPool(const ECSComponentPool& other) = delete;
            ECSComponentPool& operator=(const ECSComponentPool& other) = delete;
            ECSComponentPool(ECSComponentPool&& other) noexcept = delete;
            ECSComponentPool& operator=(ECSComponentPool&& other) noexcept = delete;

            ~ECSComponentPool() = default;

            template<typename... ARGS> 
            inline CONSTEXPR COMPONENT* addComponent(Entity entity, ARGS&&... args) noexcept {
                return Engine::sparse_set<COMPONENT>::add(entity.id(), entity, std::forward<ARGS>(args)...);
            }
            inline bool removeComponent(Entity entity) noexcept {
                return Engine::sparse_set<COMPONENT>::remove(entity.id());
            }
            inline bool removeComponent(uint32_t entityID) noexcept {
                return Engine::sparse_set<COMPONENT>::remove(entityID);
            }
            inline CONSTEXPR COMPONENT* getComponent(Entity entity) const noexcept {
                return Engine::sparse_set<COMPONENT>::get(entity.id());
            }
            inline CONSTEXPR COMPONENT* getComponent(uint32_t entityID) const noexcept {
                return Engine::sparse_set<COMPONENT>::get(entityID);
            }
    };
};
#endif