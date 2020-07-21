#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/renderer/opengl/OpenGL.h>
#include <core/engine/system/window/Window.h>

using namespace Engine::priv;

std::unordered_map<std::string, std::string> VERSION_MAP;
std::unordered_map<std::string, std::string> POPULATE_VERSION_MAP() {
    std::unordered_map<std::string, std::string> ret;

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

std::string OpenGL::getHighestGLSLVersion(Window& window) noexcept {
    if (VERSION_MAP.size() == 0) {
        VERSION_MAP = POPULATE_VERSION_MAP();
    }
    //unordered_map<string, string> VERSION_MAP = POPULATE_VERSION_MAP();
    auto& openglContext = window.getSFMLHandle().getSettings();
    std::string version = std::to_string(openglContext.majorVersion) + "." + std::to_string(openglContext.minorVersion);
    return VERSION_MAP.at(version);
}