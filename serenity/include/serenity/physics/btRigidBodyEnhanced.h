#pragma once
#ifndef ENGINE_PHYSICS_BT_RIGID_BODY_ENHANCED_H
#define ENGINE_PHYSICS_BT_RIGID_BODY_ENHANCED_H

class  btVector3;
class  btMotionState;
class  btCollisionShape;

#include <string>
#include <serenity/dependencies/glm.h>
#include <LinearMath/btScalar.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

// this is simply a btRigidBody with a std::string member "m_Name" for debugging info.
class btRigidBodyEnhanced : public btRigidBody {
    private:
        std::string m_Name;
    public:
        btRigidBodyEnhanced(btScalar mass, btMotionState*, btCollisionShape*, const btVector3& localInertia = btVector3(0, 0, 0));
        btRigidBodyEnhanced(const btRigidBodyConstructionInfo&);
        ~btRigidBodyEnhanced();

        inline void setName(const std::string& name) noexcept { m_Name = name; }

        [[nodiscard]] inline const std::string& getName() const noexcept { return m_Name; }
};

#endif