#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Engine_Physics.h"
#include "Engine_Events.h"
#include "Game.h"
#include "Scene.h"

#include "ObjectDynamic.h"
#include "Light.h"

#include <SFML/Window.hpp>
#include <SFML/System.hpp>

Engine::EngineClass::EngineClass(std::string name, unsigned int width, unsigned int height){

	#ifdef ENGINE_DEBUG
	Engine::Renderer::Detail::RenderManagement::m_DrawDebug = true;
	#else
	Engine::Renderer::Detail::RenderManagement::m_DrawDebug = false;
	#endif

	_initWindow(name,width,height);
	_initGame();
}
Engine::EngineClass::~EngineClass(){
	//glDeleteVertexArrays( 1, &m_vao );
	delete game;
	Engine::Physics::Detail::PhysicsManagement::destruct();
	Engine::Renderer::Detail::RenderManagement::destruct();
	Engine::Resources::Detail::ResourceManagement::destruct();
}
void Engine::EngineClass::_initWindow(std::string name, unsigned int width, unsigned int height){
	srand(static_cast<unsigned int>(time(NULL)));

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	sf::VideoMode videoMode;
	videoMode.width = width;
	videoMode.height = height;
	videoMode.bitsPerPixel = 32;

	int style = sf::Style::Default;
	if(style == sf::Style::Fullscreen){
		videoMode = sf::VideoMode::getDesktopMode();
		width = videoMode.width;
		height = videoMode.height;
		videoMode.bitsPerPixel = 32;
	}

	Resources::Detail::ResourceManagement::m_Window = new sf::Window(videoMode, name, style, settings);

    //Resources::getWindow()->setVerticalSyncEnabled(true);
	Resources::getWindow()->setMouseCursorVisible(false);
	Resources::getWindow()->setKeyRepeatEnabled(false);
	//Resources::getWindow()->setFramerateLimit(60);

	glClearColor(1,1,0,1);
}
void Engine::EngineClass::_initGame(){
	Resources::getMouse()->setPosition(sf::Vector2i(Resources::getWindow()->getSize().x/2,Resources::getWindow()->getSize().y/2),*Resources::getWindow());
	Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindow()->getSize().x/2,Resources::getWindow()->getSize().y/2);
	Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0,0);

	glewExperimental = GL_TRUE; 
	glewInit();

	Resources::initResources();
	Renderer::Detail::RenderManagement::init();
	Physics::Detail::PhysicsManagement::init();

	game = new Game();
	game->initResources();
	game->initLogic();

	//glGenVertexArrays( 1, &m_vao );
	//glBindVertexArray( m_vao ); //Binds vao, all vertex attributes will be bound to this VAO
}
void Engine::EngineClass::_EVENT_HANDLERS(sf::Event e){
	switch (e.type){
        case sf::Event::Closed:
			_EVENT_CLOSE();break;
        case sf::Event::KeyReleased:
			_EVENT_KEY_RELEASED(e.key);break;
        case sf::Event::KeyPressed:
			_EVENT_KEY_PRESSED(e.key);break;
		case sf::Event::MouseButtonPressed:
			_EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton);break;
		case sf::Event::MouseButtonReleased:
			_EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton);break;
		case sf::Event::MouseEntered:
			_EVENT_MOUSE_ENTERED();break;
		case sf::Event::MouseLeft:
			_EVENT_MOUSE_LEFT();break;
		case sf::Event::MouseWheelMoved:
			_EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel);break;
		case sf::Event::MouseMoved:
			_EVENT_MOUSE_MOVED(e.mouseMove);break;
		case sf::Event::Resized:
			_EVENT_RESIZE(e.size.width,e.size.height);break;
		case sf::Event::TextEntered:
			_EVENT_TEXT_ENTERED(e.text);break;
    }
}
void Engine::EngineClass::_RESET_EVENTS(){
	Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

	for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
	for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }

	Events::Mouse::MouseProcessing::m_Delta *= 0.97f * (1-Resources::dt());

	if(Resources::getWindow()->hasFocus()){
		glm::vec2 mousePos = Engine::Events::Mouse::getMousePosition();
		float mouseDistFromCenter = glm::abs(glm::distance(mousePos,glm::vec2(Resources::getWindow()->getSize().x/2,Resources::getWindow()->getSize().y/2)));
		if(mouseDistFromCenter > 50){
			Resources::getMouse()->setPosition(sf::Vector2i(Resources::getWindow()->getSize().x/2,Resources::getWindow()->getSize().y/2),*Resources::getWindow());
			Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindow()->getSize().x/2,Resources::getWindow()->getSize().y/2);
		}
	}
}
void Engine::EngineClass::_update(float dt){
	game->update(dt);
	Resources::getCurrentScene()->update(dt);
	Events::Mouse::MouseProcessing::m_Difference *= (0.975f * (1-dt));
}
void Engine::EngineClass::_render(){
	game->render();
	Engine::Renderer::Detail::RenderManagement::render(Engine::Renderer::isDebug());
}
#pragma region Event Handler Methods
void Engine::EngineClass::_EVENT_RESIZE(unsigned int width, unsigned int height){
	glViewport(0,0,width,height);

	Renderer::Detail::RenderManagement::m_gBuffer->resizeBaseBuffer(width,height);
	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
		Renderer::Detail::RenderManagement::m_gBuffer->resizeBuffer(i,width,height);
	}
	for(auto camera:Resources::Detail::ResourceManagement::m_Cameras){
		camera.second->resize(width,height);
	}
}
void Engine::EngineClass::_EVENT_CLOSE(){

}
void Engine::EngineClass::_EVENT_LOST_FOCUS()
{
	Resources::getWindow()->setMouseCursorVisible(true);
}
void Engine::EngineClass::_EVENT_GAINED_FOCUS()
{
	Resources::getWindow()->setMouseCursorVisible(false);
}
void Engine::EngineClass::_EVENT_TEXT_ENTERED(sf::Event::TextEvent text)
{
}
void Engine::EngineClass::_EVENT_KEY_PRESSED(sf::Event::KeyEvent key){
	Events::Keyboard::KeyProcessing::m_previousKey = Events::Keyboard::KeyProcessing::m_currentKey;
	Events::Keyboard::KeyProcessing::m_currentKey = key.code;
	Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = true;
}
void Engine::EngineClass::_EVENT_KEY_RELEASED(sf::Event::KeyEvent key){
	Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = false;
}
void Engine::EngineClass::_EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent mouseWheel){
	Events::Mouse::MouseProcessing::m_Delta += (mouseWheel.delta * 10);
}
void Engine::EngineClass::_EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent mouseButton){
	Resources::getWindow()->setMouseCursorVisible(false);
	Engine::Events::Mouse::MouseProcessing::m_previousButton = Engine::Events::Mouse::MouseProcessing::m_currentButton;
	if(mouseButton.button == sf::Mouse::Button::Left){
		Engine::Events::Mouse::MouseProcessing::m_currentButton = Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_LEFT;
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_LEFT] = true;
		return;
	}
	if(mouseButton.button == sf::Mouse::Button::Middle){
		Engine::Events::Mouse::MouseProcessing::m_currentButton = Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_MIDDLE;
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_MIDDLE] = true;
		return;
	}
	if(mouseButton.button == sf::Mouse::Button::Right){
		Engine::Events::Mouse::MouseProcessing::m_currentButton = Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_RIGHT;
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_RIGHT] = true;
		return;
	}
}
void Engine::EngineClass::_EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent mouseButton){
	Engine::Events::Mouse::MouseProcessing::m_previousButton = Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_NONE;
	Engine::Events::Mouse::MouseProcessing::m_currentButton = Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_NONE;

	if(mouseButton.button == sf::Mouse::Button::Left){
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_LEFT] = false;
		return;
	}
	if(mouseButton.button == sf::Mouse::Button::Middle){
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_MIDDLE] = false;
		return;
	}
	if(mouseButton.button == sf::Mouse::Button::Right){
		Engine::Events::Mouse::MouseProcessing::m_MouseStatus[Engine::Events::Mouse::MouseButton::MOUSE_BUTTON_RIGHT] = false;
		return;
	}
}
void Engine::EngineClass::_EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent mouse){
	if(Resources::getWindow()->hasFocus()){
		Events::Mouse::MouseProcessing::m_Position_Previous = Events::Mouse::MouseProcessing::m_Position;
		Events::Mouse::MouseProcessing::m_Position.x = static_cast<float>(mouse.x);
		Events::Mouse::MouseProcessing::m_Position.y = static_cast<float>(mouse.y);

		Events::Mouse::MouseProcessing::m_Difference.x += (Events::Mouse::MouseProcessing::m_Position.x - Events::Mouse::MouseProcessing::m_Position_Previous.x);
		Events::Mouse::MouseProcessing::m_Difference.y += (Events::Mouse::MouseProcessing::m_Position.y - Events::Mouse::MouseProcessing::m_Position_Previous.y);
	}
}
void Engine::EngineClass::_EVENT_MOUSE_ENTERED()
{
}
void Engine::EngineClass::_EVENT_MOUSE_LEFT(){
	Resources::getWindow()->setMouseCursorVisible(true);
}
/*
void Engine::EngineClass::_EVENT_JOYSTICK_BUTTON_PRESSED()
{
}
void Engine::EngineClass::_EVENT_JOYSTICK_BUTTON_RELEASED()
{
}
void Engine::EngineClass::_EVENT_JOYSTICK_MOVED()
{
}
void Engine::EngineClass::_EVENT_JOYSTICK_CONNECTED()
{
}
void Engine::EngineClass::_EVENT_JOYSTICK_DISCONNECTED()
{
}
*/
void Engine::EngineClass::run(){
	while(Resources::getWindow()->isOpen()){
		sf::Event e;
		Resources::Detail::ResourceManagement::m_DeltaTime = clock.restart().asSeconds();
		while(Resources::getWindow()->pollEvent(e)){
			_EVENT_HANDLERS(e);
		}
		_update(Resources::dt());
		_RESET_EVENTS();

		_render();
		Resources::getWindow()->display();
		Physics::Detail::PhysicsManagement::update(Resources::dt(),10);
	}
}
#pragma endregion
