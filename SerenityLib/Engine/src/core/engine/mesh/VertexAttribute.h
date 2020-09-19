#pragma once
#ifndef ENGINE_VERTEX_ATTRIBUTE_H
#define ENGINE_VERTEX_ATTRIBUTE_H

//information about individual attributes (example: position, uv, normals, etc)
struct VertexAttributeInfo final{
    int      size       = 0;
    int      type       = 0;
    size_t   typeSize   = 0;
    size_t   offset     = 0;
    int      stride     = 0;
    bool     normalized = false;

    VertexAttributeInfo() = delete;
    VertexAttributeInfo(int size_, int type_, bool normalized_, int stride_, size_t offset_, size_t typeSize_)
        :size(size_),
        type(type_),
        typeSize(typeSize_),
        offset(offset_),
        stride(stride_),
        normalized(normalized_)
    {}
};
#endif