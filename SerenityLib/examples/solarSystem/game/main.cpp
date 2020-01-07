#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>

#include <time.h>
#include <memory>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#endif

using namespace std;

int main(int argc, char* argv[]) {
    #ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
        srand(static_cast<unsigned>(time(0)));


    EngineOptions options;
    options.window_title   = "Star Trek: Homeland Security";
    options.width          = sf::VideoMode::getDesktopMode().width - 30;
    options.height         = sf::VideoMode::getDesktopMode().height - 120;
    options.fullscreen     = false;
    //options.show_console   = true;
    options.maximized      = true;
    options.aa_algorithm   = AntiAliasingAlgorithm::SMAA;
    options.argv           = argv;
    options.argc           = argc;

    Engine::epriv::Core::m_Engine = NEW Engine::epriv::EngineCore(options);
    Engine::epriv::Core::m_Engine->init(options);
    Engine::epriv::Core::m_Engine->run();

    SAFE_DELETE(Engine::epriv::Core::m_Engine);

    //#ifdef _WIN32
    //    if (GetConsoleWindow() != NULL) {
            //FreeConsole(); //erroring out for some reason
    //    }
    //#endif

    return 0;
}
