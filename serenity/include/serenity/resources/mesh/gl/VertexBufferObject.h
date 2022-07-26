#pragma once
#ifndef ENGINE_GL_VERTEX_BUFFER_OBJECT_H
#define ENGINE_GL_VERTEX_BUFFER_OBJECT_H

#include <serenity/resources/mesh/gl/MeshGLIncludes.h>

class VertexBufferObject {
    private:
        size_t                m_Capacity       = 0;
        GLuint                m_BufferGLHandle = 0;
        BufferDataDrawType    m_DrawType       = BufferDataDrawType::Unassigned;
    public:
        ~VertexBufferObject();

        void generate() noexcept;
        void destroy() noexcept;
        [[nodiscard]] inline operator GLuint() const noexcept { return m_BufferGLHandle; }
        void bind() const noexcept;

        void setData(size_t sizeInBytes, const void* data, BufferDataDrawType) noexcept;
        void setData(size_t sizeInBytes, size_t startingIndex, const void* data) const noexcept;
        void setDataOrphan(const void* data) const noexcept;

        void specifyVertexAttribute(size_t index, int numComponents, size_t type, bool normalized, size_t stride, size_t offset);
};

#endif