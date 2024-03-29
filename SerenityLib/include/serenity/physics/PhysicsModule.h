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
#include <serenity/ecs/entity/Entity.h>
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
                Engine::priv::PhysicsPipeline    m_Pipeline;
                std::mutex                       m_Mutex;
                uint32_t                         m_NumberOfStepsPerFrame = 1;
                bool                             m_Paused                = false;
            private:
                void internal_process_contact_manifolds();
            public:
                PhysicsModule();

                void init();

                void update(Scene&, const float dt, int maxSubSteps = 1, float fixedTimeStep = 0.01666666666666666f);
                void render(Scene&, const Camera&);
        };
    };
    namespace Physics{
        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, MaskType group = -1, MaskType mask = -1);
        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>&& ignored, MaskType group = -1, MaskType mask = -1);

        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, MaskType group = -1, MaskType mask = -1);
        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>&& ignored, MaskType group = -1, MaskType mask = -1);

        void drawDebugLine(const glm_vec3& start, const glm_vec3& end, float r, float g, float b);
        void drawDebugLine(const glm_vec3& start, const glm_vec3& end, const glm::vec3& color);

        void setNumberOfStepsPerFrame(uint32_t numSteps);
        [[nodiscard]] uint32_t getNumberOfStepsPerFrame();

        void cleanProxyFromPairs(btRigidBody*);
        void performDiscreteCollisionDetection() noexcept;
        void updateAABBs() noexcept;
        void calculateOverlappingPairs() noexcept;

        void setGravity(float x, float y, float z);
        inline void setGravity(const glm::vec3& gravity) noexcept { setGravity(gravity.x, gravity.y, gravity.z); }
        void pause(bool paused = true);
        void unpause();

        bool addRigidBody(btRigidBody*, MaskType group, MaskType mask);
        bool addRigidBody(btRigidBody*);
        bool removeRigidBody(btRigidBody*);
        bool removeCollisionObject(btCollisionObject*);
        void updateRigidBody(btRigidBody*);
    };
};
#endif
