#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <glm/glm.hpp>
#include <memory>
#include "core/engine/Engine_Utils.h"
#include <Bullet/LinearMath/btVector3.h>

class OLD_Entity;
class btCollisionShape;
class btRigidBody;
class btCollisionObject;
class Mesh;
class OLD_ComponentModel;
class ComponentModel;
namespace Engine{
    namespace epriv{
        struct ImportedMeshData;
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
        btCollisionShape*     m_Shape;
        btVector3             m_Inertia;

        void _init(std::vector<Mesh*>& meshes, float mass);
        void _baseInit(CollisionType::Type _type, float& mass);
    public:
        Collision();
        Collision(std::vector<Mesh*>& meshes, float mass = 0);
        Collision(OLD_ComponentModel&, float mass = 0);
        Collision(ComponentModel&, float mass = 0);
        Collision(CollisionType::Type, Mesh* mesh, float mass = 0);

        Collision(const Collision& other) = delete;
        Collision& operator=(const Collision& other) = delete;
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

                PhysicsManager(const char* name,uint w,uint h);
                ~PhysicsManager();

                void _init(const char* name,uint w,uint h,uint numCores);

                void _update(float dt,int maxSteps = 1,float = 0.0166666f);
                void _render();
        };
    };
    namespace Physics{
        // vector[0] = end point, vector[1] = hit normal
        std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,btRigidBody* ignoredObject = nullptr);
        std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,std::vector<btRigidBody*>& ignoredObjects);

        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end, OLD_Entity* ignoredObject = nullptr);
        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,std::vector<OLD_Entity*>& ignoredObjects);

        void setGravity(float,float,float); 
        void setGravity(glm::vec3&);
        void pause(bool=true);
        void unpause();
        void addRigidBody(btRigidBody*, short group, short mask);
        void addRigidBody(btRigidBody*);
        void removeRigidBody(btRigidBody*);
        void updateRigidBody(btRigidBody*);
    };
};
#endif
