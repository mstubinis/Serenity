#pragma once
#ifndef ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H
#define ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H

class  Shader;
class  ShaderProgram;

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/core/engine/events/Observer.h>

//Core since version 3.1 (GLSL 140)
class UniformBufferObject final : public Observer {
    friend class Shader;
    private:
        const char*   m_NameInShader              = "";
        uint32_t      m_SizeOfStruct              = 0;
        int           m_GlobalBindingPointNumber  = 0;
        GLuint        m_UBOObject                 = 0;

        void _load_CPU();
        void _unload_CPU();
        void _load_GPU();
        void _unload_GPU();
    public:
        static UniformBufferObject*   UBO_CAMERA;
        static GLint                  MAX_UBO_BINDINGS;
        static unsigned int           CUSTOM_UBO_AUTOMATIC_COUNT;

        UniformBufferObject(const char* nameInShader, uint32_t sizeofStruct, const int globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void onEvent(const Event&);
        inline GLuint address() const noexcept { return m_UBOObject; }

        void attachToShader(const ShaderProgram&);
        void updateData(void* data);
};
#endif