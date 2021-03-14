#ifndef ENGINE_PHYSICS_TASK_SCHEDULER_H
#define ENGINE_PHYSICS_TASK_SCHEDULER_H
#pragma once

#include <atomic>
#include <LinearMath/btThreads.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace Engine::priv {
    class PhysicsTaskScheduler final : public btITaskScheduler {
        friend class PhysicsPipeline;
    protected:
        bool                   m_DoConcurrency = false;
        const char*            m_name;
        bool                   m_isActive = true;
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
}

#endif