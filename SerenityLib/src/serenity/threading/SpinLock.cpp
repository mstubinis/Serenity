#include <serenity/threading/SpinLock.h>
#include <thread>

Engine::SpinLock::~SpinLock() {
    unlock();
}

void Engine::SpinLock::lock(bool yield) {
    for (;;) {
        if (!m_Locked.exchange(true, std::memory_order_acquire)) {
            return;
        }
        while (m_Locked.load(std::memory_order_relaxed)) {
            if (yield) {
                std::this_thread::yield();
            } else {
                ;
            }
        }
    }
}
void Engine::SpinLock::unlock() {
    m_Locked.store(false, std::memory_order_release);
}