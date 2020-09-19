#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/resources/Resource.h>
#include <core/engine/utils/Logger.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/Event.h>

void Resource::load() {
    if(!m_IsLoaded){
        m_IsLoaded = true;
        if (Engine::priv::Core::m_Engine) {
            Event e(EventType::ResourceLoaded);
            e.eventResource = Engine::priv::EventResource(this);
            Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
        }
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
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
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}
