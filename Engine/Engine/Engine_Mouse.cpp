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
    m_SFMLMouse->setPosition(sf::Vector2i(x,y),*Engine::Resources::getWindow()->getSFMLHandle());
}