
#include <serenity/resources/Resource.h>
#include <serenity/utils/Logger.h>
#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

ResourceBaseClass::ResourceBaseClass(ResourceType type)
    : m_ResourceType{ type }
{}
ResourceBaseClass::ResourceBaseClass(ResourceType type, std::string_view name)
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

void ResourceBaseClass::load() {
    if(!m_IsLoaded){
        m_IsLoaded = true;
        if (Engine::priv::Core::m_Engine) {
            Event e(EventType::ResourceLoaded);
            e.eventResource = Engine::priv::EventResource{ this };
            Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
    }
}
void ResourceBaseClass::unload() {
    if(m_IsLoaded /*&& m_UsageCount == 0*/){
        m_IsLoaded = false;
        if (Engine::priv::Core::m_Engine) {
            Event e(EventType::ResourceUnloaded);
            e.eventResource = Engine::priv::EventResource{ this };
            Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}



