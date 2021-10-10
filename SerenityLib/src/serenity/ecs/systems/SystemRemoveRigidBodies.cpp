#include <serenity/ecs/systems/SystemRemoveRigidBodies.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/physics/PhysicsModule.h>

SystemRemoveRigidBodies::SystemRemoveRigidBodies(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = static_cast<SystemRemoveRigidBodies&>(inSystem);
        for (const auto& BTRigidBody : system.m_RemovedRigidBodies) {
            Engine::Physics::removeRigidBody(BTRigidBody);
        }
        system.m_RemovedRigidBodies.clear();
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemRemoveRigidBodies&>(inSystem);
        system.forEach<SystemRemoveRigidBodies*>([](SystemRemoveRigidBodies* inSystem, Entity entity) {
            inSystem->clear();
        }, &system, SystemExecutionPolicy::Normal);
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemRemoveRigidBodies&>(inSystem);
        system.forEach<SystemRemoveRigidBodies*>([](SystemRemoveRigidBodies* inSystem, Entity entity) {
            inSystem->clear();
        }, &system, SystemExecutionPolicy::Normal);
    });
}
bool SystemRemoveRigidBodies::enqueueBody(btRigidBody* inBTRigidBody) {
    for (const btRigidBody* BTRigidBody : m_RemovedRigidBodies) {
        if (BTRigidBody == inBTRigidBody) {
            return false;
        }
    }
    m_RemovedRigidBodies.push_back(inBTRigidBody);
    return true;
}
void SystemRemoveRigidBodies::removeBody(btRigidBody* inBTRigidBody) {
    for (size_t i = 0; i < m_RemovedRigidBodies.size(); ++i) {
        if (m_RemovedRigidBodies[i] == inBTRigidBody) {
            m_RemovedRigidBodies[i] = std::move(m_RemovedRigidBodies.back());
            m_RemovedRigidBodies.pop_back();
            break;
        }
    }
}
void SystemRemoveRigidBodies::clear() {
    m_RemovedRigidBodies.clear();
}