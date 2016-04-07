#include "Engine.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

int main(){
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

    Engine::Detail::EngineClass::init(Engine::Detail::EngineClass::m_RenderingAPI,"Engine",1024,768);
    Engine::Detail::EngineClass::run(Engine::Detail::EngineClass::m_RenderingAPI);
    Engine::Detail::EngineClass::destruct();
    FreeConsole();
    return 0;
}