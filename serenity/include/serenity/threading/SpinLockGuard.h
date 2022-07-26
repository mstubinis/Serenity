#pragma once
#ifndef ENGINE_THREADING_SPIN_LOCK_GUARD_H
#define ENGINE_THREADING_SPIN_LOCK_GUARD_H

namespace Engine {
    class SpinLock;
}

#include <atomic>

namespace Engine {
    class SpinLockGuard {
        private:
            Engine::SpinLock& m_Lock;

            SpinLockGuard() = delete;
        public:
            SpinLockGuard(Engine::SpinLock&, bool yield = true);
            SpinLockGuard(const SpinLockGuard&)                = delete;
            SpinLockGuard& operator=(const SpinLockGuard&)     = delete;
            SpinLockGuard(SpinLockGuard&&) noexcept            = delete;
            SpinLockGuard& operator=(SpinLockGuard&&) noexcept = delete;
            ~SpinLockGuard();
    };
}

#endif