#pragma once
#ifndef ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD
#define ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD

//information about individual attributes (example: position, uv, normals, etc)
struct VertexAttributeInfo final{
    int      size;
    int      type;
    size_t   typeSize;
    bool     normalized;
    int      stride;
    size_t   offset;

    VertexAttributeInfo(const int _size, const int _type, const bool _normalized, const int _stride, const size_t _offset, const size_t _typeSize) {
        size       = _size;
        type       = _type;
        normalized = _normalized;
        stride     = _stride;
        offset     = _offset;
        typeSize   = _typeSize;
    }
    VertexAttributeInfo()                                                = delete;
    ~VertexAttributeInfo()                                               = default;
    VertexAttributeInfo(const VertexAttributeInfo& other)                = delete;
    VertexAttributeInfo& operator=(const VertexAttributeInfo& other)     = delete;
    VertexAttributeInfo(VertexAttributeInfo&& other) noexcept            = default;
    VertexAttributeInfo& operator=(VertexAttributeInfo&& other) noexcept = default;
};
#endif