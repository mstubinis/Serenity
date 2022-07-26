#pragma once
#ifndef ENGINE_GL_INDEX_BUFFER_OBJECT_H
#define ENGINE_GL_INDEX_BUFFER_OBJECT_H

#include <serenity/resources/mesh/gl/MeshGLIncludes.h>

class IndexBufferObject {
    private:
        size_t                m_Capacity       = 0;
        GLuint                m_BufferGLHandle = 0;
        BufferDataDrawType    m_DrawType       = BufferDataDrawType::Unassigned;
    public:
        ~IndexBufferObject();

        void generate() noexcept;
        void destroy() noexcept;
        [[nodiscard]] inline operator GLuint() const noexcept { return m_BufferGLHandle; }
        void bind() const noexcept;

        void setData(size_t sizeInBytes, const void* data, BufferDataDrawType) noexcept;
        void setData(size_t sizeInBytes, size_t startingIndex, const void* data) const noexcept;
        void setDataOrphan(const void* data) const noexcept;

        template<class T> inline void setData(const std::vector<T>& data, BufferDataDrawType drawType) noexcept {
            setData(data.size() * sizeof(T), static_cast<const void*>(data.data()), drawType);
        }
        template<class T> inline void setData(size_t startingIndex, const std::vector<T>& data) const noexcept {
            setData(data.size() * sizeof(T), startingIndex, static_cast<const void*>(data.data()));
        }
        template<class CONTAINER> inline void setDataOrphan(const CONTAINER& container) const noexcept {
            setDataOrphan(static_cast<const void*>(container.data()));
        }
};

#endif