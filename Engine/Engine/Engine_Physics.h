#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "Engine_Utils.h"


class Entity;
class btCollisionDispatcher;
class btDynamicsWorld;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btRigidBody;
class btVector3;
class btTriangleMesh;
namespace Engine{
    namespace epriv{
        struct ImportedMeshData;
		class GLDebugDrawer;
    };
};
typedef float btScalar;
typedef unsigned int uint;

class CollisionType{public: enum Type{
    ConvexHull, 
    TriangleShape,
    TriangleShapeStatic,
    Compound,
    Box,
    Sphere,
    None,

_TOTAL,};};
class Collision final{
    private:
        btTriangleMesh* m_InternalMeshData;
        btVector3* m_Inertia;
        uint m_CollisionType;
        btCollisionShape* m_CollisionShape;
        void _init(CollisionType::Type = CollisionType::None, float mass = 0);
        void _load(Engine::epriv::ImportedMeshData&, CollisionType::Type,glm::vec3 scale = glm::vec3(1.0f));
    public:
        Collision(btCollisionShape* shape = nullptr,CollisionType::Type = CollisionType::None, float mass = 0);
        Collision(Engine::epriv::ImportedMeshData&,CollisionType::Type = CollisionType::None, float mass = 0,glm::vec3 scale = glm::vec3(1.0f));
        ~Collision();

        void setMass(float mass);
        btVector3* getInertia() const { return m_Inertia; }

        btCollisionShape* getCollisionShape() const { return m_CollisionShape; }
        const uint getCollisionType() const { return m_CollisionType; }
};

namespace Engine{
    namespace epriv{
        class PhysicsManager final: private Engine::epriv::noncopyable{
            public:
				class impl;
                std::unique_ptr<impl> m_i;

                PhysicsManager(const char* name,uint w,uint h);
                ~PhysicsManager();

                void _init(const char* name,uint w,uint h);

                void _update(float dt,int maxSteps = 1,float = 0.0166666f);
                void _render();

                void _removeCollision(Collision*);
                const btDiscreteDynamicsWorld* _world() const;
        };
    };





    namespace Physics{
        // vector[0] = end point, vector[1] = hit normal
        std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,btRigidBody* ignoredObject = nullptr);
        std::vector<glm::vec3> rayCast(const btVector3& start, const btVector3& end,std::vector<btRigidBody*>& ignoredObjects);

        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,Entity* ignoredObject = nullptr);
        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,std::vector<Entity*>& ignoredObjects);

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
