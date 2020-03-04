#pragma once
#ifndef ENGINE_PHYSICS_WORLD_H
#define ENGINE_PHYSICS_WORLD_H

class btSequentialImpulseConstraintSolverMt;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionConfiguration;
class btConstraintSolver;
class btBroadphaseInterface;
class btDispatcher;
class btCollisionShape;
class btTransform;
class btVector3;
class btDiscreteDynamicsWorld;
class btPersistentManifold;
//struct btCollisionAlgorithmConstructionInfo;
//struct btCollisionObjectWrapper;

#include <core/engine/physics/DebugDrawer.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h>

#include <BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>
//#include <BulletCollision/CollisionDispatch/btCollisionCreateFunc.h>
//#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
//#include <BulletCollision/CollisionDispatch/SphereTriangleDetector.h>

namespace Engine::priv {
    class PhysicsWorld final : public Engine::NonCopyable, public Engine::NonMoveable{
        public:
            btBroadphaseInterface*                  m_Broadphase;
            btDefaultCollisionConfiguration*        m_CollisionConfiguration;
            btCollisionDispatcher*                  m_Dispatcher;
            btSequentialImpulseConstraintSolver*    m_Solver;
            btSequentialImpulseConstraintSolverMt*  m_SolverMT;
            btDiscreteDynamicsWorld*                m_World;
            GLDebugDrawer                           m_DebugDrawer;
        public:
            PhysicsWorld();
            ~PhysicsWorld();
    };
};
#endif