#pragma once
#ifndef ENGINE_PHYSICS_MODULE_H
#define ENGINE_PHYSICS_MODULE_H

struct Entity;
class  btRigidBody;
class  btCollisionObject;
class  ComponentBody;
class  Camera;
namespace Engine::priv {
    struct MeshImportedData;
    class  GLDebugDrawer;
};

#include <core/engine/physics/PhysicsPipeline.h>
#include <core/engine/math/Engine_Math.h>

#define PHYSICS_MIN_STEP 0.016666666666666666f

namespace Engine{
    struct RayCastResult final {
        glm::vec3          hitPosition           = glm::vec3(0.0f);
        glm::vec3          hitNormal             = glm::vec3(0.0f);
        btCollisionObject* collisionObject       = nullptr;
  
        RayCastResult(const btCollisionWorld::ClosestRayResultCallback& closestHitResult) {
            if (closestHitResult.hasHit()) {
                hitPosition     = Engine::Math::btVectorToGLM(closestHitResult.m_hitPointWorld);
                hitNormal       = Engine::Math::btVectorToGLM(closestHitResult.m_hitNormalWorld);
                collisionObject = const_cast<btCollisionObject*>(closestHitResult.m_collisionObject);
            }
        }
        RayCastResult(const btCollisionWorld::AllHitsRayResultCallback& allHitResult, const int index) {
            hitPosition           = Engine::Math::btVectorToGLM(allHitResult.m_hitPointWorld[index]);
            hitNormal             = Engine::Math::btVectorToGLM(allHitResult.m_hitNormalWorld[index]);
            collisionObject       = const_cast<btCollisionObject*>(allHitResult.m_collisionObjects[index]);
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
                unsigned int                     m_NumberOfStepsPerFrame = 1;
            public:
                PhysicsModule();
                ~PhysicsModule();

                void _init();

                void preUpdate(const float dt) noexcept;
                void _update(const float dt, int maxSubSteps = 1, float fixedTimeStep = 0.0166666f);
                void _render(const Camera& camera);

                bool add_rigid_body(btRigidBody* rigidBody, MaskType group, MaskType mask, bool doGroupAndMask) noexcept;
        };
    };
    namespace Physics{
        std::vector<RayCastResult> rayCast(btVector3& start, btVector3& end, ComponentBody* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        std::vector<RayCastResult> rayCast(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        std::vector<RayCastResult> rayCast(glm::vec3& start, glm::vec3& end, Entity* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        std::vector<RayCastResult> rayCast(glm::vec3& start, glm::vec3& end, std::vector<Entity>& ignoredObjects, MaskType group = -1, MaskType mask = -1);


        RayCastResult rayCastNearest(btVector3& start, btVector3& end, ComponentBody* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        RayCastResult rayCastNearest(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        RayCastResult rayCastNearest(glm::vec3& start, glm::vec3& end, Entity* ignoredObject = nullptr, MaskType group = -1, MaskType mask = -1);
        RayCastResult rayCastNearest(glm::vec3& start, glm::vec3& end, std::vector<Entity>& ignoredObjects, MaskType group = -1, MaskType mask = -1);

        void setNumberOfStepsPerFrame(unsigned int numSteps);
        unsigned int getNumberOfStepsPerFrame();

        void setGravity(float x, float y, float z);
        void setGravity(const glm::vec3& gravity);
        void pause(bool paused = true);
        void unpause();

        bool addRigidBody(Entity entity);
        bool addRigidBody(ComponentBody&);
        bool addRigidBody(btRigidBody*, MaskType group, MaskType mask);
        bool addRigidBody(btRigidBody*);
        bool removeRigidBody(Entity entity);
        bool removeRigidBody(btRigidBody*);
        bool removeRigidBody(ComponentBody&);
        bool removeCollisionObject(btCollisionObject* object);

        void updateRigidBody(btRigidBody*);

        bool addRigidBodyThreadSafe(Entity entity);
        bool addRigidBodyThreadSafe(ComponentBody&);
        bool addRigidBodyThreadSafe(btRigidBody*, MaskType group, MaskType mask);
        bool addRigidBodyThreadSafe(btRigidBody*);
        bool removeRigidBodyThreadSafe(Entity entity);
        bool removeRigidBodyThreadSafe(btRigidBody*);
        bool removeRigidBodyThreadSafe(ComponentBody&);
        bool removeCollisionObjectThreadSafe(btCollisionObject* object);

        void updateRigidBodyThreadSafe(btRigidBody*);
    };
};
#endif
