#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H
#define ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H

#include <core/engine/shaders/ShaderIncludes.h>

namespace Engine::priv::opengl::glsl {
    class DepthOfFieldCode final {
        public:
            static void convert(std::string& code, unsigned int versionNumber, ShaderType shaderType);
    };
};

#endif