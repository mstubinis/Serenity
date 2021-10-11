#include <serenity/threading/SpinLockGuard.h>
#include <serenity/threading/SpinLock.h>

Engine::SpinLockGuard::SpinLockGuard(Engine::SpinLock& inLock, bool yield) 
    : m_Lock { inLock }
{
    m_Lock.lock(yield);
}

Engine::SpinLockGuard::~SpinLockGuard() {
    m_Lock.unlock();
}