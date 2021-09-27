
#include <serenity/resources/Handle.h>
#include <serenity/system/Engine.h>

Engine::view_ptr<std::mutex> Handle::getMutex() noexcept {
    return (null()) ? nullptr : &Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.getMutex(*this);
}

void* Handle::internal_get_base() const noexcept {
    return Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.getVoid(*this);
}
void* Handle::internal_get_base_thread_safe() noexcept {
    void* outPtr = nullptr;
    {
        auto mutex = getMutex();
        ASSERT(mutex, __FUNCTION__ << "(): mutex was nullptr!");
        std::lock_guard lock{ *mutex };
        outPtr = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.getVoid(*this);
    }
    return outPtr;
}