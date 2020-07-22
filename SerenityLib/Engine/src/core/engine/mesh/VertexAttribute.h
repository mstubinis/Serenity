#pragma once
#ifndef ENGINE_VERTEX_ATTRIBUTE_H
#define ENGINE_VERTEX_ATTRIBUTE_H

//information about individual attributes (example: position, uv, normals, etc)
struct VertexAttributeInfo final{
    int      size;
    int      type;
    size_t   typeSize;
    int      stride;
    size_t   offset;
    bool     normalized;

    VertexAttributeInfo(const int size_, const int type_, const bool normalized_, const int stride_, const size_t offset_, const size_t typeSize_) {
        size       = size_;
        type       = type_;
        normalized = normalized_;
        stride     = stride_;
        offset     = offset_;
        typeSize   = typeSize_;
    }
    VertexAttributeInfo()                                                = delete;

    VertexAttributeInfo(const VertexAttributeInfo& other)                = default;
    VertexAttributeInfo& operator=(const VertexAttributeInfo& other)     = default;
    VertexAttributeInfo(VertexAttributeInfo&& other) noexcept            = default;
    VertexAttributeInfo& operator=(VertexAttributeInfo&& other) noexcept = default;

    ~VertexAttributeInfo() = default;
};
#endif