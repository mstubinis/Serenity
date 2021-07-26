#pragma once
#ifndef ENGINE_PHYSICS_RAYCAST_H
#define ENGINE_PHYSICS_RAYCAST_H

class ComponentRigidBody;
class ComponentCollisionShape;

#include <serenity/dependencies/glm.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <vector>
#include <serenity/ecs/entity/Entity.h>

class RayResultCallback : public btCollisionWorld::ClosestRayResultCallback {
    private:
        std::vector<Entity> m_IgnoredEntities;
    public:
        RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo);
        RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, std::vector<Entity>&& entitiesIgnored);
        RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask);
        RayResultCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask, std::vector<Entity>&& entitiesIgnored);


        bool needsCollision(btBroadphaseProxy*) const override;
        btScalar addSingleResult(btCollisionWorld::LocalRayResult&, bool normalInWorldSpace) override;
};

class RayResultAllCallback : public btCollisionWorld::AllHitsRayResultCallback {
    private:
        std::vector<Entity> m_IgnoredEntities;
    public:
        RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo);
        RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, std::vector<Entity>&& entitiesIgnored);
        RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask);
        RayResultAllCallback(const btVector3& rayFrom, const btVector3& rayTo, int group, int mask, std::vector<Entity>&& entitiesIgnored);

        bool needsCollision(btBroadphaseProxy*) const override;
        btScalar addSingleResult(btCollisionWorld::LocalRayResult&, bool normalInWorldSpace) override;
};

#endif

