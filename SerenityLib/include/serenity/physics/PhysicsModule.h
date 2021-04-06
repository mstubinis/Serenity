#pragma once
#ifndef ENGINE_PHYSICS_MODULE_H
#define ENGINE_PHYSICS_MODULE_H

class  Scene;
class  Entity;
class  btRigidBody;
class  btCollisionObject;
class  ComponentRigidBody;
class  Camera;
namespace Engine::priv {
    class  MeshImportedData;
    class  GLDebugDrawer;
};

#include <serenity/physics/PhysicsPipeline.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/types/ViewPointer.h>
#include <serenity/system/TypeDefs.h>
#include <mutex>

#define PHYSICS_MIN_STEP 0.016666666666666666f

namespace Engine{
    struct RayCastResult final {
        glm::vec3          hitPosition     = glm::vec3{ 0.0f };
        glm::vec3          hitNormal       = glm::vec3{ 0.0f };
        btCollisionObject* collisionObject = nullptr;
  
        RayCastResult(const btCollisionWorld::ClosestRayResultCallback& closestHitResult) {
            if (closestHitResult.hasHit()) {
                hitPosition     = Engine::Math::toGLM(closestHitResult.m_hitPointWorld);
                hitNormal       = Engine::Math::toGLM(closestHitResult.m_hitNormalWorld);
                collisionObject = const_cast<btCollisionObject*>(closestHitResult.m_collisionObject);
            }
        }
        RayCastResult(const btCollisionWorld::AllHitsRayResultCallback& allHitResult, int index) {
            hitPosition         = Engine::Math::toGLM(allHitResult.m_hitPointWorld[index]);
            hitNormal           = Engine::Math::toGLM(allHitResult.m_hitNormalWorld[index]);
            collisionObject     = const_cast<btCollisionObject*>(allHitResult.m_collisionObjects[index]);
        }
    };
    namespace priv{
        class PhysicsModule final {
            public:
                static Engine::view_ptr<PhysicsModule> PHYSICS_MANAGER;
            public:
                Engine::priv::PhysicsPipeline    m_Pipeline;
                std::mutex                       m_Mutex;
                bool                             m_Paused                = false;
                uint32_t                         m_NumberOfStepsPerFrame = 1;
            public:
                PhysicsModule();

                void init();

                void preUpdate(Scene&, const float dt) noexcept;
                void update(Scene&, const float dt, int maxSubSteps = 1, float fixedTimeStep = 0.0166666f);
                void render(Scene&, const Camera& camera);
        };
    };
    namespace Physics{
        std::vector<RayCastResult> rayCast(const btVector3& start, const btVector3& end, ComponentRigidBody* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        std::vector<RayCastResult> rayCast(const btVector3& start, const btVector3& end, std::vector<ComponentRigidBody*>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredObjects, MaskType group = -1, MaskType mask = -1);


        RayCastResult rayCastNearest(const btVector3& start, const btVector3& end, ComponentRigidBody* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        RayCastResult rayCastNearest(const btVector3& start, const btVector3& end, std::vector<ComponentRigidBody*>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        void setNumberOfStepsPerFrame(uint32_t numSteps);
        [[nodiscard]] uint32_t getNumberOfStepsPerFrame();

        void cleanProxyFromPairs(btRigidBody*);
        void updateDiscreteCollisionDetection() noexcept;
        void updateAABBs() noexcept;
        void recalculateOverlappingPairs() noexcept;

        void setGravity(float x, float y, float z);
        inline void setGravity(const glm::vec3& gravity) noexcept { setGravity(gravity.x, gravity.y, gravity.z); }
        void pause(bool paused = true);
        void unpause();

        bool addRigidBody(btRigidBody*, MaskType group, MaskType mask);
        bool addRigidBody(btRigidBody*);
        bool removeRigidBody(btRigidBody*);
        bool removeCollisionObject(btCollisionObject* object);
        void updateRigidBody(btRigidBody*);

        bool addRigidBodyThreadSafe(btRigidBody*, MaskType group, MaskType mask);
        bool addRigidBodyThreadSafe(btRigidBody*);
        bool removeRigidBodyThreadSafe(btRigidBody*);
        bool removeCollisionObjectThreadSafe(btCollisionObject* object);
        void updateRigidBodyThreadSafe(btRigidBody*);
    };
};
#endif
