#pragma once
#ifndef ENGINE_VERTEX_DATA_FORMAT_H
#define ENGINE_VERTEX_DATA_FORMAT_H

struct VertexData;

enum class VertexAttributeLayout : unsigned int {
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
};

#include <serenity/core/engine/mesh/VertexAttributeInfo.h>

//information about a vertex structure ( a list of all its attributes and the way the attributes are weaved into memory)
struct VertexDataFormat {
    std::vector<VertexAttributeInfo>  m_Attributes;
    VertexAttributeLayout             m_InterleavingType = VertexAttributeLayout::Interleaved;

    inline void add(int size, int type, bool normalized, int stride, size_t offset, size_t typeSize) {
        m_Attributes.emplace_back(size, type, normalized, stride, offset, typeSize);
    }
    void bind(const VertexData& vertData) const noexcept;
    void unbind() const noexcept {
        for (size_t i = 0; i < m_Attributes.size(); ++i) {
            glDisableVertexAttribArray((GLuint)i);
        }
    }

    static VertexDataFormat   VertexDataPositionsOnly, 
                              VertexDataNoLighting, 
                              VertexDataBasic, 
                              VertexDataAnimated;
};

#endif