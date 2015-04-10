#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Physics.h"
#include "Game.h"
#include "Engine_Events.h"

#include "ObjectDynamic.h"
#include "Light.h"

Engine::EngineClass::EngineClass(std::string name, unsigned int width, unsigned int height){
	m_DrawDebug = false;
	INIT_Window(name,width,height);
	INIT_Game();

	Window->setKeyRepeatEnabled(false);
}
Engine::EngineClass::~EngineClass(){
	glDeleteVertexArrays( 1, &m_vao );
	delete game;
	delete renderer;
	delete physicsEngine;
	delete Resources;
	delete Mouse;
	delete Window;
}
void Engine::EngineClass::INIT_Window(std::string name, unsigned int width, unsigned int height){
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

	int style = sf::Style::Fullscreen;

    Window = new sf::Window(videoMode, name, style, settings);

    Window->setVerticalSyncEnabled(true);
	Window->setMouseCursorVisible(false);

	glClearColor(0,0,0,1);
}
void Engine::EngineClass::INIT_Game(){
	Mouse->setPosition(sf::Vector2i(Window->getSize().x/2,Window->getSize().y/2),*Window);
	Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Window->getSize().x/2,Window->getSize().y/2);
	Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0,0);

	glewExperimental = GL_TRUE; 
	glewInit();

	Resources = new ResourceManager();
	Resources->INIT_Game_Resources();

	renderer = new Renderer();
	physicsEngine = new PhysicsEngine();

	game = new Game();
	game->Init_Resources();
	game->Init_Logic();

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao ); //Binds vao, all vertex attributes will be bound to this VAO
}
void Engine::EngineClass::_EVENT_HANDLERS(sf::Event e){
	switch (e.type){
        case sf::Event::Closed:
			EVENT_CLOSE();break;
        case sf::Event::KeyReleased:
			EVENT_KEY_RELEASED(e.key);break;
        case sf::Event::KeyPressed:
			EVENT_KEY_PRESSED(e.key);break;
		case sf::Event::MouseButtonPressed:
			EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton);break;
		case sf::Event::MouseButtonReleased:
			EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton);break;
		case sf::Event::MouseEntered:
			EVENT_MOUSE_ENTERED();break;
		case sf::Event::MouseLeft:
			EVENT_MOUSE_LEFT();break;
		case sf::Event::MouseWheelMoved:
			EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel);break;
		case sf::Event::MouseMoved:
			EVENT_MOUSE_MOVED(e.mouseMove);break;
		case sf::Event::Resized:
			EVENT_RESIZE(e.size.width,e.size.height);break;
		case sf::Event::TextEntered:
			EVENT_TEXT_ENTERED(e.text);break;
    }
}
void Engine::EngineClass::_RESET_EVENTS(){
	Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

	for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
	for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }

	Events::Mouse::MouseProcessing::_SetMouseWheelDelta(0);

	glm::vec2 mousePos = Engine::Events::Mouse::GetMousePosition();
	if(mousePos.x < 50 || mousePos.y < 50 || mousePos.x > static_cast<int>(Window->getSize().x - 50) || mousePos.y > static_cast<int>(Window->getSize().y - 50)){
		Mouse->setPosition(sf::Vector2i(Window->getSize().x/2,Window->getSize().y/2),*Window);
		Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Window->getSize().x/2,Window->getSize().y/2);
	}
}
void Engine::EngineClass::_Update(float dt,sf::Event e){
	game->Update(dt);
	for(auto object:Resources->Objects)
		object->Update(dt);
	for(auto light:Resources->Lights)
		light->Update(dt);
	Events::Mouse::MouseProcessing::m_Difference *= (0.975f * (1-dt));

	physicsEngine->Update(dt);
}
void Engine::EngineClass::_Render(){
	renderer->Render(m_DrawDebug);

	Window->display();
}
#pragma region Event Handler Methods
void Engine::EngineClass::EVENT_RESIZE(unsigned int width, unsigned int height)
{
}
void Engine::EngineClass::EVENT_CLOSE(){

}
void Engine::EngineClass::EVENT_LOST_FOCUS()
{
}
void Engine::EngineClass::EVENT_GAINED_FOCUS()
{
}
void Engine::EngineClass::EVENT_TEXT_ENTERED(sf::Event::TextEvent text)
{
}
void Engine::EngineClass::EVENT_KEY_PRESSED(sf::Event::KeyEvent key){
	Events::Keyboard::KeyProcessing::m_previousKey = Events::Keyboard::KeyProcessing::m_currentKey;
	Events::Keyboard::KeyProcessing::m_currentKey = key.code;
	Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = true;
}
void Engine::EngineClass::EVENT_KEY_RELEASED(sf::Event::KeyEvent key){
	Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;
	Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = false;
}
void Engine::EngineClass::EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent mouseWheel){
	Events::Mouse::MouseProcessing::_SetMouseWheelDelta(mouseWheel.delta);
}
void Engine::EngineClass::EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent mouseButton){
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
void Engine::EngineClass::EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent mouseButton){
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
void Engine::EngineClass::EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent mouse){
	Events::Mouse::MouseProcessing::m_Position_Previous = Events::Mouse::MouseProcessing::m_Position;
	Events::Mouse::MouseProcessing::m_Position.x = static_cast<float>(mouse.x);
	Events::Mouse::MouseProcessing::m_Position.y = static_cast<float>(mouse.y);

	Events::Mouse::MouseProcessing::m_Difference.x += (Events::Mouse::MouseProcessing::m_Position.x - Events::Mouse::MouseProcessing::m_Position_Previous.x);
	Events::Mouse::MouseProcessing::m_Difference.y += (Events::Mouse::MouseProcessing::m_Position.y - Events::Mouse::MouseProcessing::m_Position_Previous.y);
}
void Engine::EngineClass::EVENT_MOUSE_ENTERED()
{
}
void Engine::EngineClass::EVENT_MOUSE_LEFT(){
	Mouse->setPosition(sf::Vector2i(Window->getSize().x/2,Window->getSize().y/2),*Window);
}
void Engine::EngineClass::EVENT_JOYSTICK_BUTTON_PRESSED()
{
}
void Engine::EngineClass::EVENT_JOYSTICK_BUTTON_RELEASED()
{
}
void Engine::EngineClass::EVENT_JOYSTICK_MOVED()
{
}
void Engine::EngineClass::EVENT_JOYSTICK_CONNECTED()
{
}
void Engine::EngineClass::EVENT_JOYSTICK_DISCONNECTED()
{
}
void Engine::EngineClass::Run(){
	sf::Clock clock;
	sf::Event event;
	while(true){
		bool poll = Window->pollEvent(event);
		Resources->dt = clock.restart().asSeconds();

		_EVENT_HANDLERS(event);
		_Update(Resources->dt,event);
		_RESET_EVENTS();

		_Render();
	}
}
#pragma endregion
