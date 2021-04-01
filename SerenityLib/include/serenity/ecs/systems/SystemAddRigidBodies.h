#pragma once
#ifndef ENGINE_ECS_SYSTEM_ADD_RIGID_BODIES_H
#define ENGINE_ECS_SYSTEM_ADD_RIGID_BODIES_H

class  btRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/system/TypeDefs.h>

class SystemAddRigidBodies final : public SystemCRTP<SystemAddRigidBodies> {
    private:
        std::vector<btRigidBody*>                                 m_AddedRigidBodies;
        std::vector<std::tuple<btRigidBody*, MaskType, MaskType>> m_AddedRigidBodiesWithGroupAndMask;
    public:
        SystemAddRigidBodies(Engine::priv::ECS& ecs);

        bool enqueueBody(btRigidBody*);
        bool enqueueBody(btRigidBody*, MaskType group, MaskType mask);

        void removeBody(btRigidBody*);
};

#endif