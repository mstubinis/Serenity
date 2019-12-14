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
        srand((unsigned)time(0));

    #ifdef _WIN32
        if (GetConsoleWindow() == NULL) {
            AllocConsole();
        }
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        unordered_set<string> args;
        locale loc;
        for (int i = 0; i < argc; i++) {
            const string key = string(argv[i]);
            string lowerKey  = "";
            for (size_t j = 0; j < key.length(); ++j)
                lowerKey += std::tolower(key[j], loc);
            args.insert(lowerKey);
        }
        if (!args.count("console")) {
            //ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
        }
    #endif

        EngineOptions options;
        options.window_title   = "Solar System";
        options.width          = 1024;
        options.height         = 768;
        options.fullscreen     = false;
        options.aa_algorithm   = AntiAliasingAlgorithm::SMAA;

        Engine::epriv::Core::m_Engine = NEW Engine::epriv::EngineCore(options);
        Engine::epriv::Core::m_Engine->init(options);
        Engine::epriv::Core::m_Engine->run();

        SAFE_DELETE(Engine::epriv::Core::m_Engine);
    #ifdef _WIN32
        if (GetConsoleWindow() != NULL) {
            //FreeConsole(); //erroring out for some reason
        }
    #endif
    return 0;
}
