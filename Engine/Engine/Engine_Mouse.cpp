#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Mouse.h"
#include <SFML/Window.hpp>

class Engine_Mouse::impl{
	private:
		sf::Mouse* m_handle;
	public:
		sf::Mouse* handle(){ return m_handle; }
		void _destruct(){ delete m_handle; }
		void _setPosition(uint x, uint y){
			m_handle->setPosition(sf::Vector2i(x,y),*Engine::Resources::getWindow()->getSFMLHandle());
		}
};

Engine_Mouse::Engine_Mouse():m_i(new impl()){
}
Engine_Mouse::~Engine_Mouse(){
	m_i->_destruct();
}
void Engine_Mouse::setPosition(uint x, uint y){
	m_i->_setPosition(x,y);
}