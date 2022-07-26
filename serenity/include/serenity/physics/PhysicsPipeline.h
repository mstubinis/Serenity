#pragma once
#ifndef ENGINE_PHYSICS_WORLD_H
#define ENGINE_PHYSICS_WORLD_H

class  Scene;
class  btDynamicsWorld;
class  btConstraintSolverPoolMt;
class  btSequentialImpulseConstraintSolverMt;
class  btSequentialImpulseConstraintSolver;
class  btDefaultCollisionConfiguration;
class  btCollisionDispatcher;
class  btCollisionConfiguration;
class  btConstraintSolver;
class  btBroadphaseInterface;
class  btDispatcher;
class  btCollisionShape;
class  btTransform;
class  btVector3;
class  btDiscreteDynamicsWorld;
class  btPersistentManifold;
class  BulletWorld;
namespace Engine::priv {
    class  PhysicsTaskScheduler;
    class  PhysicsPipeline;
};

#include <serenity/physics/DebugDrawer.h>
#include <serenity/physics/PhysicsTaskScheduler.h>
#include <LinearMath/btThreads.h>

namespace Engine::priv {
    class PhysicsPipeline final {
        public:
            btInternalTickCallback                                     m_PreTickCallback  = [](btDynamicsWorld*, btScalar) {};
            btInternalTickCallback                                     m_PostTickCallback = [](btDynamicsWorld*, btScalar) {};

            std::unique_ptr<PhysicsTaskScheduler>                      m_TaskScheduler;
            std::unique_ptr<btBroadphaseInterface>                     m_Broadphase;
            std::unique_ptr<btDefaultCollisionConfiguration>           m_CollisionConfiguration;
            std::unique_ptr<btCollisionDispatcher>                     m_Dispatcher;
            std::unique_ptr<btConstraintSolverPoolMt>                  m_SolverPool;
            std::unique_ptr<btSequentialImpulseConstraintSolver>       m_Solver;
            std::unique_ptr<btSequentialImpulseConstraintSolverMt>     m_SolverMT;
            std::unique_ptr<btDiscreteDynamicsWorld>                   m_World;
            GLDebugDrawer                                              m_DebugDrawer;
        public:
            PhysicsPipeline();
            ~PhysicsPipeline();

            void init();

            void drawLine(const glm::vec3& start, const glm::vec3& end, float r, float g, float b);

            void cleanup();

            void setPreTickCallback(btInternalTickCallback);
            void setPostTickCallback(btInternalTickCallback);
    };
};
#endif