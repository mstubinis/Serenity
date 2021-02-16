#pragma once
#ifndef ENGINE_VERTEX_DATA_H
#define ENGINE_VERTEX_DATA_H

struct VertexData;

#include <serenity/resources/mesh/VertexBufferObject.h>
#include <serenity/resources/mesh/VertexDataFormat.h>
#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/system/TypeDefs.h>
#include <vector>
#include <memory>
#include <iterator>

constexpr uint32_t MESH_DEFAULT_MODIFICATION_FLAGS = MeshModifyFlags::None | MeshModifyFlags::UploadToGPU;

struct VertexAttrDataBuffer final {
    std::vector<uint8_t>  m_Buffer;
    size_t                m_Size = 0;

    VertexAttrDataBuffer() = default;
    VertexAttrDataBuffer(const VertexAttrDataBuffer&)                = delete;
    VertexAttrDataBuffer& operator=(const VertexAttrDataBuffer&)     = delete;
    VertexAttrDataBuffer(VertexAttrDataBuffer&&) noexcept            = default;
    VertexAttrDataBuffer& operator=(VertexAttrDataBuffer&&) noexcept = default;

    inline void clear() noexcept { 
        m_Buffer.clear(); 
        m_Size = 0; 
    }
};

struct VertexData final {
    VertexDataFormat                     m_Format;
    std::vector<VertexAttrDataBuffer>    m_Data;
    std::vector<uint32_t>                m_Indices;
    std::vector<Engine::priv::Triangle>  m_Triangles;
    std::vector<BufferObject>            m_Buffers;
    GLuint                               m_VAO        = 0;

    VertexData() = delete;
    VertexData(VertexDataFormat& format);

    VertexData(const VertexData&) = delete;
    VertexData& operator=(const VertexData&) = delete;
    VertexData(VertexData&&) noexcept;
    VertexData& operator=(VertexData&&) noexcept;
    ~VertexData();

    template<typename T> 
    std::vector<T> getData(size_t attributeIndex) const noexcept {
        if (attributeIndex >= m_Data.size()) {
            return {};
        }
        const T* data_ptr = reinterpret_cast<const T*>(m_Data[attributeIndex].m_Buffer.data());
        return { data_ptr, data_ptr + m_Data[attributeIndex].m_Size };
    }
    template<typename T> 
    void setData(size_t attributeIndex, const T* source_new_data, size_t bufferCount, MeshModifyFlags::Flag flags = (MeshModifyFlags::Flag)MESH_DEFAULT_MODIFICATION_FLAGS) noexcept {
        if (m_Buffers.size() == 0) {
            m_Buffers.emplace_back(BufferDataType::VertexArray);
        }
        if (attributeIndex >= m_Data.size()) {
            return;
        }
        auto& destination_data = m_Data[attributeIndex].m_Buffer;
        const auto totalSize   = (bufferCount * sizeof(T));
        destination_data.clear();
        destination_data.reserve(totalSize);
        const uint8_t* raw_src_data_uchar = reinterpret_cast<const uint8_t*>(source_new_data);
        std::copy(raw_src_data_uchar, raw_src_data_uchar + totalSize, std::back_inserter(destination_data));
        m_Data[attributeIndex].m_Size = bufferCount;
        if (flags & MeshModifyFlags::UploadToGPU) {
            if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(flags & MeshModifyFlags::Orphan, -1);
            }else{
                sendDataToGPU(flags & MeshModifyFlags::Orphan, (int)attributeIndex);
            }
        }
    }

    std::vector<glm::vec3> getPositions() const;

    void setData(size_t attributeIndex, uint8_t* buffer, size_t source_new_data_amount, size_t vertexCount, MeshModifyFlags::Flag flags = (MeshModifyFlags::Flag)MESH_DEFAULT_MODIFICATION_FLAGS) noexcept {
        if (m_Buffers.size() == 0) {
            m_Buffers.emplace_back(BufferDataType::VertexArray);
        }
        if (attributeIndex >= m_Data.size()) {
            return;
        }
        auto& destination_data = m_Data[attributeIndex].m_Buffer;
        destination_data.clear();
        destination_data.reserve(source_new_data_amount);
        std::copy(buffer, buffer + source_new_data_amount, std::back_inserter(destination_data));
        m_Data[attributeIndex].m_Size = vertexCount;
        if (flags & MeshModifyFlags::UploadToGPU) {
            if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(flags & MeshModifyFlags::Orphan, -1);
            }else{
                sendDataToGPU(flags & MeshModifyFlags::Orphan, (int)attributeIndex);
            }
        }
    }

    void setIndices(const uint32_t* data, size_t bufferCount, MeshModifyFlags::Flag flags);

    void clearData();

    void finalize();
    void bind() const;
    void unbind() const;
    void sendDataToGPU(bool orphan, int attributeIndex = -1);
};

#endif