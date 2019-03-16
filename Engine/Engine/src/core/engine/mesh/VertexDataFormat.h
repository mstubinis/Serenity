#pragma once
#ifndef ENGINE_VERTEX_DATA_FORMAT_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_FORMAT_H_INCLUDE_GUARD

#include <vector>
#include <core/engine/mesh/VertexAttribute.h>

struct VertexAttributeLayout final {enum Type {
    Interleaved,
    NonInterleaved,
};};
struct VertexData;
struct VertexDataFormat {
    std::vector<VertexAttributeInfo>    attributes;
    VertexAttributeLayout::Type         interleavingType;

    VertexDataFormat();
    ~VertexDataFormat()                                            = default;
    VertexDataFormat(const VertexDataFormat& other)                = delete;
    VertexDataFormat& operator=(const VertexDataFormat& other)     = delete;
    VertexDataFormat(VertexDataFormat&& other) noexcept            = default;
    VertexDataFormat& operator=(VertexDataFormat&& other) noexcept = default;

    void add(int size,int type,bool normalized,int stride,size_t offset,size_t typeSize);
    void bind(VertexData& vertData);
    void unbind();

    static VertexDataFormat   VertexDataPositionsOnly, VertexDataNoLighting, VertexDataBasic, VertexDataAnimated;
};

#endif