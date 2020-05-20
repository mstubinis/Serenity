#include <core/engine/renderer/opengl/OpenGL.h>
#include <core/engine/system/window/Window.h>
#include <unordered_map>

using namespace std;
using namespace Engine::priv;

unordered_map<string, string> VERSION_MAP;
unordered_map<string, string> POPULATE_VERSION_MAP() {
    unordered_map<string, string> ret;

    ret["1.1"] = "110";
    ret["2.0"] = "110";
    ret["2.1"] = "120";
    ret["3.0"] = "130";
    ret["3.1"] = "140";
    ret["3.2"] = "150";
    ret["3.3"] = "330";
    ret["4.0"] = "400";
    ret["4.1"] = "410";
    ret["4.2"] = "420";
    ret["4.3"] = "430";
    ret["4.4"] = "440";
    ret["4.5"] = "450";
    ret["4.6"] = "460";

    return ret;
}

const string OpenGL::getHighestGLSLVersion(const Window& window) {
    if (VERSION_MAP.size() == 0) {
        VERSION_MAP = POPULATE_VERSION_MAP();
    }
    //unordered_map<string, string> VERSION_MAP = POPULATE_VERSION_MAP();
    const auto& openglContext = const_cast<Window&>(window).getSFMLHandle().getSettings();
    const string version = to_string(openglContext.majorVersion) + "." + to_string(openglContext.minorVersion);
    return VERSION_MAP.at(version);
}