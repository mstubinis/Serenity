#include "Engine.h"
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#endif

#ifdef _DEBUG
    #include <vld.h> //TODO: remove this header eventually, it's only used to track memory leaks in debug mode
#endif

int main(){
    srand((unsigned)time(0));
    #ifdef _WIN32
        if(GetConsoleWindow() == NULL){ AllocConsole(); }
        freopen("CONIN$", "r", stdin);freopen("CONOUT$", "w", stdout);freopen("CONOUT$", "w", stderr);
        #ifndef _DEBUG
            //ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
        #endif
    #endif

    Engine::init("Engine",1024,768);
    Engine::run();

    #ifdef _WIN32
        FreeConsole();
    #endif
    return 0;
}