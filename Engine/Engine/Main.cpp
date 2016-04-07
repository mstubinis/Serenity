#include "Engine.h"
#include "Engine_Resources.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

using namespace Engine;

int main(){
    Resources::Detail::ResourceManagement::m_RenderingAPI = ENGINE_RENDERING_API_OPENGL;
    //Resources::Detail::ResourceManagement::m_RenderingAPI = ENGINE_RENDERING_API_DIRECTX;
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
    Detail::EngineClass::run(Resources::Detail::ResourceManagement::m_RenderingAPI);
    Detail::EngineClass::destruct();
    FreeConsole();
    return 0;
}