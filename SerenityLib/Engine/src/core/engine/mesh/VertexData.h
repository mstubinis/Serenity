#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include <core/engine/mesh/MeshIncludes.h>
#include <memory>
#include <string>

struct VertexData final{
    VertexDataFormat&                              m_Format;
    std::vector<std::vector<uint8_t>>              m_Data;
    std::vector<size_t>                            m_DataSizes;
    std::vector<size_t>                            m_DataSizesCapacity;
    std::vector<unsigned int>                      m_Indices;
    std::vector<Engine::priv::Triangle>            m_Triangles;
    GLuint                                         m_VAO = 0;
    std::vector<std::unique_ptr<BufferObject>>     m_Buffers;

    VertexData()                                       = delete;
    VertexData(const VertexDataFormat& _format);
    VertexData(const VertexData& other)                = delete;
    VertexData& operator=(const VertexData& other)     = delete;
    VertexData(VertexData&& other) noexcept            = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    ~VertexData();

    template<typename T> const std::vector<T> getData(const size_t& attributeIndex) const {
        auto* buffer = (m_Data[attributeIndex].data());
        const T* data_as_t_ptr = reinterpret_cast<const T*>(buffer);
        const std::vector<T> data_as_t(data_as_t_ptr, data_as_t_ptr + m_DataSizes[attributeIndex]);
        return data_as_t;
    }
    template<typename T> void setData(const size_t& attributeIndex, const std::vector<T>& source_new_data, const bool addToGPU = false, const bool orphan = false) {
        if (m_Buffers.size() == 0)
            m_Buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= m_Data.size())
            return;

        auto& destination_data = m_Data[attributeIndex];
        const auto sizeofT     = sizeof(T);
        const auto totalSize   = (source_new_data.size() * sizeofT);
        destination_data.clear();
        m_DataSizesCapacity[attributeIndex] = source_new_data.size();
        destination_data.reserve(totalSize);
        auto* raw_src_data_uchar = reinterpret_cast<uint8_t*>(const_cast<T*>(source_new_data.data()));
        std::copy(raw_src_data_uchar, raw_src_data_uchar + totalSize, std::back_inserter(destination_data));
        m_DataSizes[attributeIndex] = source_new_data.size();
        if (addToGPU) {
            if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan, -1);
            }else{
                sendDataToGPU(orphan, static_cast<int>(attributeIndex));
            }
        }
    }
    void setIndices(std::vector<unsigned int>& data, const bool addToGPU = false, const bool orphan = false, const bool recalcTriangles = false);

    void clearData();

    void finalize();
    void bind() const;
    void unbind() const;
    void sendDataToGPU(const bool orphan, const int attributeIndex = -1);
};

#endif