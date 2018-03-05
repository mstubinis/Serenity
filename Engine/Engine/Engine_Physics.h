#pragma once
#ifndef ENGINE_ENGINE_PHYSICS_H
#define ENGINE_ENGINE_PHYSICS_H

#include <glm/fwd.hpp>
#include <vector>
#include <memory>

class Object;
class ObjectDynamic;
class btCollisionDispatcher;
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
        void _load(ImportedMeshData&, CollisionType::Type);
    public:
        Collision(btCollisionShape* shape = nullptr,CollisionType::Type = CollisionType::None, float mass = 0);
        Collision(ImportedMeshData&,CollisionType::Type = CollisionType::None, float mass = 0);
        ~Collision();

        void setMass(float mass);
        btVector3* getInertia() const { return m_Inertia; }

        btCollisionShape* getCollisionShape() const { return m_CollisionShape; }
        const uint getCollisionType() const { return m_CollisionType; }
};

namespace Engine{
	namespace epriv{
		class PhysicsManager final{
		    private:
				class impl;
		    public:
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

        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,Object* ignoredObject = nullptr);
        std::vector<glm::vec3> rayCast(const glm::vec3& start, const glm::vec3& end,std::vector<Object*>& ignoredObjects);

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
