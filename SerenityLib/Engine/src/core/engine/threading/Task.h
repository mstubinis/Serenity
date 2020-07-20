#pragma once
#ifndef ENGINE_THREADING_TASK_H
#define ENGINE_THREADING_TASK_H

namespace Engine::priv {
    class Task final {
        private:
            std::packaged_task<void()> m_Task;
        public:
            Task(std::function<void()>&& inTask);
            ~Task();

            void operator()() noexcept {
                //if (m_Task.valid()) {
                m_Task();
                //}
            }
            inline std::future<void> get_future() noexcept {
                return m_Task.get_future();
            }
    };
};

#endif