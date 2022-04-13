#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H
#define ENGINE_VERTEX_BUFFER_OBJECT_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

class BufferDataType final {
    public:
        enum Type : uint32_t {
            VertexArray = GL_ARRAY_BUFFER,
            ElementArray = GL_ELEMENT_ARRAY_BUFFER,
        };
        BUILD_ENUM_CLASS_MEMBERS(BufferDataType, Type)
};
class BufferDataDrawType final {
    public:
        enum Type : uint32_t {
            Unassigned = 0,
            Static = GL_STATIC_DRAW,
            Dynamic = GL_DYNAMIC_DRAW,
            Stream = GL_STREAM_DRAW,
        };
        BUILD_ENUM_CLASS_MEMBERS(BufferDataDrawType, Type)
};

class BufferObject {
    private:
        size_t                m_Capacity       = 0;
        GLuint                m_BufferGLHandle = 0;
        BufferDataDrawType    m_DrawType       = BufferDataDrawType::Unassigned;
        BufferDataType        m_Type           = BufferDataType::VertexArray;
    public:
        BufferObject() = delete;
        BufferObject(BufferDataType bufferDataType) 
            : m_Type{ bufferDataType }
        {}

        ~BufferObject() { 
            destroy(); 
        }

        void generate() noexcept {
            if (!m_BufferGLHandle) {
                glGenBuffers(1, &m_BufferGLHandle);
            }
        }
        void destroy() noexcept {
            if (m_BufferGLHandle) {
                glDeleteBuffers(1, &m_BufferGLHandle);
                m_BufferGLHandle = 0;
            }
        }
        inline operator GLuint() const noexcept { 
            return m_BufferGLHandle; 
        }
        inline void bind() const noexcept { 
            glBindBuffer(m_Type, m_BufferGLHandle); 
        }

        void setData(size_t size, const void* data, BufferDataDrawType drawType_) noexcept {
            m_DrawType = drawType_;
            if (m_DrawType == BufferDataDrawType::Unassigned) {
                return;
            }
            if (size > m_Capacity) {
                m_Capacity = size;
                glBufferData(m_Type, size, data, m_DrawType);
            } else {
                glBufferSubData(m_Type, 0, size, data);
            }
        }
        void setData(size_t size, size_t startingIndex, const void* data) const noexcept {
            if (m_DrawType == BufferDataDrawType::Unassigned) {
                return;
            }
            glBufferSubData(m_Type, startingIndex, size, data);
        }
        void setDataOrphan(const void* data) const noexcept {
            if (m_DrawType == BufferDataDrawType::Unassigned || m_Capacity == 0) {
                return;
            }
            glBufferData(m_Type, m_Capacity, nullptr, m_DrawType);
            glBufferSubData(m_Type, 0, m_Capacity, data);
        }

        template<class T> inline void setData(const std::vector<T>& data, BufferDataDrawType drawType) noexcept { 
            setData(data.size() * sizeof(T), (void*)data.data(), drawType); 
        }
        template<class T> inline void setData(size_t startingIndex, const std::vector<T>& data) const noexcept { 
            setData(data.size() * sizeof(T), startingIndex, (void*)data.data()); 
        }
        template<class T> inline void setDataOrphan(const std::vector<T>& data) const noexcept { 
            setDataOrphan((void*)data.data()); 
        }
};

#endif