#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H

#include <string>
#include <serenity/core/engine/system/TypeDefs.h>

namespace Engine::priv::opengl::glsl {
    class Compression final {
        public:
            static void convert(std::string& code, uint32_t versionNumber);
    };
};

#endif