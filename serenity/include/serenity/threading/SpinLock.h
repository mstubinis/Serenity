#pragma once
#ifndef ENGINE_THREADING_SPIN_LOCK_H
#define ENGINE_THREADING_SPIN_LOCK_H

#include <atomic>

namespace Engine {
    class SpinLock {
        private:
            std::atomic_bool m_Locked = false;
        public:
            SpinLock() = default;
            SpinLock(const SpinLock&)                = delete;
            SpinLock& operator=(const SpinLock&)     = delete;
            SpinLock(SpinLock&&) noexcept            = delete;
            SpinLock& operator=(SpinLock&&) noexcept = delete;
            ~SpinLock();

            void lock(bool yield = true);
            void unlock();
    };
}

#endif