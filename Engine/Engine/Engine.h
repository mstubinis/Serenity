#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

class Texture;
class Engine_Window;

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

typedef unsigned int GLuint;
typedef unsigned int uint;

namespace Engine{
    namespace Detail{
         class EngineClass final{
            public:
                static void initGame();

                #pragma region Event Handlers
                static void EVENT_RESIZE(uint width, uint height,bool saveSize = true);
                static void EVENT_CLOSE();
                static void EVENT_LOST_FOCUS();
                static void EVENT_GAINED_FOCUS();
                static void EVENT_TEXT_ENTERED(sf::Event::TextEvent);
                static void EVENT_KEY_PRESSED(uint);
                static void EVENT_KEY_RELEASED(uint);
                static void EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent);
                static void EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent);
                static void EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent);
                static void EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent);
                static void EVENT_MOUSE_ENTERED();
                static void EVENT_MOUSE_LEFT();
                static void EVENT_JOYSTICK_BUTTON_PRESSED();
                static void EVENT_JOYSTICK_BUTTON_RELEASED();
                static void EVENT_JOYSTICK_MOVED();
                static void EVENT_JOYSTICK_CONNECTED();
                static void EVENT_JOYSTICK_DISCONNECTED();
                #pragma endregion

                static void RESET_EVENTS();

                static sf::Clock clock;

                static void update();
                static void render();

                static void init(const char* name,uint width=0,uint height=0);
                static void destruct();

                static void run();
        };
    };
    const float getFPS();
    Engine_Window* getWindow();
    const sf::Vector2u& getWindowSize();
    void setWindowIcon(Texture* texture);
    void showMouseCursor();
    void hideMouseCursor();
    void stop();
    void setFullScreen(bool b);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(float);
    void render();
    void cleanup();

    #pragma region EventHandlers
    void onResize(uint width,uint height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(sf::Event::TextEvent);
    void onKeyPressed(uint);
    void onKeyReleased(uint);
    void onMouseWheelMoved(sf::Event::MouseWheelEvent);
    void onMouseButtonPressed(sf::Event::MouseButtonEvent);
    void onMouseButtonReleased(sf::Event::MouseButtonEvent);
    void onMouseMoved(sf::Event::MouseMoveEvent);
    void onMouseEntered();
    void onMouseLeft();
    void onPreUpdate(float dt);
    void onPostUpdate(float dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
    #pragma endregion
};
#endif