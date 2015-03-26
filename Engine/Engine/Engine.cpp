#include "Engine.h"
#include "Game.h"
#include "Engine_Events.h"

Engine::EngineClass::EngineClass(std::string name, unsigned int width, unsigned int height){
	m_State = ENGINE_STATE_PLAY;
	m_DrawDebug = true;
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

	renderer = new Renderer(RENDER_TYPE_DEFERRED);
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

	//Serenity::Events::Mouse::MouseProcessing::m_previousButton = Serenity::Events::Mouse::MouseButton::MOUSE_BUTTON_NONE;
	//Serenity::Events::Mouse::MouseProcessing::m_currentButton = Serenity::Events::Mouse::MouseButton::MOUSE_BUTTON_NONE;

	for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
	for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }
}
void Engine::EngineClass::EDIT_Update(float dt, sf::Event e){
	m_Timer += dt;
	#pragma region DebugCamera
	//update debug camera
	if(Events::Keyboard::IsKeyDown("q") == true)
		Resources->Current_Camera()->Rotate(0,0,0.5f);
	if(Events::Keyboard::IsKeyDown("e") == true)
		Resources->Current_Camera()->Rotate(0,0,-0.5f);
	if(m_MouseMoved == false && m_Timer > 0.165f){
		m_MouseMoved = true;
		m_Timer = 0;
	}
	if(Events::Mouse::IsMouseButtonDown("left") == true){
		Resources->Current_Camera()->Translate(0,0,Mouse_Difference.y*0.2f);
		if(Events::Mouse::IsMouseButtonDown("right") == true)
			Resources->Current_Camera()->Translate(Mouse_Difference.x*0.2f,0,0);
	}
	else{
		if(Events::Mouse::IsMouseButtonDown("right") == true)
			Resources->Current_Camera()->Rotate(Mouse_Difference.y*0.005f,Mouse_Difference.x*0.005f,0);
	}
	#pragma endregion

	for(auto object:Resources->Objects)
		object->Update(dt);
	//other stuff
	Mouse_Difference *= (0.975f * (1-dt));
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
	bullet->Update(dt);
	renderer->Update(dt);

	Mouse_Difference *= (0.975f * (1-dt));
}
void Engine::EngineClass::Render(){
	renderer->Render(m_DrawDebug);

	if(m_DrawDebug)
		bullet->Render();

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

		switch(m_State){
			case ENGINE_STATE_EDIT:
				EDIT_Update(Resources->dt,event);
				if(Events::Keyboard::IsKeyDownOnce("p") == true){
					this->_SWITCH_STATES(ENGINE_STATE_PLAY);
				}
				break;
			case ENGINE_STATE_PLAY:
				Update(Resources->dt,event);
				if(Events::Keyboard::IsKeyDownOnce("p") == true){
					this->_SWITCH_STATES(ENGINE_STATE_EDIT);
				}
				break;
			case ENGINE_STATE_PAUSE:
				break;
			default:
				break;
		}
		this->_RESET_EVENTS();

		Render();

	}
}
void Engine::EngineClass::_SWITCH_STATES(ENGINE_STATE state){
	m_State = state;
	switch(m_State){
		case ENGINE_STATE_EDIT:
			Resources->Set_Active_Camera("Debug");
			break;
		case ENGINE_STATE_PLAY:
			Resources->Set_Active_Camera("Default");
			break;
		case ENGINE_STATE_PAUSE:
			break;
		default:
			break;
	}
}
#pragma endregion
