#pragma once
#ifndef ENGINE_ECS_SYSTEM_TRANSFORM_PARENT_CHILD_H
#define ENGINE_ECS_SYSTEM_TRANSFORM_PARENT_CHILD_H

class  ComponentTransform;
class  ComponentRigidBody;
class  ComponentCollisionShape;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemTransformParentChild final : public SystemCRTP<SystemTransformParentChild, ComponentTransform> {
    friend class  ComponentTransform;
    private:
        static inline constexpr const uint32_t NULL_INDEX = std::numeric_limits<uint32_t>().max();
    private:
        [[nodiscard]] inline uint32_t& getParent(uint32_t childID) noexcept { return m_Parents[childID - 1U]; }
        [[nodiscard]] uint32_t getRootParent(uint32_t childID) noexcept { 
            uint32_t parent = m_Parents[childID - 1U];
            while (parent != 0 && getParent(parent) != 0) {
                parent = getParent(parent);
            }
            return parent;
        }
        [[nodiscard]] inline glm_mat4& getWorld(uint32_t ID) noexcept { return m_WorldTransforms[ID - 1U]; }
        [[nodiscard]] inline glm_mat4& getLocal(uint32_t ID) noexcept { return m_LocalTransforms[ID - 1U]; }

        void internal_reserve_from_insert(uint32_t parentID, uint32_t childID);    
    public:
        std::vector<glm_mat4>    m_WorldTransforms;
        std::vector<glm_mat4>    m_LocalTransforms;
        std::vector<uint32_t>    m_Parents;
        std::vector<uint32_t>    m_Order;

        void clear_all();
        void clear_and_shrink_all();

        void acquireMoreMemory(uint32_t entityID);
        void resize(size_t size);
        void reserve(size_t size);

        void addChild(uint32_t parentID, uint32_t childID);
        void removeChild(uint32_t parentID, uint32_t childID);
        std::pair<uint32_t, uint32_t> getBlockIndices(uint32_t parentID);

        [[nodiscard]] inline size_t size() const noexcept { return m_Order.size(); }
        [[nodiscard]] inline size_t capacity() const noexcept { return m_Order.capacity(); }
    public:
        SystemTransformParentChild(Engine::priv::ECS& ecs);

        void computeAllParentChildWorldTransforms();
        void syncRigidToTransform(ComponentRigidBody*, ComponentCollisionShape*, Entity);

        [[nodiscard]] Entity getParentEntity(Entity) const;
};

#endif