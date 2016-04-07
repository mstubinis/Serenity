#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <string>
#include <SFML/Window.hpp>
#include "Engine.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

typedef unsigned int uint;

class Engine_Window final{
    #ifdef _WIN32
        private: void _createDirectXWindow(const char* name,uint width,uint height);
        public: void setRenderingAPI(uint);
    #endif

    private:
        const char* m_WindowName;
        sf::Window* m_SFMLWindow;
        uint m_Width;
        uint m_Height;
        void _createOpenGLWindow(const char* name,uint width,uint height);
		void _destroyOpenGLContext();
    public:
        Engine_Window(const char* name,uint width,uint height,ENGINE_RENDERING_API);
        ~Engine_Window();

        const char* name();
        sf::Vector2u getSize();
        void setName(const char* name);
        void setIcon(uint width, uint height, const sf::Uint8* pixels);
        void setMouseCursorVisible(bool);
        void setKeyRepeatEnabled(bool);
        void setVerticalSyncEnabled(bool);
        void close();
        void requestFocus();

        bool hasFocus();
        bool isOpen();
        bool pollEventSFML(sf::Event&);
        void setFullScreen(bool);

        void display();

		sf::Window* getSFMLHandle(){ return m_SFMLWindow; }
};

#endif