#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/WorkerThreadContainer.h>

using namespace Engine::priv;

WorkerThreadContainer::WorkerThreadContainer() {

}
WorkerThreadContainer::~WorkerThreadContainer() {
    for (auto& worker_thread : m_WorkerThreads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}
void WorkerThreadContainer::clear() noexcept {
    m_WorkerThreadsHashed.clear();
    m_WorkerThreads.clear();
}
void WorkerThreadContainer::reserve(size_t newReserveSize) noexcept {
    m_WorkerThreads.reserve(newReserveSize);
    m_WorkerThreadsHashed.reserve(newReserveSize);
}