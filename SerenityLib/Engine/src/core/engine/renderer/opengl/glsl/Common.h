#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMMON_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMMON_H

namespace Engine::priv::opengl::glsl {
    class Common final {
        public:
            static void convert(std::string& code, unsigned int versionNumber);
    };
};

#endif