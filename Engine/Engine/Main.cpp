#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include <iostream>

//TODO: remove this header, it's only used to track memory leaks
#ifdef _DEBUG
#include <vld.h>
#endif

using namespace Engine;

int main(){
	srand((unsigned)time(0));
	#ifdef _WIN32
        if(GetConsoleWindow() == NULL){ 
            AllocConsole();
        }
        freopen("CONIN$", "r", stdin);freopen("CONOUT$", "w", stdout);freopen("CONOUT$", "w", stderr);
        #ifndef _DEBUG
            ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
        #endif
    #endif
    Detail::EngineClass::init("Engine",1024,768);
    Detail::EngineClass::run();
    Detail::EngineClass::destruct();
    FreeConsole();
    return 0;
}