#pragma once
#ifndef ENGINE_ECS_SYSTEM_REMOVE_RIGID_BODIES_H
#define ENGINE_ECS_SYSTEM_REMOVE_RIGID_BODIES_H

class  btRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemRemoveRigidBodies final : public SystemCRTP<SystemRemoveRigidBodies> {
    private:
        std::vector<btRigidBody*> m_RemovedRigidBodies;
    public:
        SystemRemoveRigidBodies(Engine::priv::ECS&);

        bool enqueueBody(btRigidBody*);
        void removeBody(btRigidBody*);
        void clear();
};

#endif