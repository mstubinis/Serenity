#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H
#define ENGINE_VERTEX_BUFFER_OBJECT_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <serenity/core/engine/system/TypeDefs.h>

enum class BufferDataType : uint32_t {
    VertexArray  = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferDataDrawType : uint32_t {
    Unassigned   = 0,
    Static       = GL_STATIC_DRAW,
    Dynamic      = GL_DYNAMIC_DRAW,
    Stream       = GL_STREAM_DRAW,
};

struct BufferObject {
    GLuint                buffer   = 0;
    BufferDataDrawType    drawType = BufferDataDrawType::Unassigned;
    BufferDataType        type     = BufferDataType::VertexArray;
    size_t                capacity = 0;

    BufferObject() = default;
    BufferObject(BufferDataType bufferDataType) 
        : type { bufferDataType }
    {}
    BufferObject(const BufferObject& other) = delete;
    BufferObject& operator=(const BufferObject& other) = delete;
    BufferObject(BufferObject&& other) noexcept 
        : buffer   { std::exchange(other.buffer, 0) }
        , drawType { std::move(other.drawType) }
        , type     { std::move(other.type) }
        , capacity { std::move(other.capacity) }
    {}
    BufferObject& operator=(BufferObject&& other) noexcept {
        buffer   = std::exchange(other.buffer, 0);
        drawType = std::move(other.drawType);
        type     = std::move(other.type);
        capacity = std::move(other.capacity);
        return *this;
    }
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

    void setData(size_t size, const void* data, BufferDataDrawType drawType_) noexcept {
        drawType = drawType_;
        if (drawType == BufferDataDrawType::Unassigned) {
            return;
        }
        if (size > capacity) {
            capacity = size;
            glBufferData((GLuint)type, size, data, (GLuint)drawType);
        }else{
            glBufferSubData((GLuint)type, 0, size, data);
        }
    }
    void setData(size_t size, size_t startingIndex, const void* data) const noexcept {
        if (drawType == BufferDataDrawType::Unassigned) {
            return;
        }
        glBufferSubData((GLuint)type, startingIndex, size, data);
    }
    void setDataOrphan(const void* data) const noexcept {
        if (drawType == BufferDataDrawType::Unassigned || capacity == 0) {
            return;
        }
        glBufferData((GLuint)type, capacity, nullptr, (GLuint)drawType);
        glBufferSubData((GLuint)type, 0, capacity, data);
    }

    template<class T> inline void setData(const std::vector<T>& data, BufferDataDrawType drawType) noexcept { setData(data.size() * sizeof(T), (void*)data.data(), drawType); }
    template<class T> inline void setData(size_t startingIndex, const std::vector<T>& data) const noexcept { setData(data.size() * sizeof(T), startingIndex, (void*)data.data()); }
    template<class T> inline void setDataOrphan(const std::vector<T>& data) const noexcept { setDataOrphan((void*)data.data()); }
};
struct VertexBufferObject final : public BufferObject {
    VertexBufferObject() 
        : BufferObject(BufferDataType::VertexArray)
    {}
    VertexBufferObject(const VertexBufferObject& other)            = delete;
    VertexBufferObject& operator=(const VertexBufferObject& other) = delete;
    VertexBufferObject(VertexBufferObject&& other) noexcept 
        : BufferObject{ std::move(other) }
    {}
    VertexBufferObject& operator=(VertexBufferObject&& other) noexcept {
        BufferObject::operator=(std::move(other));
        return *this;
    }
};
struct ElementBufferObject final : public BufferObject {
    ElementBufferObject() 
        : BufferObject(BufferDataType::ElementArray)
    {}
    ElementBufferObject(const ElementBufferObject& other)            = delete;
    ElementBufferObject& operator=(const ElementBufferObject& other) = delete;
    ElementBufferObject(ElementBufferObject&& other) noexcept
        : BufferObject{ std::move(other) }
    {}
    ElementBufferObject& operator=(ElementBufferObject&& other) noexcept {
        BufferObject::operator=(std::move(other));
        return *this;
    }
};

#endif