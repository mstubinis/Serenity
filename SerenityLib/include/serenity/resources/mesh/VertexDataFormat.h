#pragma once
#ifndef ENGINE_VERTEX_DATA_FORMAT_H
#define ENGINE_VERTEX_DATA_FORMAT_H

struct VertexData;

enum class VertexAttributeLayout : unsigned int {
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
};

//information about individual attributes (example: position, uv, normals, etc)
struct VertexAttributeInfo final {
    size_t   typeSize      = 0; //not used for Interleaved layout
    size_t   offset        = 0;
    int      numComponents = 0;
    int      type          = 0;
    int      stride        = 0;
    bool     normalized    = false;

    VertexAttributeInfo() = default;
    VertexAttributeInfo(int numComponents_, int type_, bool normalized_, int stride_, size_t offset_, size_t typeSize_)
        : numComponents{ numComponents_ }
        , type{ type_ }
        , typeSize{ typeSize_ }
        , offset{ offset_ }
        , stride{ stride_ }
        , normalized{ normalized_ }
    {}
    VertexAttributeInfo(const VertexAttributeInfo&)                = default;
    VertexAttributeInfo& operator=(const VertexAttributeInfo&)     = default;
    VertexAttributeInfo(VertexAttributeInfo&&) noexcept            = default;
    VertexAttributeInfo& operator=(VertexAttributeInfo&&) noexcept = default;
};

//information about a vertex structure ( a list of all its attributes and the way the attributes are weaved into memory)
struct VertexDataFormat {
    std::vector<VertexAttributeInfo>  m_Attributes;
    VertexAttributeLayout             m_InterleavingType = VertexAttributeLayout::Interleaved;

    inline void add(int numComponents, int type, bool normalized, int stride, size_t offset, size_t typeSize) {
        m_Attributes.emplace_back(numComponents, type, normalized, stride, offset, typeSize);
    }
    void bind(const VertexData& vertData) const noexcept;
    void unbind() const noexcept;

    static VertexDataFormat   VertexDataPositionsOnly, 
                              VertexDataNoLighting, 
                              VertexDataBasic, 
                              VertexDataAnimated;
};

#endif