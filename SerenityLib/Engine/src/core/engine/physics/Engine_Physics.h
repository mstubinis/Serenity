#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

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
#include <LinearMath/btVector3.h>

#define PHYSICS_MIN_STEP 0.016666666666666666f

namespace Engine{
    struct RayCastResult final {
        glm::vec3          hitPosition     = glm::vec3(0.0f);
        glm::vec3          hitNormal       = glm::vec3(0.0f);
        btCollisionObject* collisionObject = nullptr;
    };
    namespace priv{
        class PhysicsManager final {
            public:
                static Engine::view_ptr<PhysicsManager> PHYSICS_MANAGER;
            public:
                Engine::priv::PhysicsPipeline    m_Pipeline;
                std::mutex                       m_Mutex;
                bool                             m_Paused                = false;
                unsigned int                     m_NumberOfStepsPerFrame = 1;
            public:
                PhysicsManager();
                ~PhysicsManager();

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
