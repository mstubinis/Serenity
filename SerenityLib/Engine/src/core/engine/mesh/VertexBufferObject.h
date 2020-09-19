#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H
#define ENGINE_VERTEX_BUFFER_OBJECT_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

enum class BufferDataType : unsigned int {
    VertexArray  = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferDataDrawType : unsigned int {
    Unassigned   = 0,
    Static       = GL_STATIC_DRAW,
    Dynamic      = GL_DYNAMIC_DRAW,
    Stream       = GL_STREAM_DRAW,
};

struct BufferObject : public Engine::NonCopyable, public Engine::NonMoveable {
    GLuint                buffer   = 0;
    BufferDataDrawType    drawType = BufferDataDrawType::Unassigned;
    BufferDataType        type     = BufferDataType::VertexArray;
    size_t                capacity = 0;

    virtual ~BufferObject() { destroy(); }

    void generate() noexcept {
        if (!buffer) {
            glGenBuffers(1, &buffer);
        }
    }
    void destroy() noexcept {
        if (buffer) {
            glDeleteBuffers(1, &buffer);
            buffer = 0;
        }
    }
    inline operator GLuint() const noexcept { return buffer; }
    inline void bind() const noexcept { glBindBuffer((GLuint)type, buffer); }

    void setData(size_t size_, const void* data_, BufferDataDrawType drawType_) noexcept {
        drawType = drawType_;
        if (drawType == BufferDataDrawType::Unassigned) {
            return;
        }
        if (size_ > capacity) {
            capacity = size_;
            glBufferData((GLuint)type, size_, data_, (GLuint)drawType);
        }else{
            glBufferSubData((GLuint)type, 0, size_, data_);
        }
    }
    void setData(size_t size_, size_t startingIndex_, const void* data_) const noexcept {
        if (drawType == BufferDataDrawType::Unassigned) {
            return;
        }
        glBufferSubData((GLuint)type, startingIndex_, size_, data_);
    }
    void setDataOrphan(const void* data_) const noexcept {
        if (drawType == BufferDataDrawType::Unassigned || capacity == 0) {
            return;
        }
        glBufferData((GLuint)type, capacity, nullptr, (GLuint)drawType);
        glBufferSubData((GLuint)type, 0, capacity, data_);
    }

    template<class T> inline void setData(const std::vector<T>& data_, BufferDataDrawType drawType_) noexcept {
        setData(data_.size() * sizeof(T), (void*)data_.data(), drawType_);
    }
    template<class T> inline void setData(size_t startingIndex_, const std::vector<T>& data_) const noexcept {
        setData(data_.size() * sizeof(T), startingIndex_, (void*)data_.data());
    }
    template<class T> inline void setDataOrphan(const std::vector<T>& data_) const noexcept {
        setDataOrphan((void*)data_.data());
    }
};
struct VertexBufferObject final : public BufferObject {
    VertexBufferObject() {
        type = BufferDataType::VertexArray;
    }
};
struct ElementBufferObject final : public BufferObject {
    ElementBufferObject() {
        type = BufferDataType::ElementArray;
    }
};

#endif