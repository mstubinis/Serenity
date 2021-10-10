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
        auto& system = static_cast<SystemAddRigidBodies&>(inSystem);
        for (btRigidBody* BTRigidBody : system.m_AddedRigidBodies) {
            Engine::Physics::addRigidBody(BTRigidBody);
        }
        system.m_AddedRigidBodies.clear();


        for (const auto& data : system.m_AddedRigidBodiesWithGroupAndMask) {
            Engine::Physics::addRigidBody(data.rigidBody, data.group, data.mask);
        }
        system.m_AddedRigidBodiesWithGroupAndMask.clear();
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemAddRigidBodies&>(inSystem);
        system.forEach<SystemAddRigidBodies*>([](SystemAddRigidBodies* inSystem, Entity entity) {
            inSystem->clear();
        }, &system, SystemExecutionPolicy::Normal);
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemAddRigidBodies&>(inSystem);
        system.forEach<SystemAddRigidBodies*>([](SystemAddRigidBodies* inSystem, Entity entity) {
            inSystem->clear();
        }, &system, SystemExecutionPolicy::Normal);
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
    for (auto& data : m_AddedRigidBodiesWithGroupAndMask) {
        if (data.rigidBody == inBTRigidBody) {
            data.group = inGroup;
            data.mask  = inMask;
            return false;
        }
    }
    m_AddedRigidBodiesWithGroupAndMask.emplace_back(inBTRigidBody, inGroup, inMask);
    return true;
}
void SystemAddRigidBodies::removeBody(btRigidBody* inBTRigidBody) {
    for (size_t i = 0; i < m_AddedRigidBodies.size(); ++i) {
        if (m_AddedRigidBodies[i] == inBTRigidBody) {
            m_AddedRigidBodies[i] = std::move(m_AddedRigidBodies.back());
            m_AddedRigidBodies.pop_back();
            break;
        }
    }
    for (size_t i = 0; i < m_AddedRigidBodiesWithGroupAndMask.size(); ++i) {
        if (m_AddedRigidBodiesWithGroupAndMask[i].rigidBody == inBTRigidBody) {
            m_AddedRigidBodiesWithGroupAndMask[i] = std::move(m_AddedRigidBodiesWithGroupAndMask.back());
            m_AddedRigidBodiesWithGroupAndMask.pop_back();
            break;
        }
    }
}
void SystemAddRigidBodies::clear() {
    m_AddedRigidBodies.clear();
    m_AddedRigidBodiesWithGroupAndMask.clear();
}