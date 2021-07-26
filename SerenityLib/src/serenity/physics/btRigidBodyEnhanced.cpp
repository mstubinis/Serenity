#include <serenity/physics/btRigidBodyEnhanced.h>

btRigidBodyEnhanced::btRigidBodyEnhanced(btScalar mass, btMotionState* motionState, btCollisionShape* collisionShape, const btVector3& localInertia)
    : btRigidBody{ mass, motionState, collisionShape, localInertia }
{

}
btRigidBodyEnhanced::btRigidBodyEnhanced(const btRigidBodyConstructionInfo& ci) 
    : btRigidBody{ ci }
{
}