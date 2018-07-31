#pragma once
#ifndef ENGINE_COLLISION_H
#define ENGINE_COLLISION_H


class btTriangleMesh;
class btVector3;
class btCollisionShape;
class Collision;
struct ImportedMeshData;

typedef unsigned int uint;

enum COLLISION_TYPE { 
    COLLISION_TYPE_CONVEXHULL, 
    COLLISION_TYPE_TRIANGLESHAPE,
    COLLISION_TYPE_STATIC_TRIANGLESHAPE,
    COLLISION_TYPE_COMPOUND,
    COLLISION_TYPE_BOXSHAPE,
    COLLISION_TYPE_NONE
};

class Collision final{
    private:
        btTriangleMesh* m_InternalMeshData;
        btVector3* m_Inertia;
        uint m_CollisionType;
        btCollisionShape* m_CollisionShape;
        void _init(COLLISION_TYPE = COLLISION_TYPE_NONE, float mass = 0);
        void _load(ImportedMeshData&, COLLISION_TYPE);
    public:
        Collision(btCollisionShape* shape = nullptr,COLLISION_TYPE = COLLISION_TYPE_NONE, float mass = 0);
        Collision(ImportedMeshData&,COLLISION_TYPE = COLLISION_TYPE_NONE, float mass = 0);
        ~Collision();

        void setMass(float mass);
        btVector3* getInertia() const { return m_Inertia; }

        btCollisionShape* getCollisionShape() const { return m_CollisionShape; }
        const uint getCollisionType() const { return m_CollisionType; }
};

#endif
