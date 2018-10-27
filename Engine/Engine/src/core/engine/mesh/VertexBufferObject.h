#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD
#define ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD

#include <GL/glew.h>
#include <GL/GL.h>

struct BufferDataType {enum Type {
    Static = GL_STATIC_DRAW,
    Dynamic = GL_DYNAMIC_DRAW,
    Stream = GL_STREAM_DRAW,
_TOTAL};};
struct BufferObject {
    GLuint buffer;
    BufferObject() :buffer(0) { if (!buffer) { glGenBuffers(1, &buffer); } }

    BufferObject(const BufferObject& other) = delete;
    BufferObject& operator=(const BufferObject& other) = delete;
    BufferObject(BufferObject&& other) noexcept = delete;
    BufferObject& operator=(BufferObject&& other) noexcept = delete;

    virtual ~BufferObject() { destroy(); }

    void destroy() { if (buffer) { glDeleteBuffers(1, &buffer); buffer = 0; } }
    inline operator GLuint() const { return buffer; }

    virtual void bind() { }
    virtual void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { }
    virtual void bufferSubData(size_t _size, const void* _data) { }
};
struct VertexBufferObject : public BufferObject{
    VertexBufferObject() = default;
    ~VertexBufferObject() = default;
    VertexBufferObject(const VertexBufferObject& other) = delete;
    VertexBufferObject& operator=(const VertexBufferObject& other) = delete;
    VertexBufferObject(VertexBufferObject&& other) noexcept = delete;
    VertexBufferObject& operator=(VertexBufferObject&& other) noexcept = delete;

    void bind() { glBindBuffer(GL_ARRAY_BUFFER, buffer); }
    void bufferData(size_t _size,const void* _data, BufferDataType::Type _drawType) { glBufferData(GL_ARRAY_BUFFER, _size, _data, _drawType); }
    void bufferSubData(size_t _size, const void* _data) { glBufferSubData(GL_ARRAY_BUFFER, 0, _size, _data); }
};
struct ElementBufferObject : public BufferObject {
    ElementBufferObject() = default;
    ~ElementBufferObject() = default;
    ElementBufferObject(const ElementBufferObject& other) = delete;
    ElementBufferObject& operator=(const ElementBufferObject& other) = delete;
    ElementBufferObject(ElementBufferObject&& other) noexcept = delete;
    ElementBufferObject& operator=(ElementBufferObject&& other) noexcept = delete;

    void bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); }
    void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { glBufferData(GL_ELEMENT_ARRAY_BUFFER, _size, _data, _drawType); }
    void bufferSubData(size_t _size, const void* _data) { glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _size, _data); }
};

#endif