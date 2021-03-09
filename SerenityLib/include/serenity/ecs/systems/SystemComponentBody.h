#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_H

class  ComponentBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentBody final : public SystemCRTP<SystemComponentBody, ComponentBody> {
    friend class  ComponentBody;
    private:
        class ParentChildVector final {
            private:
                [[nodiscard]] inline uint32_t& getParent(uint32_t childID) noexcept { return Parents[childID - 1U]; }
                [[nodiscard]] inline glm_mat4& getWorld(uint32_t ID) noexcept { return WorldTransforms[ID - 1U]; }
                [[nodiscard]] inline glm_mat4& getLocal(uint32_t ID) noexcept { return LocalTransforms[ID - 1U]; }

                void internal_reserve_from_insert(uint32_t parentID, uint32_t childID);
            public:
                std::vector<glm_mat4>    WorldTransforms;
                std::vector<glm_mat4>    LocalTransforms;
                std::vector<uint32_t>    Parents;
                std::vector<uint32_t>    Order;
                uint32_t                 OrderHead = 0;

                void resize(size_t size);
                void reserve(size_t size);
                void insert(uint32_t parent, uint32_t child);
                void remove(uint32_t parent, uint32_t child);

                [[nodiscard]] inline constexpr uint32_t size() const noexcept { return OrderHead; }
                [[nodiscard]] inline size_t capacity() const noexcept { return Order.capacity(); }
        };
    public:
        SystemComponentBody::ParentChildVector ParentChildSystem;
    public:
        SystemComponentBody(Engine::priv::ECS& ecs);
};

#endif