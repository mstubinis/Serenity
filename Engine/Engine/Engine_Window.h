#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <string>
#include <SFML/Window.hpp>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

typedef unsigned int uint;

class Engine_Window final{
    #ifdef _WIN32
        private: void _createDirectXWindow(const char* name,uint width,uint height,BOOL windowed = TRUE);
        public: void setRenderingAPI(uint);
		private: void _destroyDirectXContext();
    #endif

    private:
		uint m_Style;
		sf::VideoMode m_VideoMode;
        const char* m_WindowName;
        sf::Window* m_SFMLWindow;
        uint m_Width;
        uint m_Height;
        void _createOpenGLWindow(const char* name,uint width,uint height);
		void _destroyOpenGLContext();
    public:
        Engine_Window(const char* name,uint width,uint height,uint api);
        ~Engine_Window();

        const char* name();
        sf::Vector2u getSize();
        void setName(const char* name);
		void setSize(uint w, uint h);
        void setIcon(uint width, uint height, const sf::Uint8* pixels);
        void setMouseCursorVisible(bool);
        void setKeyRepeatEnabled(bool);
        void setVerticalSyncEnabled(bool);
        void close();
        void requestFocus();
		void setActive(bool);

		uint getStyle(){ return m_Style; }
        bool hasFocus();
        bool isOpen();
        bool pollEvent(sf::Event&);
        void setFullScreen(bool);
		void setStyle(uint style);

        void display();

		sf::Window* getSFMLHandle(){ return m_SFMLWindow; }
};

#endif