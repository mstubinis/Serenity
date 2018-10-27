#pragma once
#ifndef ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD
#define ENGINE_VERTEX_ATTRIBUTE_H_INCLUDE_GUARD

struct VertexAttributeInfo final{
    int      size;
    int      type;
    size_t   typeSize;
    bool     normalized;
    
    //this information is important for interleaved vs non-interleaved
    int      stride; //Specifies the byte offset between consecutive generic vertex attributes. If stride is 0, the generic vertex attributes are understood to be tightly packed in the array. The initial value is 0
    size_t   offset; //Specifies a offset of the first component of the first generic vertex attribute in the array in the data store of the buffer currently bound to the GL_ARRAY_BUFFER target. The initial value is 0

    VertexAttributeInfo(int _size, int _type, bool _normalized, int _stride, size_t _offset, size_t _typeSize) {
        size = _size; type = _type; normalized = _normalized; stride = _stride; offset = _offset; typeSize = _typeSize;
    }
    ~VertexAttributeInfo() = default;
    VertexAttributeInfo(const VertexAttributeInfo& other) = delete;
    VertexAttributeInfo& operator=(const VertexAttributeInfo& other) = delete;
    VertexAttributeInfo(VertexAttributeInfo&& other) noexcept = default;
    VertexAttributeInfo& operator=(VertexAttributeInfo&& other) noexcept = default;
};

#endif