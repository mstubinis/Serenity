#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>

#include <time.h>
#include <SFML/Window.hpp>

#include "config/ConfigFile.h"

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

    ConfigFile config;
    const auto& info = config.readFromFile();

    EngineOptions options;
    options.window_title   = "Star Trek: Homeland Security";
    options.icon           = "../data/Textures/icon.png";
    options.width          = info.window_width;
    options.height         = info.window_height;
    options.window_mode    = info.getWindowModeInt();
    //options.show_console   = true;
    options.maximized      = info.window_maximized;
    options.aa_algorithm   = AntiAliasingAlgorithm::SMAA;
    options.argv           = argv;
    options.argc           = argc;

    Engine::epriv::Core::m_Engine = NEW Engine::epriv::EngineCore(options);
    Engine::epriv::Core::m_Engine->init(options);
    Engine::epriv::Core::m_Engine->run();

    SAFE_DELETE(Engine::epriv::Core::m_Engine);

    return 0;
}
