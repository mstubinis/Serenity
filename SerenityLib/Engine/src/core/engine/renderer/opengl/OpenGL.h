#pragma once
#ifndef ENGINE_RENDERER_OPENGL_H
#define ENGINE_RENDERER_OPENGL_H

class Window;

#include <string>

namespace Engine::priv {
    class OpenGL final {
        private:

        public:
            static const std::string getHighestGLSLVersion(const Window& window);
    };
};

#endif