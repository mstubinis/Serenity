#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <glm/vec3.hpp>
#include <memory>
#include <core/engine/utils/Utils.h>
#include <LinearMath/btVector3.h>

struct Entity;
class  btRigidBody;
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
namespace Physics{
    // vector[0] = end point, vector[1] = hit normal
    std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end, btRigidBody* ignoredObject = nullptr);
    std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end, std::vector<btRigidBody*>& ignoredObjects);

    std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr);
    std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredObjects);

    void setGravity(const float x, const float y, const float z);
    void setGravity(const glm::vec3& gravity);
    void pause(bool paused = true);
    void unpause();
    void addRigidBody(btRigidBody*, short group, short mask);
    void addRigidBody(btRigidBody*);
    void removeRigidBody(btRigidBody*);
    void updateRigidBody(btRigidBody*);
};
};
#endif
