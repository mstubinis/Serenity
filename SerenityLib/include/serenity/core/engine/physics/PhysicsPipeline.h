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

#include <serenity/core/engine/physics/DebugDrawer.h>
#include <LinearMath/btThreads.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace Engine::priv {
    class PhysicsTaskScheduler final : public btITaskScheduler {
        friend class PhysicsPipeline;
        protected:
            bool                   m_DoConcurrency = false;
            const char*            m_name;
            bool                   m_isActive      = true;
            std::atomic<btScalar>  m_sumRes;
        public:
            PhysicsTaskScheduler(const char* name);
            ~PhysicsTaskScheduler();

            inline const char* getName() const noexcept { return m_name; }

            int getMaxNumThreads() const override;
            int getNumThreads() const override;
            void setNumThreads(int numThreads) override;
            void parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) override;
            btScalar parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) override;
            void sleepWorkerThreadsHint() override {}

            // internal use only
            void activate() override { m_isActive = true; }
            void deactivate() override { m_isActive = false; }
    };
    class PhysicsPipeline final {
        public:
            btInternalTickCallback                                         m_PreTickCallback  = [](btDynamicsWorld*, btScalar) {};
            btInternalTickCallback                                         m_PostTickCallback = [](btDynamicsWorld*, btScalar) {};

            std::unique_ptr<PhysicsTaskScheduler>                          m_TaskScheduler;
            std::unique_ptr<btBroadphaseInterface>                         m_Broadphase;
            std::unique_ptr<btDefaultCollisionConfiguration>               m_CollisionConfiguration;
            std::unique_ptr<btCollisionDispatcher>                         m_Dispatcher;
            std::unique_ptr<btConstraintSolverPoolMt>                      m_SolverPool;
            std::unique_ptr<btSequentialImpulseConstraintSolver>           m_Solver;
            std::unique_ptr<btSequentialImpulseConstraintSolverMt>         m_SolverMT;
            std::unique_ptr<btDiscreteDynamicsWorld>                       m_World;
            GLDebugDrawer                                                  m_DebugDrawer;
        public:
            PhysicsPipeline();
            ~PhysicsPipeline();

            void cleanup();

            void update(const float dt);

            void setPreTickCallback(btInternalTickCallback preTicCallback);
            void setPostTickCallback(btInternalTickCallback postTickCallback);
    };
};
#endif