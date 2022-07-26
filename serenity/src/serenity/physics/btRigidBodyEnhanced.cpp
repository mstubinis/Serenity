#include <serenity/physics/btRigidBodyEnhanced.h>
#include <serenity/system/Macros.h>

btRigidBodyEnhanced::btRigidBodyEnhanced(btScalar mass, btMotionState* motionState, btCollisionShape* collisionShape, const btVector3& localInertia)
    : btRigidBody{ mass, motionState, collisionShape, localInertia }
{

}
btRigidBodyEnhanced::btRigidBodyEnhanced(const btRigidBodyConstructionInfo& ci) 
    : btRigidBody{ ci }
{
}
btRigidBodyEnhanced::~btRigidBodyEnhanced() {
    ENGINE_PRODUCTION_LOG("Deleteing btRigidBodyEnhanced: " << m_Name)
}