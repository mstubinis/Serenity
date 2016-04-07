#include "Engine.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

int main(){
    const char* appName = "Engine";
    const unsigned int appWidth = 1024;
    const unsigned int appHeight = 768;
    srand((unsigned)time(0));
	Engine::Detail::EngineClass::m_RenderingAPI = ENGINE_RENDERING_API_OPENGL;
	//Engine::Detail::EngineClass::m_RenderingAPI = ENGINE_RENDERING_API_DIRECTX;
	#ifdef _WIN32
		if(GetConsoleWindow() == NULL){ 
			AllocConsole();
		}
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		#ifndef ENGINE_DEBUG
			ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
		#endif
	#endif

	Engine::Detail::EngineClass::init(appName,appWidth,appHeight);

	#ifdef _WIN32
	if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        Engine::Detail::EngineClass::runDirectX();
	}
	#endif
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        Engine::Detail::EngineClass::run();
		Engine::Detail::EngineClass::destruct();
		FreeConsole();
	}
	return 0;
}