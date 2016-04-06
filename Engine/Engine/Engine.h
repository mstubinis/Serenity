#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

class Texture;

namespace Engine{
    namespace Detail{
         class EngineClass final{
            public:
                static void initWindow(std::string name, unsigned int width, unsigned int height);
                static void initGame();

                #pragma region Event Handlers
                static void EVENT_RESIZE(unsigned int width, unsigned int height);
                static void EVENT_CLOSE();
                static void EVENT_LOST_FOCUS();
                static void EVENT_GAINED_FOCUS();
                static void EVENT_TEXT_ENTERED(sf::Event::TextEvent);
                static void EVENT_KEY_PRESSED(sf::Event::KeyEvent);
                static void EVENT_KEY_RELEASED(sf::Event::KeyEvent);
                static void EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent);
                static void EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent);
                static void EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent);
                static void EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent);
                static void EVENT_MOUSE_ENTERED();
                static void EVENT_MOUSE_LEFT();
                /*
                static void EVENT_JOYSTICK_BUTTON_PRESSED();
                static void EVENT_JOYSTICK_BUTTON_RELEASED();
                static void EVENT_JOYSTICK_MOVED();
                static void EVENT_JOYSTICK_CONNECTED();
                static void EVENT_JOYSTICK_DISCONNECTED();
                */
                #pragma endregion

                static void RESET_EVENTS();

                static GLuint m_vao;
                static sf::Clock clock;

                static void update();
                static void render();

                static void init(std::string app_name, unsigned int width=0, unsigned int height=0);
                static void destruct();

                static void run();
        };
    };
    float getFPS();
    sf::Window* getWindow();
    sf::Vector2u getWindowSize();
    sf::Mouse* getMouse();
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
};
#endif