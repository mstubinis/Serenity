#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <core/engine/utils/Utils.h>
#include <glm/vec3.hpp>
#include <memory>
#include <LinearMath/btVector3.h>

struct Entity;
class  btRigidBody;
class  ComponentBody;
class  Camera;
namespace Engine{
    namespace epriv{
        struct MeshImportedData;
        class GLDebugDrawer;
    };
};

namespace Engine{
namespace epriv{
    class PhysicsManager final{
        public:
            class impl; std::unique_ptr<impl> m_i;

            PhysicsManager();
            ~PhysicsManager();

            void _init(const char* name, const uint& w, const uint& h, const uint& numCores);

            void _update(const double& dt, int maxSteps = 1, float = 0.0166666f);
            void _render(Camera& camera);
    };
};

struct RayCastResult {
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    RayCastResult() {
        hitNormal = hitPosition = glm::vec3(0.0f);
    }
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
