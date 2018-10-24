#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <vector>
#include <core/engine/mesh/VertexAttribute.h>
#include <GL/glew.h>
#include <GL/GL.h>

struct VertexData {
    std::vector<VertexAttributeInfo>  attributes;

    VertexData() = default;
    ~VertexData() = default;
    VertexData(const VertexData& other) = delete;
    VertexData& operator=(const VertexData& other) = delete;
    VertexData(VertexData&& other) noexcept = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    void add(int _size, int _type, bool _normalized, int _stride, size_t _offset) {
        attributes.emplace_back(_size, _type, _normalized, _stride, _offset);
    }
    void bind() {
        for (unsigned int i = 0; i < attributes.size(); ++i) {
            const auto& attribute = attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.size, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }
    void unbind() {
        for (unsigned int i = 0; i < attributes.size(); ++i) {
            glDisableVertexAttribArray(i);
        }
    }

    static VertexData VertexDataBasic;
    static VertexData VertexDataAnimated;
};


#endif