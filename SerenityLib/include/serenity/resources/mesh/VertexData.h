#pragma once
#ifndef ENGINE_VERTEX_DATA_H
#define ENGINE_VERTEX_DATA_H

struct VertexData;

#include <serenity/resources/mesh/VertexBufferObject.h>
#include <serenity/resources/mesh/VertexDataFormat.h>
#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <serenity/system/TypeDefs.h>
#include <vector>
#include <memory>
#include <iterator>

constexpr uint32_t MESH_DEFAULT_MODIFICATION_FLAGS = MeshModifyFlags::None | MeshModifyFlags::UploadToGPU;

class VertexAttrDataBuffer final {
private:
    std::vector<uint8_t>  m_Buffer;
    size_t                m_Size = 0; //size NOT IN BYTES
public:
    VertexAttrDataBuffer() = default;
    VertexAttrDataBuffer(const VertexAttrDataBuffer&) = delete;
    VertexAttrDataBuffer& operator=(const VertexAttrDataBuffer&) = delete;
    VertexAttrDataBuffer(VertexAttrDataBuffer&&) noexcept = default;
    VertexAttrDataBuffer& operator=(VertexAttrDataBuffer&&) noexcept = default;

    [[nodiscard]] inline size_t getSize() const noexcept { return m_Size; }
    [[nodiscard]] inline const std::vector<uint8_t>& getBuffer() const noexcept { return m_Buffer; }
    [[nodiscard]] inline const uint8_t* getBufferData() const noexcept { return m_Buffer.data(); }

    inline void clear() noexcept { 
        m_Buffer.clear(); 
        m_Size = 0; 
    }

    void setData(size_t vertexCount, size_t bufferSizeInBytes, const uint8_t* inData) {
        m_Buffer.clear();
        m_Buffer.reserve(bufferSizeInBytes);
        std::copy(inData, inData + bufferSizeInBytes, std::back_inserter(m_Buffer));
        m_Size = vertexCount;
    }
};

struct VertexData final {
    VertexDataFormat                     m_Format;
    std::vector<VertexAttrDataBuffer>    m_Data;
    std::vector<uint32_t>                m_Indices;
    std::vector<Engine::priv::Triangle>  m_Triangles;
    std::vector<BufferObject>            m_Buffers;
    GLuint                               m_VAO = 0;

    VertexData() = delete;
    VertexData(const VertexDataFormat& format);

    VertexData(const VertexData&)            = delete;
    VertexData& operator=(const VertexData&) = delete;
    VertexData(VertexData&&) noexcept;
    VertexData& operator=(VertexData&&) noexcept;

    template<typename T> 
    std::vector<T> getData(size_t attributeIndex) const noexcept {
        if (attributeIndex >= m_Data.size()) {
            return {};
        }
        const T* data_ptr = reinterpret_cast<const T*>(m_Data[attributeIndex].getBufferData());
        return { data_ptr, data_ptr + m_Data[attributeIndex].getSize()};
    }
    template<typename T> 
    void setData(size_t attributeIndex, const T* source_new_data, size_t bufferCount, MeshModifyFlags::Flag flags = (MeshModifyFlags::Flag)MESH_DEFAULT_MODIFICATION_FLAGS) noexcept {
        if (m_Buffers.size() == 0) {
            m_Buffers.emplace_back(BufferDataType::VertexArray);
        }
        if (attributeIndex >= m_Data.size()) {
            return;
        }
        const uint8_t* buffer = reinterpret_cast<const uint8_t*>(source_new_data);
        m_Data[attributeIndex].setData(bufferCount, bufferCount * sizeof(T), buffer);
        if (flags & MeshModifyFlags::UploadToGPU) {
            sendDataToGPU(flags & MeshModifyFlags::Orphan, m_Format.getLayoutType() == VertexAttributeLayout::Interleaved ? -1 : (int)attributeIndex);
        }
    }
    template<typename CONTAINER>
    void setData(size_t attributeIndex, const CONTAINER& container, MeshModifyFlags::Flag flags = (MeshModifyFlags::Flag)MESH_DEFAULT_MODIFICATION_FLAGS) noexcept {
        setData(attributeIndex, container.data(), container.size(), flags);
    }

    std::vector<glm::vec3> getPositions() const;

    void setData(size_t attributeIndex, const uint8_t* buffer, size_t bufferSizeInBytes, size_t vertexCount, MeshModifyFlags::Flag flags = (MeshModifyFlags::Flag)MESH_DEFAULT_MODIFICATION_FLAGS) noexcept {
        if (m_Buffers.size() == 0) {
            m_Buffers.emplace_back(BufferDataType::VertexArray);
        }
        if (attributeIndex >= m_Data.size()) {
            return;
        }
        m_Data[attributeIndex].setData(vertexCount, bufferSizeInBytes, buffer);
        if (flags & MeshModifyFlags::UploadToGPU) {
            sendDataToGPU(flags & MeshModifyFlags::Orphan, m_Format.getLayoutType() == VertexAttributeLayout::Interleaved ? -1 : (int)attributeIndex);
        }
    }

    void setIndices(const uint32_t* data, size_t bufferCount, MeshModifyFlags::Flag flags);
    void setIndices(const std::vector<uint32_t>& indices, MeshModifyFlags::Flag flags);

    void clearData();

    void finalize();
    void bind() const;
    void unbind() const;
    void sendDataToGPU(bool orphan, int attributeIndex = -1);
};

#endif