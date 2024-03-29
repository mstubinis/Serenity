#pragma once
#ifndef ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H
#define ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H

class  Shader;
class  ShaderProgram;
class  UniformBufferObjectMapper;
class  UniformBufferObject;

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/events/Observer.h>
#include <serenity/resources/Resource.h>

//RAII wrapper for mapping uniform buffer block data
class UniformBufferObjectMapper final {
    private:
        void* m_Ptr = nullptr;
    public:
        UniformBufferObjectMapper() = delete;
        UniformBufferObjectMapper(UniformBufferObject&);
        UniformBufferObjectMapper(const UniformBufferObjectMapper&)                = delete;
        UniformBufferObjectMapper& operator=(const UniformBufferObjectMapper&)     = delete;
        UniformBufferObjectMapper(UniformBufferObjectMapper&&) noexcept            = delete;
        UniformBufferObjectMapper& operator=(UniformBufferObjectMapper&&) noexcept = delete;
        ~UniformBufferObjectMapper();

        inline void* getPtr() noexcept { return m_Ptr; }
};

class UniformBufferObject final : public Observer, public Resource<UniformBufferObject> {
    friend class  Shader;
    friend class  UniformBufferObjectMapper;
    public:
        static GLint     MAX_UBO_BINDINGS;
        static uint32_t  CUSTOM_UBO_AUTOMATIC_COUNT;
    private:
        uint32_t      m_SizeOfStruct              = 0;
        int           m_GlobalBindingPointNumber  = 0;
        GLuint        m_UBOObject                 = 0;

        void internal_load_CPU();
        void internal_unload_CPU();
        void internal_load_GPU();
        void internal_unload_GPU();
    public:
        UniformBufferObject(std::string_view nameInShader, uint32_t sizeofStruct, const int globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void onEvent(const Event&);
        inline GLuint address() const noexcept { return m_UBOObject; }

        bool attachToShaderProgram(ShaderProgram&);

        void updateData(void* data);
};
#endif