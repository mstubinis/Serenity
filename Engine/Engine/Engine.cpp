#include "Engine.h"
#include "Game.h"
#include "Engine_Events.h"

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
	delete bullet;
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
    Window = new sf::Window(sf::VideoMode(width, height), name, sf::Style::Default, settings);
    Window->setVerticalSyncEnabled(true);
	Window->setMouseCursorVisible(false);
	
	m_MouseMoved = true;

	glClearColor(0,0,0,1); // black background
}
void Engine::EngineClass::INIT_Game(){
	Mouse->setPosition(sf::Vector2i(Window->getSize().x/2,Window->getSize().y/2),*Window);
	Mouse_Position = Mouse_Position_Previous = glm::vec2(Window->getSize().x/2,Window->getSize().y/2);
	Mouse_Difference = glm::vec2(0,0);

	glewExperimental = GL_TRUE; 
	glewInit();

	Resources = new ResourceManager();
	Resources->INIT_Game_Resources();

	renderer = new Renderer();
	bullet = new Bullet();

	game = new Game();
	game->Init_Resources();
	game->Init_Logic();

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao ); //Binds vao, all vertex attributes will be bound to this VAO
}
void Engine::EngineClass::_EVENT_HANDLERS(sf::Event e){
	#pragma region Event Handlers
	switch (e.type){
        case sf::Event::Closed:
			this->EVENT_CLOSE();
            break;
        case sf::Event::KeyReleased:
			this->EVENT_KEY_RELEASED(e.key);
            break;
        case sf::Event::KeyPressed:
			this->EVENT_KEY_PRESSED(e.key);
            break;
		case sf::Event::MouseButtonPressed:
			this->EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton);
			break;
		case sf::Event::MouseButtonReleased:
			this->EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton);
			break;
		case sf::Event::MouseEntered:
			this->EVENT_MOUSE_ENTERED();
			break;
		case sf::Event::MouseLeft:
			this->EVENT_MOUSE_LEFT();
			break;
		case sf::Event::MouseWheelMoved:
			this->EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel);
			break;
		case sf::Event::MouseMoved:
			this->EVENT_MOUSE_MOVED(e.mouseMove,m_MouseMoved);
			break;
		case sf::Event::Resized:
			this->EVENT_RESIZE(e.size.width,e.size.height);
			break;
		case sf::Event::TextEntered:
			this->EVENT_TEXT_ENTERED(e.text);
			break;
        default:
            break;
    }
	#pragma endregion
}
void Engine::EngineClass::_RESET_EVENTS(){
	Engine::Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Engine::Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

	for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
	for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }
}
void Engine::EngineClass::Update(float dt,sf::Event e){
	m_Timer += dt;

	if(m_MouseMoved == false && m_Timer > 0.165f){
		m_MouseMoved = true;
		m_Timer = 0;
	}

	game->Update(dt);
	for(auto object:Resources->Objects)
		object->Update(dt);
	for(auto light:Resources->Lights)
		light->Update(dt);
	Mouse_Difference *= (0.975f * (1-dt));

	bullet->Update(dt);
}
void Engine::EngineClass::Render(){
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
	Engine::Events::Keyboard::KeyProcessing::m_previousKey = Engine::Events::Keyboard::KeyProcessing::m_currentKey;
	Engine::Events::Keyboard::KeyProcessing::m_currentKey = key.code;
	Engine::Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = true;
}
void Engine::EngineClass::EVENT_KEY_RELEASED(sf::Event::KeyEvent key){
	Engine::Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
	Engine::Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;
	Engine::Events::Keyboard::KeyProcessing::m_KeyStatus[key.code] = false;
}
void Engine::EngineClass::EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent mouseWheel){
	Mouse_Position_Previous = Mouse_Position;
	Mouse_Position.x = static_cast<float>(mouseWheel.x);
	Mouse_Position.y = static_cast<float>(mouseWheel.y);
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
void Engine::EngineClass::EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent mouse,bool doit){
	Mouse_Position_Previous = Mouse_Position;
	Mouse_Position.x = static_cast<float>(mouse.x);
	Mouse_Position.y = static_cast<float>(mouse.y);
	if (doit == false){
		return;
	}

	if(m_MouseMoved == true){
		Mouse_Difference.x += (Mouse_Position.x - Mouse_Position_Previous.x);
		Mouse_Difference.y += (Mouse_Position.y - Mouse_Position_Previous.y);
	}
	if(mouse.x < 50 || mouse.y < 50 || mouse.x > static_cast<int>(Window->getSize().x - 50) || mouse.y > static_cast<int>(Window->getSize().y - 50)){
		m_MouseMoved = false;
		m_Timer = 0;
		Mouse->setPosition(sf::Vector2i(Window->getSize().x/2,Window->getSize().y/2),*Window);
	}
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
		this->_EVENT_HANDLERS(event);

		this->Update(Resources->dt,event);

		this->_RESET_EVENTS();

		Render();
	}
}
#pragma endregion
