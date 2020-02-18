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

#include <core/engine/physics/DebugDrawer.h>

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