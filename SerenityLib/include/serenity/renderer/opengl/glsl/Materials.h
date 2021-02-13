#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_MATERIALS_H
#define ENGINE_RENDERER_OPENGL_GLSL_MATERIALS_H

#include <serenity/resources/shader/ShaderIncludes.h>
#include <string>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv::opengl::glsl {
    class Materials final {
        public:
            static void convert(std::string& code, uint32_t versionNumber, ShaderType shaderType);
    };
};

#endif