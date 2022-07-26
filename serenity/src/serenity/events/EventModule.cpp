#include <serenity/events/EventModule.h>
#include <serenity/types/ViewPointer.h>
#include <serenity/system/window/Window.h>

namespace {
    Engine::view_ptr<Engine::priv::EventModule> EVENT_MODULE = nullptr;
}


Engine::priv::EventModule::EventModule() {
    EVENT_MODULE = this;
}

void Engine::priv::EventModule::postUpdate(){
    m_EventDispatcher.postUpdate();
}
