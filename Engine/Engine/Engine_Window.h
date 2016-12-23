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

    public:
        Engine_Window(const char* name,uint width,uint height,uint api);
        ~Engine_Window();

		#ifdef _WIN32
		void setRenderingAPI(uint);
		#endif

        const char* name() const;
        sf::Vector2u getSize();
        void setName(const char* name);
        void setSize(uint w, uint h);
        void setIcon(Texture* texture);
        void setIcon(const char* file);
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
		void keepMouseInWindow(bool=true);

        sf::Window* getSFMLHandle() const;
};

#endif