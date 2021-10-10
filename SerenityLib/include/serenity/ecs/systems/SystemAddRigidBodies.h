#pragma once
#ifndef ENGINE_ECS_SYSTEM_ADD_RIGID_BODIES_H
#define ENGINE_ECS_SYSTEM_ADD_RIGID_BODIES_H

class  btRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    struct SystemAddRigidBodies_RigidBodyWithGroupAndMask final {
        btRigidBody*  rigidBody;
        MaskType      group;
        MaskType      mask;

        SystemAddRigidBodies_RigidBodyWithGroupAndMask() = default;
        SystemAddRigidBodies_RigidBodyWithGroupAndMask(btRigidBody* inBody, MaskType inGroup, MaskType inMask) noexcept
            : rigidBody{ inBody }
            , group    { inGroup }
            , mask     { inMask }
        {}
        SystemAddRigidBodies_RigidBodyWithGroupAndMask(const SystemAddRigidBodies_RigidBodyWithGroupAndMask&) = default;
        SystemAddRigidBodies_RigidBodyWithGroupAndMask& operator=(const SystemAddRigidBodies_RigidBodyWithGroupAndMask&) = default;
        SystemAddRigidBodies_RigidBodyWithGroupAndMask(SystemAddRigidBodies_RigidBodyWithGroupAndMask&& other) noexcept
            : rigidBody{ std::exchange(other.rigidBody, nullptr) }
            , group    { std::move(other.group) }
            , mask     { std::move(other.mask) }
        {}
        SystemAddRigidBodies_RigidBodyWithGroupAndMask& operator=(SystemAddRigidBodies_RigidBodyWithGroupAndMask&& other) noexcept {
            rigidBody = std::exchange(other.rigidBody, nullptr);
            group     = std::move(other.group);
            mask      = std::move(other.mask);
            return *this;
        }
    };
};
class SystemAddRigidBodies final : public SystemCRTP<SystemAddRigidBodies> {
    private:
        std::vector<btRigidBody*>                                                  m_AddedRigidBodies;
        std::vector<Engine::priv::SystemAddRigidBodies_RigidBodyWithGroupAndMask>  m_AddedRigidBodiesWithGroupAndMask;
    public:
        SystemAddRigidBodies(Engine::priv::ECS&);

        bool enqueueBody(btRigidBody*);
        bool enqueueBody(btRigidBody*, MaskType group, MaskType mask);

        void removeBody(btRigidBody*);
        void clear();
};

#endif