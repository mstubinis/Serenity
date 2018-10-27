#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <vector>
#include <core/engine/mesh/VertexAttribute.h>
#include <core/engine/mesh/VertexBufferObject.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include "core/engine/Engine_Renderer.h"
#include <memory>

struct VertexDataFormat {
    std::vector<VertexAttributeInfo>  attributes;
    VertexDataFormat() = default;
    ~VertexDataFormat() = default;
    VertexDataFormat(const VertexDataFormat& other) = delete;
    VertexDataFormat& operator=(const VertexDataFormat& other) = delete;
    VertexDataFormat(VertexDataFormat&& other) noexcept = default;
    VertexDataFormat& operator=(VertexDataFormat&& other) noexcept = default;

    void add(int _size, int _type, bool _normalized, int _stride, size_t _offset) {
        attributes.emplace_back(_size, _type, _normalized, _stride, _offset);
    }
    void bind() {
        for (size_t i = 0; i < attributes.size(); ++i) {
            const auto& attribute = attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.size, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }
    void unbind() { for (size_t i = 0; i < attributes.size(); ++i) { glDisableVertexAttribArray(i); } }

    static VertexDataFormat   VertexDataBasic;
    static VertexDataFormat   VertexDataAnimated;
};

struct VertexData {
    VertexDataFormat&                              format;
    std::vector<std::vector<void*>>                data;
    std::vector<unsigned short>                    indices;
    GLuint                                         vao;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData(const VertexDataFormat& _format) :format(const_cast<VertexDataFormat&>(_format)), vao(0) {
        data.reserve(_format.attributes.size());
        for (size_t i = 0; i < data.capacity(); ++i) { data.emplace_back(); }

        buffers.push_back(std::make_unique<VertexBufferObject>());
    }
    template<typename T> const std::vector<T*>& getData(size_t attributeIndex) {
        return reinterpret_cast<std::vector<T*>>(data[attributeIndex]);
    }
    template<typename T> void setData(size_t attributeIndex, const T* _data, size_t _dataSize) {
        if(buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        auto& _buffer = *buffers[0];
        _buffer.bind();
        data[attributeIndex].assign(_data, _data + _dataSize);
        _buffer.bufferData(_dataSize * sizeof(T), _data, BufferDataType::Dynamic);
    }
    void setDataIndices(const unsigned short* _data, size_t _dataSize) {
        if (buffers.size() == 1)
            buffers.push_back(std::make_unique<ElementBufferObject>());
        auto& _buffer = *buffers[1];
        _buffer.bind();
        indices.assign(_data, _data + _dataSize);
        _buffer.bufferData(_dataSize * sizeof(unsigned short), _data, BufferDataType::Static);
    }

    void finalize() {
        //build the vao itself
        Engine::Renderer::deleteVAO(vao);
        if (Engine::epriv::RenderManager::OPENGL_VERSION >= 30) {
            Engine::Renderer::genAndBindVAO(vao);
            for (auto& buffer : buffers) buffer->bind();
            format.bind();
            Engine::Renderer::bindVAO(0);
        }
    }

    ~VertexData() {
        Engine::Renderer::deleteVAO(vao);
        for (auto& vec : data) {
            SAFE_DELETE_VECTOR(vec);
            vector_clear(data);
        }
    }

    void bind() {
        if (vao) {
            Engine::Renderer::bindVAO(vao);
        }else{
            for (auto& buffer : buffers) buffer->bind();
            format.bind();
        }
    }
    void unbind() {
        if (vao) {
            Engine::Renderer::bindVAO(0);
        }else{
            format.unbind();
            //for (auto& buffer : buffers) buffer->unbind();
        }
    }

    VertexData(const VertexData& other) = delete;
    VertexData& operator=(const VertexData& other) = delete;
    VertexData(VertexData&& other) noexcept = default;
    VertexData& operator=(VertexData&& other) noexcept = default;
};

#endif