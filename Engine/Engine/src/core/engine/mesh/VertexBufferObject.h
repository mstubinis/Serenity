#pragma once
#ifndef ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD
#define ENGINE_VERTEX_BUFFER_OBJECT_H_INCLUDE_GUARD

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct BufferDataType {enum Type {
    Unassigned,
    Static = GL_STATIC_DRAW,
    Dynamic = GL_DYNAMIC_DRAW,
    Stream = GL_STREAM_DRAW,
_TOTAL};};
struct BufferObject {
    GLuint buffer;
    BufferDataType::Type drawType;


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
    virtual void bufferData(size_t _size, const void* _data) { }
    virtual void bufferDataMap(size_t _size, const void* _data, BufferDataType::Type _drawType) { }
    virtual void bufferDataMap(size_t _size, const void* _data) { }
    virtual void bufferDataOrphan(size_t _size, const void* _data) { }
    virtual void bufferDataOrphan(size_t _size, size_t _startingIndex, const void* _data) { }
    virtual void bufferSubData(size_t _size, const void* _data) { }
    virtual void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) { }
};
struct VertexBufferObject final : public BufferObject{
    VertexBufferObject() = default;
    ~VertexBufferObject() = default;

    void bind() override;
    void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferData(size_t _size, const void* _data) override;
    void bufferDataMap(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferDataMap(size_t _size, const void* _data) override;
    void bufferDataOrphan(size_t _size, const void* _data) override;
    void bufferDataOrphan(size_t _size, size_t _startingIndex, const void* _data) override;
    void bufferSubData(size_t _size, const void* _data) override;
    void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) override;
};
struct ElementBufferObject final : public BufferObject {
    ElementBufferObject() = default;
    ~ElementBufferObject() = default;

    void bind() override;
    void bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferData(size_t _size, const void* _data) override;
    void bufferDataMap(size_t _size, const void* _data, BufferDataType::Type _drawType) override;
    void bufferDataMap(size_t _size, const void* _data) override;
    void bufferDataOrphan(size_t _size, const void* _data) override;
    void bufferDataOrphan(size_t _size, size_t _startingIndex, const void* _data) override;
    void bufferSubData(size_t _size, const void* _data) override;
    void bufferSubData(size_t _size, size_t _startingIndex, const void* _data) override;
};

#endif