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
        : typeSize{ typeSize_ }
        , offset{ offset_ }
        , numComponents{ numComponents_ }
        , type{ type_ }
        , stride{ stride_ }
        , normalized{ normalized_ }
    {}
};

//information about a vertex structure ( a list of all its attributes and the way the attributes are weaved into memory)
class VertexDataFormat {
    private:
        std::vector<VertexAttributeInfo>  m_Attributes;
        VertexAttributeLayout             m_InterleavingType = VertexAttributeLayout::Interleaved;
    public:
        VertexDataFormat() = default;
        VertexDataFormat(VertexAttributeLayout layout)
            : m_InterleavingType{ layout }
        {}

        void add(int numComponents, int type, bool normalized, int stride, size_t offset, size_t typeSize);
        void bind(const VertexData&) const noexcept;
        void unbind() const noexcept;

        [[nodiscard]] inline const std::vector<VertexAttributeInfo>& getAttributes() const noexcept { return m_Attributes; }
        [[nodiscard]] inline VertexAttributeLayout getLayoutType() const noexcept { return m_InterleavingType; }
    public:
        static VertexDataFormat   VertexDataPositionsOnly, 
                                  VertexData2D,
                                  VertexDataNoLighting,
                                  VertexDataBasic,
                                  VertexDataAnimated;
};

#endif