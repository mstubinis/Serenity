#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_LIGHTING_H
#define ENGINE_RENDERER_OPENGL_GLSL_LIGHTING_H

#include <core/engine/shaders/ShaderIncludes.h>
#include <string>

namespace Engine::priv::opengl::glsl {
    class Lighting final {
        public:
            static void convert(std::string& code, const unsigned int versionNumber, const ShaderType::Type shaderType);
    };
};

#endif