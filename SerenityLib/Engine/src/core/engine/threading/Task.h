#pragma once
#ifndef ENGINE_THREADING_TASK_H
#define ENGINE_THREADING_TASK_H

namespace Engine::priv {
    class Task final {
        private:
            std::packaged_task<void()> m_Task;
        public:
            Task(std::function<void()>&& inTask)
                : m_Task{ std::move(inTask) }
            {}

            inline void operator()() noexcept {
                ASSERT(m_Task.valid(), "Engine::priv::Task::operator()(): m_Task was not valid!");
                if (m_Task.valid())
                    m_Task();
            }
            inline std::future<void> get_future() noexcept { return m_Task.get_future(); }
    };
};

#endif