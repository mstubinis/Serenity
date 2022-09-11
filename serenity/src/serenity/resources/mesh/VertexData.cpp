
#include <serenity/resources/mesh/VertexData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>

#pragma region VertexAttrDataBuffer

void VertexAttrDataBuffer::clear() noexcept {
    m_Buffer.clear();
    m_Size = 0;
}
void VertexAttrDataBuffer::setData(size_t vertexCount, size_t bufferSizeInBytes, const uint8_t* inData) {
    m_Buffer.clear();
    m_Buffer.reserve(bufferSizeInBytes);
    std::copy(inData, inData + bufferSizeInBytes, std::back_inserter(m_Buffer));
    m_Size = vertexCount;
}

#pragma endregion

#pragma region VertexData

VertexData::VertexData(const VertexDataFormat& format)
    : m_Format{ format }
{
    m_Data.resize(format.getAttributes().size());
}

VertexData::VertexData(VertexData&& other) noexcept
    : m_Format    { std::move(other.m_Format) }
    , m_Data      { std::move(other.m_Data) }
    , m_Indices   { std::move(other.m_Indices) }
    , m_Triangles { std::move(other.m_Triangles) }
    , m_VBO       { std::move(other.m_VBO) }
    , m_IBO       { std::move(other.m_IBO) }
    , m_VAO       { std::move(other.m_VAO)  }
{}
VertexData& VertexData::operator=(VertexData&& other) noexcept {
    if (this != &other) {
        m_Format    = std::move(other.m_Format);
        m_Data      = std::move(other.m_Data);
        m_Indices   = std::move(other.m_Indices);
        m_Triangles = std::move(other.m_Triangles);
        m_VBO       = std::move(other.m_VBO);
        m_IBO       = std::move(other.m_IBO);
        m_VAO       = std::move(other.m_VAO);
    }
    return *this;
}
void VertexData::clearData() {
    for (auto& data : m_Data) {
        data.clear();
    }
    m_Indices.clear();
    m_Triangles.clear();
}
void VertexData::finalize() {
    m_VAO.deleteVAO();
    if (Engine::priv::APIState<Engine::priv::OpenGL>::supportsVAO()) {
        //build the vao itself
        const bool success = m_VAO.generateVAO();
        m_VAO.bindVAO();
        sendDataToGPU(false, -1);
        m_Format.bind(*this);
        Engine::Renderer::bindVAO(0);
    } else {
        sendDataToGPU(false, -1);
        m_Format.bind(*this);
    }
}
void VertexData::bind() const {
    if (m_VAO) {
        m_VAO.bindVAO();
    } else {
        m_VBO.bind();
        m_IBO.bind();
        m_Format.bind(*this);
    }
}
void VertexData::unbind() const {
    if (m_VAO) {
        m_VAO.unbindVAO();
    } else {
        m_Format.unbind();
    }
}

std::vector<glm::vec3> VertexData::getPositions() const {
    std::vector<glm::vec3> points;
    if (m_Format.getAttributes()[0].type != GL_FLOAT) {
        const auto pts_half = getData<glm::highp_u16vec3>(0);
        points.reserve(pts_half.size());
        for (const auto& half : pts_half) {
            points.emplace_back(Engine::Math::ToFloat(half.x), Engine::Math::ToFloat(half.y), Engine::Math::ToFloat(half.z));
        }
    } else {
        points = getData<glm::vec3>(0);
    }
    return points;
}

void VertexData::setIndices(const uint32_t* data, size_t bufferCount, MeshModifyFlags::Flag flags) {
    if (data != m_Indices.data()) {
        m_Indices.clear();
        m_Indices.reserve(bufferCount);
        for (uint32_t i = 0; i < bufferCount; ++i) {
            m_Indices.emplace_back(data[i]);
        }
    }
    if (flags & MeshModifyFlags::RecalculateTriangles) {
        auto positions = getPositions();
        m_Triangles.clear();
        m_Triangles.reserve(bufferCount / size_t(3)); //do not process any extra indices that shouldn't be there to begin with
        Engine::priv::Triangle tri;
        const size_t numIndicesProper = m_Triangles.capacity() * 3;
        for (size_t i = 0; i < numIndicesProper; i += 3) {
            tri.setPositions( positions[m_Indices[i + 0]], positions[m_Indices[i + 1]], positions[m_Indices[i + 2]] );
            tri.setIndices( m_Indices[i + 0], m_Indices[i + 1], m_Indices[i + 2] );
            tri.recalcMidpoint();
            m_Triangles.push_back(tri);
        }
    }
    if (flags & MeshModifyFlags::UploadToGPU) {
        m_IBO.generate();
        m_IBO.bind();
        !(flags & MeshModifyFlags::Orphan) ? m_IBO.setData(m_Indices, BufferDataDrawType::Static) : m_IBO.setDataOrphan(m_Indices);
    }
}
void VertexData::setIndices(const std::vector<uint32_t>& indices, MeshModifyFlags::Flag flags) {
    setIndices(indices.data(), indices.size(), flags);
}

void VertexData::sendDataToGPU(bool orphan, int attributeIndex) {
    //Interleaved format makes use of attributeIndex = -1
    /*
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
    */
    m_VBO.generate();
    m_VBO.bind();

    size_t accumulator = 0;
    size_t size = 0;
    std::vector<uint8_t> gpu_data_buffer;
    if (m_Format.getLayoutType() == VertexAttributeLayout::Interleaved) {
        size = (m_Format.getAttributes()[0].stride * m_Data[0].getSize());
        gpu_data_buffer.reserve(size);
        for (size_t i = 0; i < m_Data[0].getSize(); ++i) {
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                const auto& sizeofT     = m_Format.getAttributes()[attribute_index].typeSize;
                auto* gpu_destination   = &(gpu_data_buffer.data())[accumulator];
                const auto at           = i * sizeofT;
                auto& src_data          = m_Data[attribute_index];
                auto* cpu_source        = &(src_data.getBufferData())[at];
                std::memcpy(gpu_destination, cpu_source, sizeofT);
                accumulator            += sizeofT;
            }
        }
        (!orphan) ? m_VBO.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : m_VBO.setDataOrphan(gpu_data_buffer.data());
    } else {
        if (attributeIndex == -1) {
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                size += m_Format.getAttributes()[attribute_index].typeSize * m_Data[attribute_index].getSize();
            }
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                const auto blockSize  = m_Data[attribute_index].getSize() * m_Format.getAttributes()[attribute_index].typeSize;
                auto* gpu_destination = &(gpu_data_buffer.data())[accumulator];
                auto& src_data        = m_Data[attribute_index];
                auto* cpu_source      = &(src_data.getBufferData())[0];
                std::memcpy(gpu_destination, cpu_source, blockSize);
                accumulator          += blockSize;
            }
            (!orphan) ? m_VBO.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : m_VBO.setDataOrphan(gpu_data_buffer.data());
        } else {
            size += (m_Format.getAttributes()[attributeIndex].typeSize * m_Data[attributeIndex].getSize());
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                if (attribute_index != attributeIndex) {
                    accumulator += m_Data[attribute_index].getSize() * m_Format.getAttributes()[attribute_index].typeSize;
                } else {
                    auto* gpu_destination     = &(gpu_data_buffer.data())[0];
                    auto& src_data            = m_Data[attribute_index];
                    auto* cpu_source          = &(src_data.getBufferData())[0];
                    std::memcpy(gpu_destination, cpu_source, size);
                    break;
                }
            }
            m_VBO.setData(size, accumulator, gpu_data_buffer.data());
        }
    }
    if (attributeIndex == -1) {
        m_IBO.generate();
        m_IBO.bind();
        m_IBO.setData(m_Indices, BufferDataDrawType::Static);
    }
}

#pragma endregion