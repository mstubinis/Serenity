#include <serenity/physics/Raycast.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>

#include <iostream>

namespace {
    bool needs_collision_impl(btBroadphaseProxy* proxy0, int filterGroup, int filterMask, const std::vector<Entity>& entitiesIgnored) {
        bool collides = (proxy0->m_collisionFilterGroup & filterMask) != 0;
        collides = collides && (filterGroup & proxy0->m_collisionFilterMask);
        if (entitiesIgnored.size() > 0) {
            btCollisionObject* b = static_cast<btCollisionObject*>(proxy0->m_clientObject);
            for (auto entity : entitiesIgnored) {
                auto colShape = entity.getComponent<ComponentCollisionShape>();
                if (colShape && colShape->getBtShape() == b->getCollisionShape()) {
                    return false;
                }
            }
        }
        return collides;
    }
}

RayResultCallback::RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo)
    : btCollisionWorld::ClosestRayResultCallback{ rayFrom, rayTo }
{}
RayResultCallback::RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, std::vector<Entity>&& entitiesIgnored)
    : btCollisionWorld::ClosestRayResultCallback{ rayFrom, rayTo }
    , m_IgnoredEntities{ std::move(entitiesIgnored) }
{}
RayResultCallback::RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask) 
    : btCollisionWorld::ClosestRayResultCallback{ rayFrom, rayTo }
{
    m_collisionFilterGroup = group;
    m_collisionFilterMask = mask;
}
RayResultCallback::RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask, std::vector<Entity>&& entitiesIgnored) 
    : btCollisionWorld::ClosestRayResultCallback(rayFrom, rayTo)
    , m_IgnoredEntities{ std::move(entitiesIgnored) }
{
    m_collisionFilterGroup = group;
    m_collisionFilterMask = mask;
}
bool RayResultCallback::needsCollision(btBroadphaseProxy* proxy0) const {
    return needs_collision_impl(proxy0, m_collisionFilterGroup, m_collisionFilterMask, m_IgnoredEntities);
}
btScalar RayResultCallback::addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {
    return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
}











RayResultAllCallback::RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo)
    : btCollisionWorld::AllHitsRayResultCallback{ rayFrom, rayTo }
{}
RayResultAllCallback::RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, std::vector<Entity>&& entitiesIgnored)
    : btCollisionWorld::AllHitsRayResultCallback{ rayFrom, rayTo }
    , m_IgnoredEntities{ std::move(entitiesIgnored) }
{}
RayResultAllCallback::RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask)
    : btCollisionWorld::AllHitsRayResultCallback{ rayFrom, rayTo }
{
    m_collisionFilterGroup = group;
    m_collisionFilterMask = mask;
}
RayResultAllCallback::RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask, std::vector<Entity>&& entitiesIgnored)
    : btCollisionWorld::AllHitsRayResultCallback{ rayFrom, rayTo }
    , m_IgnoredEntities{ std::move(entitiesIgnored) }
{
    m_collisionFilterGroup = group;
    m_collisionFilterMask = mask;
}
bool RayResultAllCallback::needsCollision(btBroadphaseProxy* proxy0) const {
    return needs_collision_impl(proxy0, m_collisionFilterGroup, m_collisionFilterMask, m_IgnoredEntities);
}
btScalar RayResultAllCallback::addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {
    return AllHitsRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
}