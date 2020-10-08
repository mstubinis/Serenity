#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

struct SceneOptions;
class  Scene;
namespace Engine::priv {
    struct InternalScenePublicInterface;
};

#include <ecs/Entity.h>

namespace Engine::priv {
    class ECSEntityPool final{
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            std::vector<Entity>          m_Pool;
            std::vector<uint32_t>   m_Freelist;
        public:
            ECSEntityPool() = default;
            ECSEntityPool(const ECSEntityPool&)                      = delete;
            ECSEntityPool& operator=(const ECSEntityPool&)           = delete;
            ECSEntityPool(ECSEntityPool&& other) noexcept            = delete;
            ECSEntityPool& operator=(ECSEntityPool&& other) noexcept = delete;
            ~ECSEntityPool() = default;

            void init(const SceneOptions& options);

            bool isEntityVersionDifferent(Entity entity) const noexcept;

            void destroyFlaggedEntity(uint32_t entityID);
            Entity addEntity(const Scene& scene) noexcept;
            Entity getEntity(uint32_t entityData) const noexcept;
        };
};

#endif