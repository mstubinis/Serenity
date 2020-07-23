#pragma once
#ifndef ENGINE_VERTEX_DATA_H
#define ENGINE_VERTEX_DATA_H

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include <core/engine/mesh/MeshIncludes.h>

struct VertexData final{
    VertexDataFormat                               m_Format;
    GLuint                                         m_VAO = 0;
    std::vector<std::vector<uint8_t>>              m_Data;
    std::vector<size_t>                            m_DataSizes;
    std::vector<unsigned int>                      m_Indices;
    std::vector<Engine::priv::Triangle>            m_Triangles;
    std::vector<std::unique_ptr<BufferObject>>     m_Buffers;

    VertexData()                                       = delete;
    VertexData(VertexDataFormat& format);

    VertexData(const VertexData& other)                = delete;
    VertexData& operator=(const VertexData& other)     = delete;
    VertexData(VertexData&& other) noexcept            = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    ~VertexData();

    template<typename T> std::vector<T> getData(size_t attributeIndex) const noexcept {
        if (attributeIndex >= m_Data.size()) {
            return std::vector<T>{};
        }
        auto* buffer           = (m_Data[attributeIndex].data());
        const T* data_as_t_ptr = reinterpret_cast<const T*>(buffer);
        const std::vector<T> data_as_t(data_as_t_ptr, data_as_t_ptr + m_DataSizes[attributeIndex]);
        return data_as_t;
    }
    template<typename T> void setData(size_t attributeIndex, const T* source_new_data, size_t bufferCount, bool addToGPU = false, bool orphan = false) noexcept {
        if (m_Buffers.size() == 0)
            m_Buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= m_Data.size())
            return;
        auto& destination_data = m_Data[attributeIndex];
        const auto totalSize   = (bufferCount * sizeof(T));
        destination_data.clear();
        destination_data.reserve(totalSize);
        const uint8_t* raw_src_data_uchar = reinterpret_cast<const uint8_t*>(source_new_data);
        std::copy(raw_src_data_uchar, raw_src_data_uchar + totalSize, std::back_inserter(destination_data));
        m_DataSizes[attributeIndex] = bufferCount;
        if (addToGPU) {
            if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan, -1);
            }else{
                sendDataToGPU(orphan, static_cast<int>(attributeIndex));
            }
        }
    }

    std::vector<glm::vec3> getPositions() const;

    void setData(size_t attributeIndex, uint8_t* buffer, size_t source_new_data_amount, size_t vertexCount, bool addToGPU = false, bool orphan = false) noexcept {
        if (m_Buffers.size() == 0)
            m_Buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= m_Data.size())
            return;
        auto& destination_data = m_Data[attributeIndex];
        destination_data.clear();
        destination_data.reserve(source_new_data_amount);
        std::copy(buffer, buffer + source_new_data_amount, std::back_inserter(destination_data));
        m_DataSizes[attributeIndex] = vertexCount;
        if (addToGPU) {
            if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan, -1);
            }else{
                sendDataToGPU(orphan, static_cast<int>(attributeIndex));
            }
        }
    }

    void setIndices(const unsigned int* data, size_t bufferCount, bool addToGPU = false, bool orphan = false, bool recalcTriangles = false);

    void clearData();

    void finalize();
    void bind() const;
    void unbind() const;
    void sendDataToGPU(bool orphan, int attributeIndex = -1);
};

#endif