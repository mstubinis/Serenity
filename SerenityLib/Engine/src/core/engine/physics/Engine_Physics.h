#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <glm/glm.hpp>
#include <memory>
#include <core/engine/Engine_Utils.h>
#include <LinearMath/btVector3.h>

struct Entity;
class  btCollisionShape;
class  btRigidBody;
class  btCollisionObject;
class  btHeightfieldTerrainShape;
class  Mesh;
class  ComponentModel;
namespace Engine{
namespace epriv{
    struct MeshImportedData;
    class GLDebugDrawer;
};
};
struct CollisionType{enum Type{
    None,
    Sphere,
    Box,
    ConvexHull, 
    TriangleShape,
    TriangleShapeStatic,
    Compound,
_TOTAL,};};
class Collision final{
    private:
        CollisionType::Type   m_Type;
        btVector3             m_Inertia;
        btCollisionShape*     m_Shape;

        void _init(const std::vector<Mesh*>& meshes, const float& mass);
        void _baseInit(const CollisionType::Type _type, const float& mass);
    public:
        Collision();
        Collision(const std::vector<Mesh*>& meshes, const float& mass = 0);
        Collision(ComponentModel&, const float& mass = 0);
        Collision(btHeightfieldTerrainShape&, const CollisionType::Type, const float& mass = 0);
        Collision(const CollisionType::Type, Mesh* mesh, const float& mass = 0);

        Collision(const Collision& other);
        Collision& operator=(const Collision& other);
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;


        ~Collision();

        void setMass(float mass);
        const btVector3& getInertia() const;
        btCollisionShape* getShape() const;
        const uint getType() const;
};

namespace Engine{
namespace epriv{
    class PhysicsManager final{
        public:
            class impl; std::unique_ptr<impl> m_i;

            PhysicsManager(const char* name, const uint& w, const uint& h);
            ~PhysicsManager();

            void _init(const char* name, const uint& w, const uint& h, const uint& numCores);

            void _update(const double& dt,int maxSteps = 1,float = 0.0166666f);
            void _render();
    };
};
namespace Physics{
    // vector[0] = end point, vector[1] = hit normal
    std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,btRigidBody* ignoredObject = nullptr);
    std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,std::vector<btRigidBody*>& ignoredObjects);

    std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignoredObject = nullptr);
    std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,std::vector<Entity>& ignoredObjects);

    void setGravity(const float x, const float y, const float z);
    void setGravity(const glm::vec3& gravity);
    void pause(bool=true);
    void unpause();
    void addRigidBody(btRigidBody*, short group, short mask);
    void addRigidBody(btRigidBody*);
    void removeRigidBody(btRigidBody*);
    void updateRigidBody(btRigidBody*);
};
};
#endif
