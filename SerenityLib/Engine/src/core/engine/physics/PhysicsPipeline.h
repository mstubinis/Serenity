#pragma once
#ifndef ENGINE_PHYSICS_WORLD_H
#define ENGINE_PHYSICS_WORLD_H

class btDynamicsWorld;
class btConstraintSolverPoolMt;
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
namespace Engine::priv {
    class PhysicsTaskScheduler;
    class PhysicsPipeline;
};

#include <functional>
#include <core/engine/physics/DebugDrawer.h>
#include <LinearMath/btThreads.h>
#include <atomic>

namespace Engine::priv {
    class PhysicsTaskScheduler final : public btITaskScheduler, public Engine::NonCopyable, public Engine::NonMoveable {
        friend class PhysicsPipeline;
        protected:
            bool                   m_DoConcurrency = false;
            const char*            m_name;
            bool                   m_isActive = true;
            std::atomic<btScalar>  m_sumRes;
        public:
            PhysicsTaskScheduler(const char* name);
            ~PhysicsTaskScheduler();
            const char* getName() const;

            int getMaxNumThreads() const override;
            int getNumThreads() const override;
            void setNumThreads(int numThreads) override;
            void parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) override;
            btScalar parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) override;
            void sleepWorkerThreadsHint() override;

            // internal use only
            void activate() override;
            void deactivate() override;
    };
    class PhysicsPipeline final : public Engine::NonCopyable, public Engine::NonMoveable {
        public:
            std::function<void(btDynamicsWorld* world, btScalar timeStep)> m_PreTickCallback  = [](btDynamicsWorld*, btScalar) {};
            std::function<void(btDynamicsWorld* world, btScalar timeStep)> m_PostTickCallback = [](btDynamicsWorld*, btScalar) {};

            PhysicsTaskScheduler*                                          m_TaskScheduler          = nullptr;
            btBroadphaseInterface*                                         m_Broadphase             = nullptr;
            btDefaultCollisionConfiguration*                               m_CollisionConfiguration = nullptr;
            btCollisionDispatcher*                                         m_Dispatcher             = nullptr;
            btConstraintSolverPoolMt*                                      m_SolverPool             = nullptr;
            btSequentialImpulseConstraintSolver*                           m_Solver                 = nullptr;
            btSequentialImpulseConstraintSolverMt*                         m_SolverMT               = nullptr;
            btDiscreteDynamicsWorld*                                       m_World                  = nullptr;
            GLDebugDrawer                                                  m_DebugDrawer;
        public:
            PhysicsPipeline();
            ~PhysicsPipeline();

            void update(const float dt);

            void setPreTickCallback(std::function<void(btDynamicsWorld* world, btScalar timeStep)> preTicCallback);
            void setPostTickCallback(std::function<void(btDynamicsWorld* world, btScalar timeStep)> postTickCallback);
    };
};
#endif