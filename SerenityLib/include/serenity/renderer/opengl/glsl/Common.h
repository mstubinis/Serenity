#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMMON_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMMON_H

#include <string>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv::opengl::glsl {
    class Common final {
        public:
            static void convert(std::string& code, uint32_t versionNumber);
    };
};

#endif