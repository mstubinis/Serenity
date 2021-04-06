#include <serenity/ecs/systems/SystemRemoveRigidBodies.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/physics/PhysicsModule.h>

SystemRemoveRigidBodies::SystemRemoveRigidBodies(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemRemoveRigidBodies&)inSystem;
        if (system.m_RemovedRigidBodies.size() > 0) {
            for (const auto& BTRigidBody : system.m_RemovedRigidBodies) {
                Engine::Physics::removeRigidBody(BTRigidBody);
            }
            system.m_RemovedRigidBodies.clear();
        }
    });
}
bool SystemRemoveRigidBodies::enqueueBody(btRigidBody* inBTRigidBody) {
    for (const auto& BTRigidBody : m_RemovedRigidBodies) {
        if (BTRigidBody == inBTRigidBody) {
            return false;
        }
    }
    m_RemovedRigidBodies.push_back(inBTRigidBody);
    return true;
}
void SystemRemoveRigidBodies::removeBody(btRigidBody* inBTRigidBody) {
    std::erase_if(m_RemovedRigidBodies, [inBTRigidBody](const auto ptr) {
        return ptr == inBTRigidBody;
    });
}