#include <core/engine/mesh/VertexData.h>
#include <core/engine/renderer/Renderer.h>

using namespace Engine;
using namespace std;

VertexData::VertexData(const VertexDataFormat& format) : m_Format(const_cast<VertexDataFormat&>(format)){
    const auto attributesSize = format.m_Attributes.size();
    m_Data.reserve(attributesSize);
    for (size_t i = 0; i < m_Data.capacity(); ++i) {
        m_Data.emplace_back();
	}
    m_DataSizes.reserve(attributesSize);
    m_DataSizesCapacity.reserve(attributesSize);
    for (size_t i = 0; i < m_DataSizes.capacity(); ++i) {
        m_DataSizes.emplace_back(0);
        m_DataSizesCapacity.emplace_back(0);
	}
    m_Buffers.push_back(std::make_unique<VertexBufferObject>());
}
VertexData::~VertexData() {
    Engine::Renderer::deleteVAO(m_VAO);
}
void VertexData::clearData() {
    for (size_t i = 0; i < m_Data.size(); ++i) {
        m_Data[i].clear();
    }
    for (size_t i = 0; i < m_DataSizes.size(); ++i) {
        m_DataSizes[i] = 0;
    }
    for (size_t i = 0; i < m_DataSizesCapacity.size(); ++i) {
        m_DataSizesCapacity[i] = 0;
    }
    m_Indices.clear();
    m_Triangles.clear();
}
void VertexData::finalize() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (priv::Renderer::OPENGL_VERSION >= 30) {
        //build the vao itself
        Engine::Renderer::genAndBindVAO(m_VAO);
        sendDataToGPU(false, -1);
        m_Format.bind(*this);
        Engine::Renderer::bindVAO(0);
    }else{
        sendDataToGPU(false, -1);
        m_Format.bind(*this);
    }
}
void VertexData::bind() const {
    if (m_VAO) {
        Renderer::bindVAO(m_VAO);
    }else{
        for (auto& buffer : m_Buffers) {
            buffer->bind();
        }
        m_Format.bind(*this);
    }
}
void VertexData::unbind() const {
    if (m_VAO) {
        Renderer::bindVAO(0);
    }else{
        m_Format.unbind();
    }
}

void VertexData::setIndices(const unsigned int* data, size_t bufferCount, bool addToGPU, bool orphan, bool reCalcTriangles) {
    if (m_Buffers.size() == 1) {
        m_Buffers.push_back(std::make_unique<ElementBufferObject>());
    }
    if (data != &m_Indices[0]) {
        m_Indices.clear();
        m_Indices.reserve(bufferCount);
        for (unsigned int i = 0; i < bufferCount; ++i) {
            m_Indices.emplace_back(data[i]);
        }
    }
    if (reCalcTriangles) {
        const auto& positions = getData<glm::vec3>(0);
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
                }else if (j == 2) {
                    tri.position2 = positions[index];
                    tri.index2    = index;
                }else if (j == 3) {
                    tri.position3 = positions[index];
                    tri.index3    = index;
                    tri.midpoint  = tri.position1 + tri.position2 + tri.position3;
                    tri.midpoint /= 3.0f;
                    m_Triangles.push_back(std::move(tri));
                    j = 0;
                }
            }
        }
    }
    if (addToGPU) {
        auto& indiceBuffer = *m_Buffers[1];
        indiceBuffer.generate();
        indiceBuffer.bind();
        !orphan ? indiceBuffer.setData(m_Indices, BufferDataDrawType::Static) : indiceBuffer.setDataOrphan(m_Indices);
    }
}
void VertexData::sendDataToGPU(bool orphan, int attributeIndex) {
    //Interleaved format makes use of attributeIndex = -1
    /*
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
    */
    auto& vertexBuffer = *m_Buffers[0];
    vertexBuffer.generate();
    vertexBuffer.bind();

    size_t accumulator = 0;
    size_t size = 0;
    vector<uint8_t> gpu_data_buffer;
    if (m_Format.m_InterleavingType == VertexAttributeLayout::Interleaved) {
        size = (m_Format.m_Attributes[0].stride * m_DataSizes[0]);
        gpu_data_buffer.reserve(size);
        for (size_t i = 0; i < m_DataSizes[0]; ++i) {
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                const auto& sizeofT     = m_Format.m_Attributes[attribute_index].typeSize;
                auto* gpu_destination   = &(gpu_data_buffer.data())[accumulator];
                const auto at           = i * sizeofT;
                auto& src_data          = m_Data[attribute_index];
                auto* cpu_source        = &(src_data.data())[at];
                std::memcpy(gpu_destination, cpu_source, sizeofT);
                accumulator            += sizeofT;
            }
        }
        (!orphan) ? vertexBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : vertexBuffer.setDataOrphan(gpu_data_buffer.data());
    }else{
        if (attributeIndex == -1) {
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index)
                size += m_Format.m_Attributes[attribute_index].typeSize * m_DataSizes[attribute_index];
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                const auto blockSize  = m_DataSizes[attribute_index] * m_Format.m_Attributes[attribute_index].typeSize;
                auto* gpu_destination = &(gpu_data_buffer.data())[accumulator];
                auto& src_data        = m_Data[attribute_index];
                auto* cpu_source      = &(src_data.data())[0];
                std::memcpy(gpu_destination, cpu_source, blockSize);
                accumulator          += blockSize;
            }
            (!orphan) ? vertexBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : vertexBuffer.setDataOrphan(gpu_data_buffer.data());
        }else{
            size += (m_Format.m_Attributes[attributeIndex].typeSize * m_DataSizes[attributeIndex]);
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < m_Data.size(); ++attribute_index) {
                if (attribute_index != attributeIndex) {
                    accumulator += m_DataSizes[attribute_index] * m_Format.m_Attributes[attribute_index].typeSize;
                }else{
                    auto* gpu_destination     = &(gpu_data_buffer.data())[0];
                    auto& src_data            = m_Data[attribute_index];
                    auto* cpu_source          = &(src_data.data())[0];
                    std::memcpy(gpu_destination, cpu_source, size);
                    break;
                }
            }
            vertexBuffer.setData(size, accumulator, gpu_data_buffer.data());
        }
    }
    if (attributeIndex == -1) {
        auto& indiceBuffer = *m_Buffers[1];
        indiceBuffer.generate();
        indiceBuffer.bind();
        indiceBuffer.setData(m_Indices, BufferDataDrawType::Static);
    }
}