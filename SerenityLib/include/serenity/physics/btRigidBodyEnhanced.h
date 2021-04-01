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

        inline void setName(std::string_view name) { m_Name = name; }
        inline void setName(const char* cName) { m_Name = cName; }
        inline void setName(std::string name) { m_Name = std::move(name); }

        [[nodiscard]] inline const std::string& getName() const noexcept { return m_Name; }
};

#endif