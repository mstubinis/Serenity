#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

using namespace Engine;

int main(){
    Resources::Detail::ResourceManagement::m_RenderingAPI = ENGINE_RENDERING_API_OPENGL;
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

    Detail::EngineClass::init(Resources::Detail::ResourceManagement::m_RenderingAPI,"Engine",1024,768);
	//Resources::getWindow()->setRenderingAPI(ENGINE_RENDERING_API_DIRECTX);
    Detail::EngineClass::run();
    Detail::EngineClass::destruct();
    FreeConsole();
    return 0;
}