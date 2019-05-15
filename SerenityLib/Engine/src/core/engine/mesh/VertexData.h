#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include <memory>

typedef unsigned short ushort;

//client side memory for mesh data
struct VertexData final{
    VertexDataFormat&                              format;
    std::vector<char*>                             data;
    std::vector<size_t>                            dataSizes;
    std::vector<ushort>                            indices;
    GLuint                                         vao;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData() = delete;
    VertexData(const VertexData& other) = delete;
    VertexData& operator=(const VertexData& other) = delete;
    VertexData(VertexData&& other) noexcept = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    VertexData(const VertexDataFormat& _format);
    ~VertexData();
    template<typename T> const std::vector<T> getData(size_t attributeIndex) {
        const T* _data = (T*)data[attributeIndex];
        std::vector<T> ret(_data, _data + dataSizes[attributeIndex]);
        return ret;
    }
    template<typename T> void setData(size_t attributeIndex, std::vector<T>& _data, bool addToGPU = false,bool orphan = false) {
        if (buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= data.size()) return;
        auto& _buffer = *buffers[0];
        dataSizes[attributeIndex] = _data.size();
        free(data[attributeIndex]);
        auto totalSize = (_data.size() * sizeof(T)) + 1;
        data[attributeIndex] = (char*)malloc(totalSize);
        std::memmove(data[attributeIndex], _data.data(), totalSize);
        if (addToGPU) {
            if (format.interleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan,-1);
            }else{
                sendDataToGPU(orphan,attributeIndex);
            }
        }
    }
    void setDataIndices(std::vector<ushort>& _data, bool addToGPU = false, bool orphan = false);

    void finalize();
    void bind();
    void unbind();
    void sendDataToGPU(bool orphan, int attributeIndex = -1);
};

#endif