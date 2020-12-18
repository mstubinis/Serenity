
#include <serenity/core/engine/resources/Resource.h>
#include <serenity/core/engine/utils/Logger.h>
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/events/Event.h>

Resource::Resource(ResourceType type)
    : m_ResourceType{ type }
{}
Resource::Resource(ResourceType type, const std::string& name)
    : Resource{ type }
{
    m_Name = name;
}

Resource::Resource(Resource&& other) noexcept 
    : m_IsLoaded     { std::exchange(other.m_IsLoaded, false) }
    , m_Name         { std::move(other.m_Name) }
    //, m_UsageCount   { std::exchange(other.m_UsageCount, 0) }
    , m_ResourceType { std::move(other.m_ResourceType) }
{}
Resource& Resource::operator=(Resource&& other) noexcept {
    m_IsLoaded     = std::exchange(other.m_IsLoaded, false);
    m_Name         = std::move(other.m_Name);
    //m_UsageCount   = std::exchange(other.m_UsageCount, 0);
    m_ResourceType = std::move(other.m_ResourceType);
    return *this;
}

void Resource::load() {
    if(!m_IsLoaded){
        m_IsLoaded = true;
        if (Engine::priv::Core::m_Engine) {
            Event e(EventType::ResourceLoaded);
            e.eventResource = Engine::priv::EventResource(this);
            Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
    }
}
void Resource::unload() {
    if(m_IsLoaded /*&& m_UsageCount == 0*/){
        m_IsLoaded = false;
        if (Engine::priv::Core::m_Engine) {
            Event e(EventType::ResourceUnloaded);
            e.eventResource = Engine::priv::EventResource(this);
            Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
        }
        //ENGINE_PRODUCTION_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}
