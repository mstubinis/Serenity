#pragma once
#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <memory>
namespace sf{ class Window; };
typedef unsigned int uint;
class Engine_Window final{
	private:
		class impl;
		std::unique_ptr<impl> m_i;

    #ifdef _WIN32
		public: void setRenderingAPI(uint);
    #endif

    public:
        Engine_Window(const char* name,uint width,uint height,uint api);
        ~Engine_Window();

        const char* name();
        sf::Vector2u getSize();
        void setName(const char* name);
        void setSize(uint w, uint h);
        void setIcon(Texture* texture);
        void setIcon(const char*);
        void setMouseCursorVisible(bool);
        void setKeyRepeatEnabled(bool);
        void setVerticalSyncEnabled(bool);
        void close();
        void requestFocus();
        void setActive(bool);
        uint getStyle();
        bool hasFocus();
        bool isOpen();
        void setFullScreen(bool);
        void setStyle(uint style);
        void display();

        sf::Window* getSFMLHandle();
};

#endif