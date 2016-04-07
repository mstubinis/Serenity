#include "Engine.h"
#include "Engine_Events.h"
#include <iostream>

#if defined(_WIN32)
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
	#if defined(_WIN32)
		if(GetConsoleWindow() == NULL){ AllocConsole(); }
		#if !defined(ENGINE_DEBUG)
			ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
		#endif
		#if defined(ENGINE_DIRECTX)
			Engine::Detail::EngineClass::m_RenderingAPI = ENGINE_RENDERING_API_DIRECTX;
		#endif
	#endif

    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        Engine::Detail::EngineClass::initOpenGL(appName,appWidth,appHeight);
        Engine::Detail::EngineClass::run();
        Engine::Detail::EngineClass::destruct();
		FreeConsole();
        return 0;
	}
	else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        // enter the main loop:
		Engine::Detail::EngineClass::initDirectX("WindowClass1",appName,GetModuleHandle(0),1,appWidth,appHeight);
        Engine::Detail::EngineClass::run();

        //end main loop
        MSG msg;// this struct holds Windows event messages
        while(GetMessage(&msg, NULL, 0, 0)){// wait for the next message in the queue, store the result in 'msg'
            TranslateMessage(&msg);// translate keystroke messages into the right format
            DispatchMessage(&msg);// send the message to the WindowProc function
        }
		Engine::Detail::EngineClass::destruct();
		FreeConsole();
        return msg.wParam;// return this part of the WM_QUIT message to Windows
	}
}