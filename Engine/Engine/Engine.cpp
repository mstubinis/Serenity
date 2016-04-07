#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Events.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Scene.h"
#include "Texture.h"
#include "ObjectDynamic.h"
#include "Light.h"
#include "Engine_Window.h"
#include "Engine_Mouse.h"
#include <SFML/System.hpp>

#ifdef ENGINE_DEBUG
#include <iostream>
#endif

sf::Clock Engine::Detail::EngineClass::clock = sf::Clock();
GLuint Engine::Detail::EngineClass::m_vao = 0;

void Engine::Detail::EngineClass::init(uint api, const char* name,uint width,uint height){
	srand((unsigned)time(0));
    Resources::Detail::ResourceManagement::m_Window = new Engine_Window(name,width,height,api);
    Resources::Detail::ResourceManagement::m_Mouse = new Engine_Mouse();
    initGame(api);
}
void Engine::Detail::EngineClass::destruct(){
    //glDeleteVertexArrays( 1, &m_vao );
    Game::cleanup();
    Engine::Resources::Detail::ResourceManagement::destruct();
    Engine::Physics::Detail::PhysicsManagement::destruct();
    Engine::Renderer::Detail::RenderManagement::destruct();
    Engine::Sound::Detail::SoundManagement::destruct();
}
void Engine::Detail::EngineClass::initGame(uint api){
	if(api == ENGINE_RENDERING_API_DIRECTX)
		return;

    Resources::getMouse()->setPosition(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0);

    Renderer::Detail::RenderManagement::init();
    Physics::Detail::PhysicsManagement::init();
    Sound::Detail::SoundManagement::init();

    Resources::initResources();

    //the scene is the root of all games. create the default scene
    //Scene* scene;
    //if(Resources::getCurrentScene() == nullptr)
        //scene = new Scene("Default");

    Game::initResources();
    Game::initLogic();

	Resources::initRenderingContexts(api);

    //glGenVertexArrays( 1, &m_vao );
    //glBindVertexArray( m_vao ); //Binds vao, all vertex attributes will be bound to this VAO
}
void Engine::Detail::EngineClass::RESET_EVENTS(){
    Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
    Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

    for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
    for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }

    Events::Mouse::MouseProcessing::m_Delta *= 0.97f * (1-Resources::dt());

    if(Resources::getWindow()->hasFocus()){
        glm::vec2 mousePos = Engine::Events::Mouse::getMousePosition();
        float mouseDistFromCenter = glm::abs(glm::distance(mousePos,glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2)));
        if(mouseDistFromCenter > 50){
            Resources::getMouse()->setPosition(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
            Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
        }
    }
}
void Engine::Detail::EngineClass::update(uint api){
	if(api == ENGINE_RENDERING_API_DIRECTX)
		return;
    Game::update(Resources::dt());
    Resources::getCurrentScene()->update(Resources::dt());
    Engine::Physics::Detail::PhysicsManagement::update(Resources::dt());
    Events::Mouse::MouseProcessing::m_Difference *= (0.975f * (1-Resources::dt()));
    RESET_EVENTS();
}
void Engine::Detail::EngineClass::render(uint api){
	if(api == ENGINE_RENDERING_API_DIRECTX)
		return;
    Game::render();
    Engine::Renderer::Detail::RenderManagement::render();
    Resources::getWindow()->display();
}
#pragma region Event Handler Methods
void Engine::Detail::EngineClass::EVENT_RESIZE(unsigned int width, unsigned int height,bool saveSize){
    glViewport(0,0,width,height);

    Renderer::Detail::RenderManagement::m_gBuffer->resizeBaseBuffer(width,height);
    Engine::Renderer::Detail::RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,(float)width,0.0f,(float)height,0.005f,1000.0f);
    for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
        Renderer::Detail::RenderManagement::m_gBuffer->resizeBuffer(i,width,height);
    }
    for(auto camera:Resources::Detail::ResourceManagement::m_Cameras){
        camera.second.get()->resize(width,height);
    }
	if(saveSize)
		Engine::Resources::getWindow()->setSize(width,height);
}
void Engine::Detail::EngineClass::EVENT_CLOSE(){
    Resources::getWindow()->close();
}
void Engine::Detail::EngineClass::EVENT_LOST_FOCUS(){
    Resources::getWindow()->setMouseCursorVisible(true);
}
void Engine::Detail::EngineClass::EVENT_GAINED_FOCUS(){
    Resources::getWindow()->setMouseCursorVisible(false);
}
void Engine::Detail::EngineClass::EVENT_TEXT_ENTERED(sf::Event::TextEvent text){
}
void Engine::Detail::EngineClass::EVENT_KEY_PRESSED(uint key){
    Events::Keyboard::KeyProcessing::m_previousKey = Events::Keyboard::KeyProcessing::m_currentKey;

    Events::Keyboard::KeyProcessing::m_currentKey = key;
    Events::Keyboard::KeyProcessing::m_KeyStatus[key] = true;
}
void Engine::Detail::EngineClass::EVENT_KEY_RELEASED(uint key){
    Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
    Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

    Events::Keyboard::KeyProcessing::m_KeyStatus[key] = false;
}
void Engine::Detail::EngineClass::EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent mouseWheel){
    Events::Mouse::MouseProcessing::m_Delta += (mouseWheel.delta * 10);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent mouseButton){
    Resources::getWindow()->setMouseCursorVisible(false);
    Events::Mouse::MouseProcessing::m_previousButton = Engine::Events::Mouse::MouseProcessing::m_currentButton;

    Events::Mouse::MouseProcessing::m_currentButton = mouseButton.button;
    Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = true;
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent mouseButton){
    Events::Mouse::MouseProcessing::m_previousButton = 100; //we will use 100 as the "none" key
    Events::Mouse::MouseProcessing::m_currentButton = 100;  //we will use 100 as the "none" key

    Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = false;
}
void Engine::Detail::EngineClass::EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent mouse){
    if(Resources::getWindow()->hasFocus()){
        Events::Mouse::MouseProcessing::m_Position_Previous = Events::Mouse::MouseProcessing::m_Position;
        Events::Mouse::MouseProcessing::m_Position.x = static_cast<float>(mouse.x);
        Events::Mouse::MouseProcessing::m_Position.y = static_cast<float>(mouse.y);

        Events::Mouse::MouseProcessing::m_Difference.x += (Events::Mouse::MouseProcessing::m_Position.x - Events::Mouse::MouseProcessing::m_Position_Previous.x);
        Events::Mouse::MouseProcessing::m_Difference.y += (Events::Mouse::MouseProcessing::m_Position.y - Events::Mouse::MouseProcessing::m_Position_Previous.y);
    }
}
void Engine::Detail::EngineClass::EVENT_MOUSE_ENTERED(){
}
void Engine::Detail::EngineClass::EVENT_MOUSE_LEFT(){
    Resources::getWindow()->setMouseCursorVisible(true);
}
/*
void Engine::Detail::EngineClass::EVENT_JOYSTICK_BUTTON_PRESSED(){
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_BUTTON_RELEASED(){
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_MOVED(){
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_CONNECTED(){
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_DISCONNECTED(){
}
*/
float Engine::getFPS(){ return 1.0f / Resources::dt(); }
Engine_Window* Engine::getWindow(){ return Resources::Detail::ResourceManagement::m_Window; }
sf::Vector2u Engine::getWindowSize(){ return Resources::Detail::ResourceManagement::m_Window->getSize(); }
Engine_Mouse* Engine::getMouse(){ return Resources::Detail::ResourceManagement::m_Mouse; }
void Engine::setWindowIcon(Texture* texture){
    texture->generatePixelPointer();
    Resources::getWindow()->setIcon(texture->width(),texture->height(),texture->getPixelsPtr()); 
}
void Engine::showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
void Engine::stop(){ Resources::getWindow()->close(); }
void Engine::setFullScreen(bool b){
    Engine::Resources::Detail::ResourceManagement::m_Window->setFullScreen(b);
}
void Engine::Detail::EngineClass::run(uint api){
    while(Resources::getWindow()->isOpen()){
        sf::Event event;
        Resources::Detail::ResourceManagement::m_DeltaTime = clock.restart().asSeconds();
        while(Resources::getWindow()->pollEventSFML(event)){
            switch (event.type){
                case sf::Event::Closed:               EVENT_CLOSE();break;
                case sf::Event::KeyReleased:          EVENT_KEY_RELEASED(event.key.code);break;
                case sf::Event::KeyPressed:           EVENT_KEY_PRESSED(event.key.code);break;
                case sf::Event::MouseButtonPressed:   EVENT_MOUSE_BUTTON_PRESSED(event.mouseButton);break;
                case sf::Event::MouseButtonReleased:  EVENT_MOUSE_BUTTON_RELEASED(event.mouseButton);break;
                case sf::Event::MouseEntered:         EVENT_MOUSE_ENTERED();break;
                case sf::Event::MouseLeft:            EVENT_MOUSE_LEFT();break;
                case sf::Event::MouseWheelMoved:      EVENT_MOUSE_WHEEL_MOVED(event.mouseWheel);break;
                case sf::Event::MouseMoved:           EVENT_MOUSE_MOVED(event.mouseMove);break;
                case sf::Event::Resized:              EVENT_RESIZE(event.size.width,event.size.height);break;
                case sf::Event::TextEntered:          EVENT_TEXT_ENTERED(event.text);break;
                default:                              break;
            }
        }
        update(api);
		render(api);
    }
}