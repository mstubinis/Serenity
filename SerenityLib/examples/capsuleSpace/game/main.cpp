#include <core/engine/Engine.h>

#include <time.h>
#include <memory>
#include <iostream>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif
#ifdef _DEBUG
//#include <vld.h> //memory leak printing
#endif

using namespace std;

int main(int argc, char* argv[]) {
    srand((unsigned)time(0));
#ifdef _WIN32
    if (GetConsoleWindow() == NULL) { AllocConsole(); }
    freopen("CONIN$", "r", stdin); freopen("CONOUT$", "w", stdout); freopen("CONOUT$", "w", stderr);

    unordered_map<string, bool> args;
    locale loc;
    for (int i = 0; i < argc; i++) {
        const string& key = string(argv[i]);
        string lowerKey = "";
        for (unsigned int j = 0; j < key.length(); ++j)
            lowerKey += std::tolower(key[j], loc);
        args.emplace(lowerKey, true);
    }
    if (!args.count("console")) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
    }
#endif
    Engine::init("Capsule Space", 1024, 768);
    Engine::run();

#ifdef _WIN32
	if (GetConsoleWindow() != NULL) {
		//FreeConsole(); //erroring out for some reason
	}
#endif
    return 0;
}