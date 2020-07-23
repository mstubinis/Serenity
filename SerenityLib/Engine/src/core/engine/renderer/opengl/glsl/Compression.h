#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H

namespace Engine::priv::opengl::glsl {
    class Compression final {
        public:
            static void convert(std::string& code, unsigned int versionNumber);
    };
};

#endif