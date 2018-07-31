#include "Collision.h"
#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include "Engine_MeshLoader.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#include <bullet/BulletCollision/Gimpact/btCompoundFromGimpact.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

using namespace Engine;
using namespace Engine::Physics;

Collision::Collision(btCollisionShape* shape,COLLISION_TYPE type, float mass){
    m_CollisionShape = shape;
    m_CollisionType = type;
    _init(type,mass);
}
Collision::Collision(ImportedMeshData& data,COLLISION_TYPE type, float mass){ 
    _load(data,type);
    _init(type,mass);
}
void Collision::_init(COLLISION_TYPE type, float mass){
    if(!m_Inertia){
        m_Inertia = new btVector3(0,0,0);
    }
    else{
        m_Inertia->setX(0);m_Inertia->setY(0);m_Inertia->setZ(0);
    }
    setMass(mass);
    Detail::PhysicsManagement::m_Collisions.push_back(this);
}
Collision::~Collision(){ 
    SAFE_DELETE(m_Inertia);
    SAFE_DELETE(m_InternalMeshData);
    SAFE_DELETE(m_CollisionShape);
    m_CollisionType = COLLISION_TYPE_NONE;
}
void Collision::_load(ImportedMeshData& data, COLLISION_TYPE collisionType){
    m_InternalMeshData = nullptr;
    btCollisionShape* shape = nullptr;
    switch(collisionType){
        case COLLISION_TYPE_CONVEXHULL:{
            shape = new btConvexHullShape();

            for(auto vertex:data.points){
                ((btConvexHullShape*)shape)->addPoint(btVector3(vertex.x,vertex.y,vertex.z));
            }

            m_CollisionShape = shape;
            m_CollisionType = COLLISION_TYPE_CONVEXHULL;
            break;
        }
        case COLLISION_TYPE_TRIANGLESHAPE:{
            m_InternalMeshData = new btTriangleMesh();

            for(auto triangle:data.file_triangles){
                glm::vec3 v1,v2,v3;

                v1 = triangle.v1.position;
                v2 = triangle.v2.position;
                v3 = triangle.v3.position;

                btVector3 bv1 = btVector3(v1.x,v1.y,v1.z);
                btVector3 bv2 = btVector3(v2.x,v2.y,v2.z);
                btVector3 bv3 = btVector3(v3.x,v3.y,v3.z);
                m_InternalMeshData->addTriangle(bv1, bv2, bv3,true);
            }

            shape = new btGImpactMeshShape(m_InternalMeshData);
            ((btGImpactMeshShape*)shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            ((btGImpactMeshShape*)shape)->setMargin(0.001f);
            ((btGImpactMeshShape*)shape)->updateBound();

            m_CollisionShape = shape;
            m_CollisionType = COLLISION_TYPE_TRIANGLESHAPE;
            break;
        }
        case COLLISION_TYPE_STATIC_TRIANGLESHAPE:{
            m_InternalMeshData = new btTriangleMesh();

            for(auto triangle:data.file_triangles){
                glm::vec3 v1Pos,v2Pos,v3Pos;

                v1Pos = triangle.v1.position;
                v2Pos = triangle.v2.position;
                v3Pos = triangle.v3.position;

                btVector3 v1 = btVector3(v1Pos.x,v1Pos.y,v1Pos.z);
                btVector3 v2 = btVector3(v2Pos.x,v2Pos.y,v2Pos.z);
                btVector3 v3 = btVector3(v3Pos.x,v3Pos.y,v3Pos.z);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
            }

            shape = new btBvhTriangleMeshShape(m_InternalMeshData,true);
            (shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            (shape)->setMargin(0.001f);

            m_CollisionShape = shape;
            m_CollisionType = COLLISION_TYPE_STATIC_TRIANGLESHAPE;
            break;
        }
        case COLLISION_TYPE_BOXSHAPE:{
            glm::vec3 max = glm::vec3(0);

            for(auto vertex:data.file_points){
                float x = abs(vertex.x); float y = abs(vertex.y); float z = abs(vertex.z);
                if(x > max.x) max.x = x; if(y > max.y) max.y = y; if(z > max.z) max.z = z;
            }

            shape = new btBoxShape(btVector3(max.x,max.y,max.z));
            m_CollisionShape = shape;
            m_CollisionType = COLLISION_TYPE_BOXSHAPE;
            break;
        }
    }
}
void Collision::setMass(float mass){
    if(!m_CollisionShape || m_CollisionType == COLLISION_TYPE_STATIC_TRIANGLESHAPE || m_CollisionType == COLLISION_TYPE_NONE) return;

    if(m_CollisionType != COLLISION_TYPE_TRIANGLESHAPE){
        m_CollisionShape->calculateLocalInertia(mass,*m_Inertia);
    }
    else{
        ((btGImpactMeshShape*)m_CollisionShape)->calculateLocalInertia(mass,*m_Inertia);
    }
}
