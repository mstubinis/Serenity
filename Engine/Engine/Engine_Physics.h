#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include "Engine_Math.h"

class Object;
class ObjectDynamic;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;
class btDiscreteDynamicsWorld;
class GLDebugDrawer;
class btCollisionShape;
class btRigidBody;
class btVector3;
class btTriangleMesh;
struct ImportedMeshData;

typedef float btScalar;
typedef unsigned int uint;

enum CollisionType { 
    ConvexHull, 
    TriangleShape,
    TriangleShapeStatic,
    Compund,
    Box,
    None
};
class Collision final{
    private:
        btTriangleMesh* m_InternalMeshData;
        btVector3* m_Inertia;
        uint m_CollisionType;
        btCollisionShape* m_CollisionShape;
		void _init(CollisionType = CollisionType::None, float mass = 0);
        void _load(ImportedMeshData&, CollisionType);
    public:
		Collision(btCollisionShape* shape = nullptr,CollisionType = CollisionType::None, float mass = 0);
		Collision(ImportedMeshData&,CollisionType = CollisionType::None, float mass = 0);
        ~Collision();

        void setMass(float mass);
        btVector3* getInertia() const { return m_Inertia; }

        btCollisionShape* getCollisionShape() const { return m_CollisionShape; }
        const uint getCollisionType() const { return m_CollisionType; }
};

namespace Engine{
    namespace Physics{
        namespace Detail{
            class PhysicsManagement final{
                private:
                    static void _preTicCallback(btDynamicsWorld* world, btScalar timeStep);
                    static void _postTicCallback(btDynamicsWorld* world, btScalar timeStep);
                public:
                    static btBroadphaseInterface* m_broadphase;
                    static btDefaultCollisionConfiguration* m_collisionConfiguration;
                    static btCollisionDispatcher* m_dispatcher;
                    static btSequentialImpulseConstraintSolver* m_solver;
                    static btDiscreteDynamicsWorld* m_world;

                    static GLDebugDrawer* m_debugDrawer;

                    static void init();
                    static void destruct();
                    static void update(float dt,uint maxSteps = 1,float = 1/60.0f);
                    static void render();

                    static std::vector<glm::v3> rayCastInternal(const btVector3& start, const btVector3& end);

                    static std::vector<Collision*> m_Collisions;
            };
        };

        // vector[0] = end point, vector[1] = hit normal
        std::vector<glm::v3> rayCast(const btVector3& start, const btVector3& end,btRigidBody* ignoredObject = nullptr);
        std::vector<glm::v3> rayCast(const btVector3& start, const btVector3& end,std::vector<btRigidBody*> ignoredObjects);

        std::vector<glm::v3> rayCast(const glm::v3& start, const glm::v3& end,Object* ignoredObject = nullptr);
        std::vector<glm::v3> rayCast(const glm::v3& start, const glm::v3& end,std::vector<Object*> ignoredObjects);

        void setGravity(float,float,float); 
        void setGravity(glm::vec3&);
        void addRigidBody(btRigidBody*, short group, short mask);
        void addRigidBody(ObjectDynamic*);
        void addRigidBody(btRigidBody*);
        void removeRigidBody(btRigidBody*);
        void removeRigidBody(ObjectDynamic*);
    };
};


#include <bullet/LinearMath/btIDebugDraw.h>

class GLDebugDrawer : public btIDebugDraw{
    int m_debugMode;
public:
    GLDebugDrawer();
    virtual ~GLDebugDrawer(); 
    virtual void    drawLine(const btVector3& from,const btVector3& to,const btVector3& fromColor, const btVector3& toColor);
    virtual void    drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
    virtual void    drawSphere (const btVector3& p, btScalar radius, const btVector3& color);
    virtual void    drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha);
    virtual void    drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
    virtual void    reportErrorWarning(const char* warningString);
    virtual void    draw3dText(const btVector3& location,const char* textString);
    virtual void    setDebugMode(int debugMode);
    virtual int     getDebugMode() const { return m_debugMode;}
};

void GLDebugDrawStringInternal(int x,int y,const char* string,const btVector3& rgb, bool enableBlend, int spacing);
void GLDebugDrawStringInternal(int x,int y,const char* string,const btVector3& rgb);
void GLDebugDrawString(int x,int y,const char* string);
void GLDebugResetFont(int screenWidth,int screenHeight);

#endif