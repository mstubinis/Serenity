#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/system/Engine.h>

Engine::view_ptr<std::shared_mutex> Handle::getMutex() noexcept {
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
        auto sharedMutex = getMutex();
        ASSERT(sharedMutex, __FUNCTION__ << "(): sharedMutex was nullptr!");
        //if (sharedMutex) {
            std::shared_lock lock(*sharedMutex);
            Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get(outPtr, *this);
        //}
    }
    return (null()) ? nullptr : outPtr;
}