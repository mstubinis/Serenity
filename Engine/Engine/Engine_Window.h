#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <string>
#include <SFML/Window.hpp>

#if defined(_WIN32)
#include <windows.h>
#include <windowsx.h>
#endif

typedef unsigned int uint;

class Engine_Window final{
	private:
		const char* m_WindowName;
		sf::Window* m_SFMLWindow;
		HWND m_DirectXWindow;
		uint m_Width;
		uint m_Height;
		void _createOpenGLWindow(const char* name,uint width,uint height);
		void _createDirectXWindow(const char* wCName,const char* name,HINSTANCE,int nCmdShow,uint width,uint height,uint xPos, uint yPos);
	public:
		Engine_Window(const char* name,uint width,uint height);
		Engine_Window(const char* wCName,const char* name,HINSTANCE,int nCmdShow,uint width,uint height,uint xPos, uint yPos);
		~Engine_Window();

		const char* name();
		sf::Vector2u getSize();
		void setName(const char* name);
		void setIcon(uint width, uint height, const sf::Uint8* pixels);
		void setMouseCursorVisible(bool);
		void setKeyRepeatEnabled(bool);
		void setVerticalSyncEnabled(bool);
		void close();

		bool hasFocus();
		bool isOpen();
		bool pollEvent(sf::Event&);
		void setFullScreen(bool);

		void display();

		sf::Window* getOpenGLHandle();
		HWND& getDirectXHandle();

		#if defined(_WIN32)
		void setRenderingAPI(uint);
		#endif
};

#endif