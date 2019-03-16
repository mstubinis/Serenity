#pragma once
#ifndef ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD
#define ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD

struct VertexAttributeInfo final{
    int      size;
    int      type;
    size_t   typeSize;
    bool     normalized;
    int      stride;
    size_t   offset;

    VertexAttributeInfo(int _size, int _type, bool _normalized, int _stride, size_t _offset, size_t _typeSize) {
        size = _size; type = _type; normalized = _normalized; stride = _stride; offset = _offset; typeSize = _typeSize;
    }
    VertexAttributeInfo()                                                = delete;
    ~VertexAttributeInfo()                                               = default;
    VertexAttributeInfo(const VertexAttributeInfo& other)                = delete;
    VertexAttributeInfo& operator=(const VertexAttributeInfo& other)     = delete;
    VertexAttributeInfo(VertexAttributeInfo&& other) noexcept            = default;
    VertexAttributeInfo& operator=(VertexAttributeInfo&& other) noexcept = default;
};
#endif