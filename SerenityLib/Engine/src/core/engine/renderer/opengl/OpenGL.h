#pragma once
#ifndef ENGINE_RENDERER_OPENGL_H
#define ENGINE_RENDERER_OPENGL_H

class Window;

namespace Engine::priv {
    class OpenGL final {
        private:

        public:
            static std::string getHighestGLSLVersion(Window& window) noexcept;
    };
};

#endif