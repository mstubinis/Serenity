#pragma once
#ifndef ENGINE_RENDERER_OPENGL_H
#define ENGINE_RENDERER_OPENGL_H

//#define ENGINE_GL_DEBUG

class Window;

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/system/Macros.h>

#define GLASSERT(x) if((!x)) __debugbreak();

#if defined(ENGINE_GL_DEBUG)
    #define GLCall(x) GLClearErrors(); x; GLASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
    #define GLCall(x) x;
#endif

static void GLClearErrors() {
    GLenum error = glGetError(); //will infinetly return error 1282 if called once opengl context is destroyed
    while (error != GL_NO_ERROR) {
        error = glGetError();
    }
}
static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        ENGINE_PRODUCTION_LOG("OpenGL Error: file: " << file << ", function: " << function << ", line: " << line << ", error: " << gluErrorString(error))
        return false;
    }
    return true;
}

namespace Engine::priv {
    class OpenGL final {
        private:

        public:
            static std::string getHighestGLSLVersion(Window& window) noexcept;
    };
};

#endif