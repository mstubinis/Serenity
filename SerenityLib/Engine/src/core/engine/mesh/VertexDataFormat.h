#pragma once
#ifndef ENGINE_VERTEX_DATA_FORMAT_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_FORMAT_H_INCLUDE_GUARD

#include <vector>
#include <core/engine/mesh/VertexAttribute.h>

struct VertexAttributeLayout final {enum Type {
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
};};
struct VertexData;

//information about a vertex structure ( a list of all its attributes and the way the attributes are weaved into memory)
struct VertexDataFormat {
    std::vector<VertexAttributeInfo>    attributes;
    VertexAttributeLayout::Type         interleavingType;

    VertexDataFormat();
    ~VertexDataFormat()                                            = default;
    VertexDataFormat(const VertexDataFormat& other)                = delete;
    VertexDataFormat& operator=(const VertexDataFormat& other)     = delete;
    VertexDataFormat(VertexDataFormat&& other) noexcept            = default;
    VertexDataFormat& operator=(VertexDataFormat&& other) noexcept = default;

    void add(const int size, const int type, const bool normalized, const int stride, const size_t offset, const size_t typeSize);
    void bind(const VertexData& vertData);
    void unbind();

    static VertexDataFormat   VertexDataPositionsOnly, VertexDataNoLighting, VertexDataBasic, VertexDataAnimated;
};

#endif