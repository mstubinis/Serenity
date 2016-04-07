#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Mouse.h"
#include <SFML/Window.hpp>

Engine_Mouse::Engine_Mouse(){
}
Engine_Mouse::~Engine_Mouse(){
    delete(m_SFMLMouse);
}
void Engine_Mouse::setPosition(uint x, uint y){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        m_SFMLMouse->setPosition(sf::Vector2i(x,y),*Engine::Resources::getWindow()->getOpenGLHandle());
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        #ifdef _WIN32
            POINT point = {x, y};
            ClientToScreen(Engine::getWindow()->getDirectXHandle(), &point);
            SetCursorPos(point.x, point.y);
        #endif
    }
}