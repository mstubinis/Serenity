
#include <serenity/resources/Handle.h>
#include <serenity/resources/Engine_Resources.h>

[[nodiscard]] void* Handle::internal_get_base(uint32_t resourceTypeID) const noexcept {
    return null() ? nullptr : Engine::getResourceManager().m_ResourceModule.getVoid(resourceTypeID, m_Index);
}
[[nodiscard]] void* Handle::internal_get_base_thread_safe(uint32_t resourceTypeID) const noexcept {
    void* outPtr = nullptr;
    auto mutex = this->getMutex();
    assert(mutex != nullptr);
    {
        std::scoped_lock lock{ *mutex };
        outPtr = null() ? nullptr : Engine::getResourceManager().m_ResourceModule.getVoid(resourceTypeID, m_Index);
    }  
    return outPtr;
}

Engine::view_ptr<std::mutex> Handle::getMutex() const noexcept {
    return (null()) ? nullptr : std::addressof(Engine::getResourceManager().m_ResourceModule.getMutex(*this));
}

bool Handle::try_lock() const {
    auto mutex = this->getMutex();
    return mutex->try_lock();
}
void Handle::lock() const {
    auto mutex = this->getMutex();
    mutex->lock();
}
void Handle::unlock() const noexcept {
    auto mutex = this->getMutex();
    mutex->unlock();
}