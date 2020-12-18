
#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/system/Engine.h>

Engine::view_ptr<std::mutex> Handle::getMutex() noexcept {
    return (null()) ? nullptr : Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.getMutex(*this);
}

void* Handle::internal_get_base() const noexcept {
    void* outPtr;
    Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get(outPtr, *this);
    return (null()) ? nullptr : outPtr;
}
void* Handle::internal_get_base_thread_safe() noexcept {
    void* outPtr = nullptr;
    {
        auto mutex = getMutex();
        ASSERT(mutex, __FUNCTION__ << "(): sharedMutex was nullptr!");
        //if (mutex) {
            std::lock_guard lock{ *mutex };
            Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get(outPtr, *this);
        //}
    }
    return (null()) ? nullptr : outPtr;
}