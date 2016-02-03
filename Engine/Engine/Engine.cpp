#include "Engine.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Scene.h"

#include "ObjectDynamic.h"
#include "Light.h"

#include <SFML/Window.hpp>
#include <SFML/System.hpp>

Engine::EngineClass::EngineClass(std::string name, unsigned int width, unsigned int height){
	srand(static_cast<unsigned int>(time(NULL)));
	_initWindow(name,width,height);
	_initGame();
}
Engine::EngineClass::~EngineClass(){
	//glDeleteVertexArrays( 1, &m_vao );
	Game::cleanup();
	Engine::Physics::Detail::PhysicsManagement::destruct();
	Engine::Renderer::Detail::RenderManagement::destruct();
	Engine::Resources::Detail::ResourceManagement::destruct();
}
void Engine::EngineClass::_initWindow(std::string name, unsigned int width, unsigned int height){
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
	if(width == 0 || height == 0){
		style = sf::Style::Fullscreen;
		videoMode = sf::VideoMode::getDesktopMode();
		width = videoMode.width;
		height = videoMode.height;
	}
	Resources::Detail::ResourceManagement::m_Window = new sf::Window(videoMode, name, style, settings);

    //Resources::getWindow()->setVerticalSyncEnabled(true);
	Resources::getWindow()->setMouseCursorVisible(false);
	Resources::getWindow()->setKeyRepeatEnabled(false);
	//Resources::getWindow()->setFramerateLimit(60);

	glClearColor(1,1,0,1);
}
void Engine::EngineClass::_initGame(){
	Resources::getMouse()->setPosition(sf::Vector2i(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2),*Resources::getWindow());
	Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
	Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0);

	glewExperimental = GL_TRUE; 
	glewInit();

	Resources::initResources();
	Renderer::Detail::RenderManagement::init();
	Physics::Detail::PhysicsManagement::init();


	//the scene is the root of all games. create the default scene
	//Scene* scene;
	//if(Resources::getCurrentScene() == nullptr)
		//scene = new Scene("Default");

	Game::initResources();
	Game::initLogic();

	//glGenVertexArrays( 1, &m_vao );
	//glBindVertexArray( m_vao ); //Binds vao, all vertex attributes will be bound to this VAO
}
void Engine::EngineClass::_RESET_EVENTS(){
	Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

	for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
	for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }

	Events::Mouse::MouseProcessing::m_Delta *= 0.97f * (1-Resources::dt());

	if(Resources::getWindow()->hasFocus()){
		glm::vec2 mousePos = Engine::Events::Mouse::getMousePosition();
		float mouseDistFromCenter = glm::abs(glm::distance(mousePos,glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2)));
		if(mouseDistFromCenter > 50){
			Resources::getMouse()->setPosition(sf::Vector2i(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2),*Resources::getWindow());
			Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
		}
	}
}
void Engine::EngineClass::_update(float dt){
	Game::update(dt);
	Resources::getCurrentScene()->update(dt);
	Events::Mouse::MouseProcessing::m_Difference *= (0.975f * (1-dt));
}
void Engine::EngineClass::_render(){
	Game::render();
	Engine::Renderer::Detail::RenderManagement::render();
}
#pragma region Event Handler Methods
void Engine::EngineClass::_EVENT_RESIZE(unsigned int width, unsigned int height){
	glViewport(0,0,width,height);

	Renderer::Detail::RenderManagement::m_gBuffer->resizeBaseBuffer(width,height);
	Engine::Renderer::Detail::RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,(float)Resources::getWindowSize().x,0.0f,(float)Resources::getWindowSize().y,0.005f,1000.0f);
	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
		Renderer::Detail::RenderManagement::m_gBuffer->resizeBuffer(i,width,height);
	}
	for(auto camera:Resources::Detail::ResourceManagement::m_Cameras){
		camera.second->resize(width,height);
	}
}
void Engine::EngineClass::_EVENT_CLOSE(){
	Resources::getWindow()->close();
}
void Engine::EngineClass::_EVENT_LOST_FOCUS(){
	Resources::getWindow()->setMouseCursorVisible(true);
}
void Engine::EngineClass::_EVENT_GAINED_FOCUS(){
	Resources::getWindow()->setMouseCursorVisible(false);
}
void Engine::EngineClass::_EVENT_TEXT_ENTERED(sf::Event::TextEvent text){
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

	Engine::Events::Mouse::MouseProcessing::m_currentButton = mouseButton.button;
	Engine::Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = true;
}
void Engine::EngineClass::_EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent mouseButton){
	Engine::Events::Mouse::MouseProcessing::m_previousButton = 100; //we will use 100 as the "none" key
	Engine::Events::Mouse::MouseProcessing::m_currentButton = 100;  //we will use 100 as the "none" key

	Engine::Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = false;
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
void Engine::EngineClass::_EVENT_MOUSE_ENTERED(){
}
void Engine::EngineClass::_EVENT_MOUSE_LEFT(){
	Resources::getWindow()->setMouseCursorVisible(true);
}
/*
void Engine::EngineClass::_EVENT_JOYSTICK_BUTTON_PRESSED(){
}
void Engine::EngineClass::_EVENT_JOYSTICK_BUTTON_RELEASED(){
}
void Engine::EngineClass::_EVENT_JOYSTICK_MOVED(){
}
void Engine::EngineClass::_EVENT_JOYSTICK_CONNECTED(){
}
void Engine::EngineClass::_EVENT_JOYSTICK_DISCONNECTED(){
}
*/

void Engine::EngineClass::run(){
	while(Resources::getWindow()->isOpen()){

		sf::Event event;
		Resources::Detail::ResourceManagement::m_DeltaTime = clock.restart().asSeconds();
		while(Resources::getWindow()->pollEvent(event)){
			#pragma region Event Handlers
			switch (event.type){
				case sf::Event::Closed:
					_EVENT_CLOSE();break;
				case sf::Event::KeyReleased:
					_EVENT_KEY_RELEASED(event.key);break;
				case sf::Event::KeyPressed:
					_EVENT_KEY_PRESSED(event.key);break;
				case sf::Event::MouseButtonPressed:
					_EVENT_MOUSE_BUTTON_PRESSED(event.mouseButton);break;
				case sf::Event::MouseButtonReleased:
					_EVENT_MOUSE_BUTTON_RELEASED(event.mouseButton);break;
				case sf::Event::MouseEntered:
					_EVENT_MOUSE_ENTERED();break;
				case sf::Event::MouseLeft:
					_EVENT_MOUSE_LEFT();break;
				case sf::Event::MouseWheelMoved:
					_EVENT_MOUSE_WHEEL_MOVED(event.mouseWheel);break;
				case sf::Event::MouseMoved:
					_EVENT_MOUSE_MOVED(event.mouseMove);break;
				case sf::Event::Resized:
					_EVENT_RESIZE(event.size.width,event.size.height);break;
				case sf::Event::TextEntered:
					_EVENT_TEXT_ENTERED(event.text);break;
				default:
					break;
			}
			#pragma endregion
		}
		_update(Resources::dt());
		_RESET_EVENTS();

		_render();
		Resources::getWindow()->display();
		Physics::Detail::PhysicsManagement::update(Resources::dt(),10);
	}
}
#pragma endregion
