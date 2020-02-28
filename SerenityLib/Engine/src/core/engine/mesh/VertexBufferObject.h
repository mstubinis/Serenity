#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD
#define ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>

struct BufferDataType final {enum Type {
    VertexArray  = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
};};

struct BufferDataDrawType final{enum Type {
    Unassigned   = 0,
    Static       = GL_STATIC_DRAW,
    Dynamic      = GL_DYNAMIC_DRAW,
    Stream       = GL_STREAM_DRAW,
_TOTAL};};

struct BufferObject {
    GLuint                    buffer   = 0;
    BufferDataDrawType::Type  drawType = BufferDataDrawType::Unassigned;
    BufferDataType::Type      type;
    size_t                    capacity = 0;

    BufferObject() = default;
    BufferObject(const BufferObject& other)                = delete;
    BufferObject& operator=(const BufferObject& other)     = delete;
    BufferObject(BufferObject&& other) noexcept            = delete;
    BufferObject& operator=(BufferObject&& other) noexcept = delete;

    virtual ~BufferObject() { destroy(); }

    void generate();
    void destroy();
    inline operator GLuint() const { return buffer; }

    void bind();

    void setData(size_t _size, const void* _data, BufferDataDrawType::Type _drawType);
    void setData(size_t _size, size_t _startingIndex, const void* _data);
    void setDataOrphan(const void* _data);

    template<typename T> void setData(const std::vector<T>& _data, BufferDataDrawType::Type _drawType) {
        drawType     = _drawType;
        size_t _size = _data.size() * sizeof(T);
        capacity     = _size;
        glBufferData(type, _size, _data.data(), drawType);
    }
    template<typename T> void setData(size_t _startingIndex, const std::vector<T>& _data) {
        if (drawType == BufferDataDrawType::Unassigned) {
            return;
        }
        glBufferSubData(type, _startingIndex, _data.size() * sizeof(T), _data.data());
    }
    template<typename T> void setDataOrphan(const std::vector<T>& _data) {
        if (capacity == 0) {
            return;
        }
        glBufferData(type, capacity, nullptr, drawType);
        glBufferSubData(type, 0, capacity, _data.data());
    }
};
struct VertexBufferObject final : public BufferObject{
    VertexBufferObject();
    ~VertexBufferObject()  = default;
};
struct ElementBufferObject final : public BufferObject {
    ElementBufferObject();
    ~ElementBufferObject() = default;
};

#endif