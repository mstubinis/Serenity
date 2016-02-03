#include "Engine.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(){
	#ifdef _WIN32 
	ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
    #endif

	Engine::EngineClass* program = new Engine::EngineClass("Engine",1024,768);

	program->run();
	delete program;

    return 0;
}