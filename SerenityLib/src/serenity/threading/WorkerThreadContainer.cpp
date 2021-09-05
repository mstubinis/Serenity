
#include <serenity/threading/WorkerThreadContainer.h>

void Engine::priv::WorkerThreadContainer::clear() noexcept {
    m_WorkerThreadsHashed.clear();
    m_WorkerThreads.clear();
}
void Engine::priv::WorkerThreadContainer::reserve(size_t newReserveSize) noexcept {
    m_WorkerThreadsHashed.reserve(newReserveSize);
    m_WorkerThreads.reserve(newReserveSize);
}