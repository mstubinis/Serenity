
#include <serenity/resources/mesh/VertexData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/renderer/Renderer.h>

VertexData::VertexData(const VertexDataFormat& format)
    : m_Format{ format }
{
    m_Data.resize(format.getAttributes().size());
    m_Buffers.emplace_back(BufferDataType::VertexArray);
}

VertexData::VertexData(VertexData&& other) noexcept
    : m_Format    { std::move(other.m_Format) }
    , m_Data      { std::move(other.m_Data) }
    , m_Indices   { std::move(other.m_Indices) }
    , m_Triangles { std::move(other.m_Triangles) }
    , m_Buffers   { std::move(other.m_Buffers) }
    , m_VAO       { std::exchange(other.m_VAO, 0)  }
{}
VertexData& VertexData::operator=(VertexData&& other) noexcept {
    if (this != &other) {
        m_Format    = std::move(other.m_Format);
        m_Data      = std::move(other.m_Data);
        m_Indices   = std::move(other.m_Indices);
        m_Triangles = std::move(other.m_Triangles);
        m_Buffers   = std::move(other.m_Buffers);
        m_VAO       = std::exchange(other.m_VAO, 0);
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
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::OpenGLState::constants.supportsVAO()) {
        //build the vao itself
        Engine::Renderer::genAndBindVAO(m_VAO);
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
        Engine::Renderer::bindVAO(m_VAO);
    } else {
        for (auto& buffer : m_Buffers) {
            buffer.bind();
        }
        m_Format.bind(*this);
    }
}
void VertexData::unbind() const {
    if (m_VAO) {
        Engine::Renderer::bindVAO(0);
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
    if (m_Buffers.size() == 1) {
        m_Buffers.emplace_back(BufferDataType::ElementArray);
    }
    if (data != m_Indices.data()) {
        m_Indices.clear();
        m_Indices.reserve(bufferCount);
        for (uint32_t i = 0; i < bufferCount; ++i) {
            m_Indices.emplace_back(data[i]);
        }
    }
    if (flags & MeshModifyFlags::RecalculateTriangles) {
        auto positions = getPositions();
        if (positions.size() >= 0) {
            m_Triangles.clear();
            m_Triangles.reserve(bufferCount / 3);
            size_t j = 0;
            Engine::priv::Triangle tri;
            for (size_t i = 0; i < m_Indices.size(); ++i) {
                ++j;
                const auto index  = m_Indices[i];
                if (j == 1) {
                    tri.position1 = positions[index];
                    tri.index1    = index;
                } else if (j == 2) {
                    tri.position2 = positions[index];
                    tri.index2    = index;
                } else if (j == 3) {
                    tri.position3 = positions[index];
                    tri.index3    = index;
                    tri.midpoint  = tri.position1 + tri.position2 + tri.position3;
                    tri.midpoint /= 3.0f;
                    m_Triangles.push_back(tri);
                    j = 0;
                }
            }
        }
    }
    if (flags & MeshModifyFlags::UploadToGPU) {
        auto& indiceBuffer = m_Buffers[1];
        indiceBuffer.generate();
        indiceBuffer.bind();
        !(flags & MeshModifyFlags::Orphan) ? indiceBuffer.setData(m_Indices, BufferDataDrawType::Static) : indiceBuffer.setDataOrphan(m_Indices);
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
    auto& vertexBuffer = m_Buffers[0];
    vertexBuffer.generate();
    vertexBuffer.bind();

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
        (!orphan) ? vertexBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : vertexBuffer.setDataOrphan(gpu_data_buffer.data());
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
            (!orphan) ? vertexBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : vertexBuffer.setDataOrphan(gpu_data_buffer.data());
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
            vertexBuffer.setData(size, accumulator, gpu_data_buffer.data());
        }
    }
    if (attributeIndex == -1) {
        auto& indiceBuffer = m_Buffers[1];
        indiceBuffer.generate();
        indiceBuffer.bind();
        indiceBuffer.setData(m_Indices, BufferDataDrawType::Static);
    }
}