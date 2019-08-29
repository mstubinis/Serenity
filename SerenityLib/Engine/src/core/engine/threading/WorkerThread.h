#pragma once
#ifndef ENGINE_THREADING_WORKER_THREAD_H
#define ENGINE_THREADING_WORKER_THREAD_H

#include <thread>

namespace Engine {
    namespace epriv {
        class ThreadPool;
        class WorkerThread {
            private:
                std::thread*   m_Thread;
                bool           m_Stopped;
            public:
                WorkerThread();
                WorkerThread(ThreadPool&);
                ~WorkerThread();

                void join();
                void detach();

                static void Loop(WorkerThread& worker, ThreadPool&);
        };
    };
};

#endif