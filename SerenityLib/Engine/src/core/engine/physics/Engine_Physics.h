#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

struct Entity;
class  btRigidBody;
class  ComponentBody;
class  Camera;
namespace Engine {
    namespace epriv {
        struct MeshImportedData;
        class  GLDebugDrawer;
        class  PhysicsWorld;
    };
};

#include <core/engine/math/Numbers.h>
#include <vector>
#include <LinearMath/btVector3.h>

namespace Engine{
    struct RayCastResult {
        glm::vec3 hitPosition;
        glm::vec3 hitNormal;
        RayCastResult() {
            hitNormal = hitPosition = glm::vec3(0.0f);
        }
    };
    namespace epriv{
        class PhysicsManager final{
            public:
                Engine::epriv::PhysicsWorld*  m_Data;
                bool                          m_Paused;
                unsigned int                  m_NumberOfStepsPerFrame;
            public:
                PhysicsManager();
                ~PhysicsManager();

                void cleanup();

                void _init();

                void _update(const double& dt, int maxSteps = 1, float = 0.0166666f);
                void _render(Camera& camera);
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
    };
};
#endif
