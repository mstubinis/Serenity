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
    //class  PhysicsWorld;
};

#include <core/engine/physics/World.h>
#include <core/engine/math/Numbers.h>
#include <vector>
#include <mutex>
#include <LinearMath/btVector3.h>

#define PHYSICS_MIN_STEP 0.016666666666666666f

namespace Engine{
    struct RayCastResult {
        glm::vec3 hitPosition;
        glm::vec3 hitNormal;
        RayCastResult() {
            hitNormal = hitPosition = glm::vec3(0.0f);
        }
    };
    namespace priv{
        class PhysicsManager final{
            public:
                Engine::priv::PhysicsWorld    m_Data;
                std::mutex                    m_Mutex;
                bool                          m_Paused                = false;
                unsigned int                  m_NumberOfStepsPerFrame = 1;
            public:
                PhysicsManager();
                ~PhysicsManager();

                void debug_draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
                void debug_draw_line(const glm::vec3& start, const glm::vec3& end, const float r, const float g, const float b, const float a);

                void cleanup();

                void _init();

                void _update(const float& dt, int maxSubSteps = 1, float fixedTimeStep = 0.0166666f);
                void _render(const Camera& camera);
        };
    };
    namespace Physics{
        std::vector<RayCastResult> rayCast(const btVector3& start, const btVector3& end, ComponentBody* ignoredObject = nullptr, const unsigned short group = -1, const unsigned short mask = -1);
        std::vector<RayCastResult> rayCast(const btVector3& start, const btVector3& end, std::vector<ComponentBody*>& ignoredObjects, const unsigned short group = -1, const unsigned short mask = -1);

        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr, const unsigned short group = -1, const unsigned short mask = -1);
        std::vector<RayCastResult> rayCast(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredObjects, const unsigned short group = -1, const unsigned short mask = -1);


        RayCastResult rayCastNearest(const btVector3& start, const btVector3& end, ComponentBody* ignoredObject = nullptr, const unsigned short group = -1, const unsigned short mask = -1);
        RayCastResult rayCastNearest(const btVector3& start, const btVector3& end, std::vector<ComponentBody*>& ignoredObjects, const unsigned short group = -1, const unsigned short mask = -1);

        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr, const unsigned short group = -1, const unsigned short mask = -1);
        RayCastResult rayCastNearest(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredObjects, const unsigned short group = -1, const unsigned short mask = -1);

        void setNumberOfStepsPerFrame(const unsigned int numSteps);
        const unsigned int getNumberOfStepsPerFrame();

        void setGravity(const float x, const float y, const float z);
        void setGravity(const glm::vec3& gravity);
        void pause(bool paused = true);
        void unpause();

        void addRigidBody(btRigidBody*, short group, short mask);
        void addRigidBody(btRigidBody*);
        void removeRigidBody(btRigidBody*);
        void updateRigidBody(btRigidBody*);
        void addRigidBody(ComponentBody&);
        void removeRigidBody(ComponentBody&);
        void removeCollisionObject(btCollisionObject* object);



        void addRigidBodyThreadSafe(btRigidBody*, short group, short mask);
        void addRigidBodyThreadSafe(btRigidBody*);
        void removeRigidBodyThreadSafe(btRigidBody*);
        void updateRigidBodyThreadSafe(btRigidBody*);
        void addRigidBodyThreadSafe(ComponentBody&);
        void removeRigidBodyThreadSafe(ComponentBody&);
        void removeCollisionObjectThreadSafe(btCollisionObject* object);
    };
};
#endif
