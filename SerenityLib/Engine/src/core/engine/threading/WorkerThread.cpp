#include <core/engine/threading/WorkerThread.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/threading/Queue_ThreadSafe.h>
#include <core/engine/utils/Utils.h>

#include <future>
#include <queue>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;

WorkerThread::WorkerThread() {
    m_Thread = nullptr;
    m_Stopped = true;
}
WorkerThread::WorkerThread(ThreadPool& pool) {
    m_Thread = NEW std::thread(&WorkerThread::Loop, std::ref(*this), std::ref(pool));
    m_Stopped = false;
}
WorkerThread::~WorkerThread() {
    SAFE_DELETE_THREAD(m_Thread);
}

void WorkerThread::join() {
    if (m_Thread && m_Thread->joinable()) {
        m_Thread->join();
    }
}
void WorkerThread::detach() {
    if (m_Thread && m_Thread->joinable()) {
        m_Thread->detach();
    }
}
void WorkerThread::Loop(WorkerThread& worker, ThreadPool& pool) {
    while (!worker.m_Stopped) {
        std::shared_ptr<std::packaged_task<void()>> job;
        {
            std::unique_lock<std::mutex> lock(pool.m_Mutex);
            pool.m_Condition.wait(lock, [&] {
                if (pool.m_Terminated) {
                    worker.m_Stopped = true;
                    return true;
                }
                if (!pool.m_Tasks.empty()) {
                    return true;
                }
                return false;
                });
            if (worker.m_Stopped) {
                return;
            }
            job = pool.m_Tasks.front();
            pool.m_Tasks.pop();
        }
        (*job)();
    }
}