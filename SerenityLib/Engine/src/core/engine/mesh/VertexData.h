#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include <core/engine/mesh/MeshIncludes.h>
#include <memory>
#include <string>

struct VertexData final{
    VertexDataFormat&                              format;
    std::vector<std::vector<uint8_t>>              data;
    std::vector<size_t>                            dataSizes;
    std::vector<size_t>                            dataSizesCapacity;
    std::vector<unsigned short>                    indices;
    std::vector<Engine::priv::Triangle>            triangles;
    GLuint                                         vao = 0;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData()                                       = delete;
    VertexData(const VertexDataFormat& _format);
    VertexData(const VertexData& other)                = delete;
    VertexData& operator=(const VertexData& other)     = delete;
    VertexData(VertexData&& other) noexcept            = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    ~VertexData();

    template<typename T> const std::vector<T> getData(const size_t& attributeIndex) {
        auto* buffer = (data[attributeIndex].data());
        const T* data_as_t_ptr = reinterpret_cast<T*>((buffer));
        const std::vector<T> data_as_t(data_as_t_ptr, data_as_t_ptr + dataSizes[attributeIndex]);
        return data_as_t;
    }
    template<typename T> void setData(const size_t& attributeIndex, const std::vector<T>& source_new_data, const bool addToGPU = false, const bool orphan = false) {
        if (buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= data.size())
            return;

        auto& destination_data = data[attributeIndex];
        const auto sizeofT     = sizeof(T);
        const auto totalSize   = (source_new_data.size() * sizeofT);
        destination_data.clear();
        dataSizesCapacity[attributeIndex] = source_new_data.size();
        destination_data.reserve(totalSize);
        auto* raw_src_data_uchar = reinterpret_cast<uint8_t*>(const_cast<T*>(source_new_data.data()));
        std::copy(raw_src_data_uchar, raw_src_data_uchar + totalSize, std::back_inserter(destination_data));
        dataSizes[attributeIndex] = source_new_data.size();
        if (addToGPU) {
            if (format.interleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan, -1);
            }else{
                sendDataToGPU(orphan, static_cast<int>(attributeIndex));
            }
        }
    }
    void setIndices(std::vector<unsigned short>& _data, const bool addToGPU = false, const bool orphan = false, const bool reCalcTriangles = false);

    void finalize();
    void bind() const;
    void unbind() const;
    void sendDataToGPU(const bool orphan, const int attributeIndex = -1);
};

#endif