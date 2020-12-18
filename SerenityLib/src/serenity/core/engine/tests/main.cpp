#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <iostream>

int main(int argc, char** argv) {
    if (GetConsoleWindow() == NULL) {
        AllocConsole();
    }
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    int result = 0;
    result = Catch::Session().run(argc, argv);
    std::cin.get();

    return result;
}