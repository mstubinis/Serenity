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

typedef unsigned short ushort;

struct VertexDataFormat {
    std::vector<VertexAttributeInfo>  attributes;
    VertexDataFormat() = default;
    ~VertexDataFormat() = default;
    VertexDataFormat(const VertexDataFormat& other) = delete;
    VertexDataFormat& operator=(const VertexDataFormat& other) = delete;
    VertexDataFormat(VertexDataFormat&& other) noexcept = default;
    VertexDataFormat& operator=(VertexDataFormat&& other) noexcept = default;

    inline void add(int _size, int _type, bool _normalized, int _stride, size_t _offset, size_t _typeSize) {
        attributes.emplace_back(_size, _type, _normalized, _stride, _offset, _typeSize);
    }
    inline void bind() {
        for (size_t i = 0; i < attributes.size(); ++i) {
            const auto& attribute = attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.size, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }
    inline void unbind() { for (size_t i = 0; i < attributes.size(); ++i) { glDisableVertexAttribArray(i); } }

    static VertexDataFormat   VertexDataBasic;
    static VertexDataFormat   VertexDataAnimated;
};

struct VertexData {
    VertexDataFormat&                              format;
    std::vector<void*>                             data;
    std::vector<size_t>                            dataSizes;
    std::vector<ushort>                            indices;
    GLuint                                         vao;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData(const VertexDataFormat& _format) :format(const_cast<VertexDataFormat&>(_format)), vao(0) {
        data.reserve(_format.attributes.size());
        for (size_t i = 0; i < data.capacity(); ++i) { data.emplace_back(nullptr); }
        dataSizes.reserve(_format.attributes.size());
        for (size_t i = 0; i < dataSizes.capacity(); ++i) { dataSizes.emplace_back(0); }

        buffers.push_back(std::make_unique<VertexBufferObject>());
    }
    template<typename T> const std::vector<T> getData(size_t attributeIndex) {
        const T* _data = (T*)data[attributeIndex];
        std::vector<T> ret(_data, _data + dataSizes[attributeIndex]);
        return ret;
    }
    template<typename T> void setData(size_t attributeIndex, std::vector<T>& _data, bool addToGPU = false) {
        if (buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        auto& _buffer = *buffers[0];
        dataSizes[attributeIndex] = _data.size();
        free(data[attributeIndex]);
        auto totalSize = _data.size() * sizeof(T);
        data[attributeIndex] = malloc(totalSize);
        memcpy(data[attributeIndex], _data.data(), totalSize);
        if (addToGPU) {
            sendDataToGPU();
        }
    }
    void setDataIndices(std::vector<ushort>& _data, bool addToGPU = false) {
        if (buffers.size() == 1)
            buffers.push_back(std::make_unique<ElementBufferObject>());
        auto& _buffer = *buffers[1];
        indices.swap(_data);
        if (addToGPU) {
            _buffer.generate();
            _buffer.bind();
            _buffer.bufferData(indices.size() * sizeof(ushort), indices.data(), BufferDataType::Static);
        }
    }

    void finalize() {   
        Engine::Renderer::deleteVAO(vao);
        if (Engine::epriv::RenderManager::OPENGL_VERSION >= 30) {
            //build the vao itself
            Engine::Renderer::genAndBindVAO(vao);
            sendDataToGPU();
            format.bind();
            Engine::Renderer::bindVAO(0);
        }else{
            sendDataToGPU();
        }
    }

    ~VertexData() {
        Engine::Renderer::deleteVAO(vao);
        for (size_t i = 0; i < data.size(); ++i) {
            free(data[i]);
        }
        vector_clear(data);
        vector_clear(dataSizes);
        vector_clear(indices);
    }

    inline void bind() {
        if (vao) {
            Engine::Renderer::bindVAO(vao);
        }else{
            for (auto& buffer : buffers) buffer->bind();
            format.bind();
        }
    }
    inline void unbind() {
        if (vao) {
            Engine::Renderer::bindVAO(0);
        }else{
            format.unbind();
        }
    }
    void sendDataToGPU() {
        auto& _vBuffer = *buffers[0];
        _vBuffer.generate(); _vBuffer.bind();

        auto vertexSize = format.attributes[0].stride;
        char* buffer = (char*)malloc(vertexSize * dataSizes[0]);
        size_t accumulator = 0;
        for (size_t i = 0; i < dataSizes[0]; ++i) {
            for (size_t j = 0; j < data.size(); ++j) {
                const auto& sizeofT = format.attributes[j].typeSize;
                const auto& dataSize = dataSizes[j];
                char* _data = (char*)data[j];
                memcpy(&buffer[accumulator], &_data[i * sizeofT], sizeofT);
                accumulator += sizeofT;
            }
        }
        _vBuffer.bufferData(vertexSize * dataSizes[0], (void*)buffer, BufferDataType::Dynamic);
        free(buffer);

        auto& _iBuffer = *buffers[1];
        _iBuffer.generate();  _iBuffer.bind();
        _iBuffer.bufferData(indices.size() * sizeof(ushort), indices.data(), BufferDataType::Static);
    }

    VertexData(const VertexData& other) = delete;
    VertexData& operator=(const VertexData& other) = delete;
    VertexData(VertexData&& other) noexcept = default;
    VertexData& operator=(VertexData&& other) noexcept = default;
};




#endif