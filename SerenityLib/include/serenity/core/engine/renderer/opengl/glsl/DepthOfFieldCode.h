#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H
#define ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H

#include <serenity/core/engine/shaders/ShaderIncludes.h>
#include <string>
#include <serenity/core/engine/system/TypeDefs.h>

namespace Engine::priv::opengl::glsl {
    class DepthOfFieldCode final {
        public:
            static void convert(std::string& code, uint32_t versionNumber, ShaderType shaderType);
    };
};

#endif