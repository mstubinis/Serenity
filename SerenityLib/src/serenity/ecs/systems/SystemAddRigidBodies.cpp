#include <serenity/ecs/systems/SystemAddRigidBodies.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/scene/Scene.h>

SystemAddRigidBodies::SystemAddRigidBodies(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        if (scene.m_WasJustSwappedTo) {
            scene.m_WasJustSwappedTo = false;
            return;
        }
        auto& system = (SystemAddRigidBodies&)inSystem;
        if (system.m_AddedRigidBodies.size() > 0) {
            for (const auto& BTRigidBody : system.m_AddedRigidBodies) {
                Engine::Physics::addRigidBody(BTRigidBody);
            }
            system.m_AddedRigidBodies.clear();
        }
        if (system.m_AddedRigidBodiesWithGroupAndMask.size() > 0) {
            for (const auto& [BTRigidBody, group, mask] : system.m_AddedRigidBodiesWithGroupAndMask) {
                Engine::Physics::addRigidBody(BTRigidBody, group, mask);
            }
            system.m_AddedRigidBodiesWithGroupAndMask.clear();
        }
    });
}
bool SystemAddRigidBodies::enqueueBody(btRigidBody* inBTRigidBody) {
    for (const auto& BTRigidBody : m_AddedRigidBodies) {
        if (BTRigidBody == inBTRigidBody) {
            return false;
        }
    }
    m_AddedRigidBodies.push_back(inBTRigidBody);
    return true;
}
bool SystemAddRigidBodies::enqueueBody(btRigidBody* inBTRigidBody, MaskType inGroup, MaskType inMask) {
    for (const auto& [BTRigidBody, g, m] : m_AddedRigidBodiesWithGroupAndMask) {
        if (BTRigidBody == inBTRigidBody) {
            return false;
        }
    }
    m_AddedRigidBodiesWithGroupAndMask.emplace_back(inBTRigidBody, inGroup, inMask);
    return true;
}
void SystemAddRigidBodies::removeBody(btRigidBody* inBTRigidBody) {
    std::erase_if(m_AddedRigidBodies, [inBTRigidBody](const auto ptr) {
        return ptr == inBTRigidBody;
    });
    std::erase_if(m_AddedRigidBodiesWithGroupAndMask, [inBTRigidBody](const auto ptr) {
        return std::get<0>(ptr) == inBTRigidBody;
    });
}