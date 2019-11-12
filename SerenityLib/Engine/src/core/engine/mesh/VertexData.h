#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include <core/engine/mesh/MeshIncludes.h>
#include <memory>

struct VertexData final{
    VertexDataFormat&                              format;
    std::vector<char*>                             data;
    std::vector<size_t>                            dataSizes;
    std::vector<size_t>                            dataSizesCapacity;
    std::vector<unsigned short>                    indices;
    std::vector<Engine::epriv::Triangle>           triangles;
    GLuint                                         vao;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData()                                       = delete;
    VertexData(const VertexDataFormat& _format);
    VertexData(const VertexData& other)                = delete;
    VertexData& operator=(const VertexData& other)     = delete;
    VertexData(VertexData&& other) noexcept            = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    ~VertexData();

    template<typename T> const std::vector<T> getData(const size_t& attributeIndex) {
        const T* _data = (T*)data[attributeIndex];
        const std::vector<T> ret(_data, _data + dataSizes[attributeIndex]);
        return ret;
    }
    template<typename T> void setData(const size_t& attributeIndex, const std::vector<T>& new_data, const bool addToGPU = false, const bool orphan = false) {
        if (buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= data.size())
            return;

        auto& old_data = data[attributeIndex];
        const auto& sizeofT = sizeof(T);
        const auto& totalSize = (new_data.size() * sizeofT);
        if (dataSizesCapacity[attributeIndex] < new_data.size()) {
            delete[] old_data;
            old_data = new char[totalSize];
            dataSizesCapacity[attributeIndex] = new_data.size();
        }
        std::memcpy(old_data, new_data.data(), totalSize);
        dataSizes[attributeIndex] = new_data.size();
        if (addToGPU) {
            if (format.interleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan,-1);
            }else{
                sendDataToGPU(orphan,static_cast<int>(attributeIndex));
            }
        }
    }
    void setIndices(std::vector<unsigned short>& _data, const bool addToGPU = false, const bool orphan = false, const bool reCalcTriangles = false);

    void finalize();
    void bind();
    void unbind();
    void sendDataToGPU(const bool orphan, const int attributeIndex = -1);
};

#endif