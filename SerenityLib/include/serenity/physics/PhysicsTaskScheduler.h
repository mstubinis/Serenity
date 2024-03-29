#ifndef ENGINE_PHYSICS_TASK_SCHEDULER_H
#define ENGINE_PHYSICS_TASK_SCHEDULER_H
#pragma once

#include <atomic>
#include <serenity/dependencies/glm.h>
#include <LinearMath/btThreads.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace Engine::priv {
    class PhysicsTaskScheduler final : public btITaskScheduler {
        friend class PhysicsPipeline;
        protected:
            std::atomic<btScalar>  m_sumRes;
            const char*            m_name;
            bool                   m_DoConcurrency = false;
            bool                   m_isActive      = true;
        public:
            PhysicsTaskScheduler(const char* name);
            ~PhysicsTaskScheduler();

            inline const char* getName() const noexcept { return m_name; }

            int getMaxNumThreads() const override;
            int getNumThreads() const override;
            void setNumThreads(int numThreads) override;
            void parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody&) override;
            btScalar parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody&) override;
            void sleepWorkerThreadsHint() override {}

            // internal use only
            void activate() override { m_isActive = true; }
            void deactivate() override { m_isActive = false; }
    };
}

#endif