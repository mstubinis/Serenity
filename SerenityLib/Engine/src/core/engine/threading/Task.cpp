#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/threading/Task.h>

using namespace Engine::priv;

Task::Task(std::function<void()>&& inTask) {
    m_Task = std::packaged_task<void()>(std::move(inTask));
}
Task::~Task() {
}