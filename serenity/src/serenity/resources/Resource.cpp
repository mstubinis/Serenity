
#include <serenity/resources/Resource.h>
#include <serenity/utils/Logger.h>
#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

ResourceBaseClass::ResourceBaseClass(ResourceType type) noexcept
    : m_ResourceType{ type }
{}
ResourceBaseClass::ResourceBaseClass(ResourceType type, std::string_view name) noexcept
    : ResourceBaseClass{ type }
{
    m_Name = name;
}
ResourceBaseClass::ResourceBaseClass(ResourceBaseClass&& other) noexcept
    : m_IsLoaded     { std::exchange(other.m_IsLoaded, false) }
    , m_Name         { std::move(other.m_Name) }
    , m_ResourceType { std::move(other.m_ResourceType) }
{}
ResourceBaseClass& ResourceBaseClass::operator=(ResourceBaseClass&& other) noexcept {
    m_IsLoaded     = std::exchange(other.m_IsLoaded, false);
    m_Name         = std::move(other.m_Name);
    m_ResourceType = std::move(other.m_ResourceType);
    return *this;
}

void ResourceBaseClass::load(bool dispatchEventLoaded) {
    if (!m_IsLoaded) {
        m_IsLoaded = true;
        if (dispatchEventLoaded && Engine::priv::Core::m_Engine) {
            Event e{ EventType::ResourceLoaded };
            e.eventResource = Engine::priv::EventResource{ this };
            Engine::priv::Core::m_Engine->m_EventDispatcher.dispatchEvent(std::move(e));
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
    }
}
void ResourceBaseClass::unload(bool dispatchEventUnloaded) {
    if (m_IsLoaded) {
        m_IsLoaded = false;
        if (dispatchEventUnloaded && Engine::priv::Core::m_Engine) {
            Event e{ EventType::ResourceUnloaded };
            e.eventResource = Engine::priv::EventResource{ this };
            Engine::priv::Core::m_Engine->m_EventDispatcher.dispatchEvent(std::move(e));
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}



