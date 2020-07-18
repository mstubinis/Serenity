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
    struct RayCastResult {
        glm::vec3 hitPosition = glm::vec3(0.0f);
        glm::vec3 hitNormal   = glm::vec3(0.0f);
    };
    namespace priv{
        class PhysicsManager final{
            public:
                Engine::priv::PhysicsPipeline    m_Pipeline;
                std::mutex                       m_Mutex;
                bool                             m_Paused                = false;
                unsigned int                     m_NumberOfStepsPerFrame = 1;
            public:
                PhysicsManager();
                ~PhysicsManager();

                void debug_draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
                void debug_draw_line(const glm::vec3& start, const glm::vec3& end, float r, float g, float b, float a);

                void cleanup();

                void _init();

                void _update(const float dt, int maxSubSteps = 1, float fixedTimeStep = 0.0166666f);
                void _render(const Camera& camera);
        };
    };
    namespace Physics{
        std::vector<RayCastResult> rayCast(btVector3& start, btVector3& end, ComponentBody* ignoredObject = nullptr, unsigned short group = -1, unsigned short mask = -1);
        std::vector<RayCastResult> rayCast(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignoredObjects, unsigned short group = -1, unsigned short mask = -1);

        std::vector<RayCastResult> rayCast(glm::vec3& start, glm::vec3& end, Entity* ignoredObject = nullptr, unsigned short group = -1, unsigned short mask = -1);
        std::vector<RayCastResult> rayCast(glm::vec3& start, glm::vec3& end, std::vector<Entity>& ignoredObjects, unsigned short group = -1, unsigned short mask = -1);


        RayCastResult rayCastNearest(btVector3& start, btVector3& end, ComponentBody* ignoredObject = nullptr, unsigned short group = -1, unsigned short mask = -1);
        RayCastResult rayCastNearest(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignoredObjects, unsigned short group = -1, unsigned short mask = -1);

        RayCastResult rayCastNearest(glm::vec3& start, glm::vec3& end, Entity* ignoredObject = nullptr, unsigned short group = -1, unsigned short mask = -1);
        RayCastResult rayCastNearest(glm::vec3& start, glm::vec3& end, std::vector<Entity>& ignoredObjects, unsigned short group = -1, unsigned short mask = -1);

        void setNumberOfStepsPerFrame(unsigned int numSteps);
        unsigned int getNumberOfStepsPerFrame();

        void setGravity(float x, float y, float z);
        void setGravity(const glm::vec3& gravity);
        void pause(bool paused = true);
        void unpause();

        bool addRigidBody(Entity entity);
        void addRigidBody(ComponentBody&);
        void addRigidBody(btRigidBody*, short group, short mask);
        void addRigidBody(btRigidBody*);
        bool removeRigidBody(Entity entity);
        void removeRigidBody(btRigidBody*);
        void removeRigidBody(ComponentBody&);
        void removeCollisionObject(btCollisionObject* object);

        void updateRigidBody(btRigidBody*);

        bool addRigidBodyThreadSafe(Entity entity);
        void addRigidBodyThreadSafe(ComponentBody&);
        void addRigidBodyThreadSafe(btRigidBody*, short group, short mask);
        void addRigidBodyThreadSafe(btRigidBody*);
        bool removeRigidBodyThreadSafe(Entity entity);
        void removeRigidBodyThreadSafe(btRigidBody*);
        void removeRigidBodyThreadSafe(ComponentBody&);
        void removeCollisionObjectThreadSafe(btCollisionObject* object);

        void updateRigidBodyThreadSafe(btRigidBody*);
    };
};
#endif
