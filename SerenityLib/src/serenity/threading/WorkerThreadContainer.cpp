
#include <serenity/threading/WorkerThreadContainer.h>

using namespace Engine::priv;

WorkerThreadContainer::~WorkerThreadContainer() {

}
void WorkerThreadContainer::clear() noexcept {
    m_WorkerThreadsHashed.clear();
    m_WorkerThreads.clear();
}
void WorkerThreadContainer::reserve(size_t newReserveSize) noexcept {
    m_WorkerThreadsHashed.reserve(newReserveSize);
    m_WorkerThreads.reserve(newReserveSize);
}