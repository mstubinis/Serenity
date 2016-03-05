#include "Engine.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(){
	#ifdef _WIN32 
	ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
    #endif

	Engine::Detail::EngineClass::init("Engine",1024,768);
	Engine::Detail::EngineClass::run();
	Engine::Detail::EngineClass::destruct();

    return 0;
}