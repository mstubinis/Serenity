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
    BufferObject();
    BufferObject(const BufferObject& other) = delete;
    BufferObject& operator=(const BufferObject& other) = delete;
    BufferObject(BufferObject&& other) noexcept = delete;
    BufferObject& operator=(BufferObject&& other) noexcept = delete;

    virtual ~BufferObject() { destroy(); }

    void generate();
    void destroy();
    inline operator GLuint() const { return buffer; }

    virtual void bind() { }
    virtual void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { }
    virtual void bufferSubData(size_t _size, const void* _data) { }
    virtual void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) { }
};
struct VertexBufferObject : public BufferObject{
    VertexBufferObject() = default;
    ~VertexBufferObject() = default;
    VertexBufferObject(const VertexBufferObject& other) = delete;
    VertexBufferObject& operator=(const VertexBufferObject& other) = delete;
    VertexBufferObject(VertexBufferObject&& other) noexcept = delete;
    VertexBufferObject& operator=(VertexBufferObject&& other) noexcept = delete;

    void bind() override;
    void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferSubData(size_t _size, const void* _data) override;
    void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) override;
};
struct ElementBufferObject : public BufferObject {
    ElementBufferObject() = default;
    ~ElementBufferObject() = default;
    ElementBufferObject(const ElementBufferObject& other) = delete;
    ElementBufferObject& operator=(const ElementBufferObject& other) = delete;
    ElementBufferObject(ElementBufferObject&& other) noexcept = delete;
    ElementBufferObject& operator=(ElementBufferObject&& other) noexcept = delete;

    void bind() override;
    void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferSubData(size_t _size, const void* _data) override;
    void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) override;
};

#endif