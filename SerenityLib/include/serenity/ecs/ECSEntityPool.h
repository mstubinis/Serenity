#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

struct SceneOptions;
class  Scene;
namespace Engine::priv {
    struct PublicScene;
};

#include <serenity/ecs/entity/Entity.h>
#include <serenity/system/Macros.h>

namespace Engine::priv {
    class ECSEntityPool final{
        friend struct Engine::priv::PublicScene;
        private:
            std::vector<Entity>    m_Pool;
            std::vector<uint32_t>  m_Freelist;
        public:
            ECSEntityPool() = default;
            ECSEntityPool(const ECSEntityPool&)                = delete;
            ECSEntityPool& operator=(const ECSEntityPool&)     = delete;
            ECSEntityPool(ECSEntityPool&&) noexcept            = delete;
            ECSEntityPool& operator=(ECSEntityPool&&) noexcept = delete;
            ~ECSEntityPool() = default;

            void init(const SceneOptions&);

            bool isEntityVersionDifferent(Entity) const noexcept;

            void destroyFlaggedEntity(uint32_t entityID);

            [[nodiscard]] Entity createEntity(const Scene&) noexcept;
            [[nodiscard]] Entity getEntityFromID(uint32_t entityID) const noexcept;


            void clear() noexcept {
                m_Pool.clear(); m_Freelist.clear();
            }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(std::vector<Entity>, m_Pool)
        };
};

#endif