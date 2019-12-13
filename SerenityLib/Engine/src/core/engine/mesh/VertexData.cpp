#include <core/engine/mesh/VertexData.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;
using namespace std;

VertexData::VertexData(const VertexDataFormat& _format) : format(const_cast<VertexDataFormat&>(_format)), vao(0) {
    data.reserve(_format.attributes.size());
    for (size_t i = 0; i < data.capacity(); ++i) { 
		//data.emplace_back(nullptr); 
        data.emplace_back();
	}
    dataSizes.reserve(_format.attributes.size());
    dataSizesCapacity.reserve(_format.attributes.size());
    for (size_t i = 0; i < dataSizes.capacity(); ++i) { 
		dataSizes.emplace_back(0); 
        dataSizesCapacity.emplace_back(0);
	}
    buffers.push_back(std::make_unique<VertexBufferObject>());
}
VertexData::~VertexData() {
    Renderer::deleteVAO(vao);

    for (size_t i = 0; i < data.size(); ++i) {
        //delete[] (data[i]);
    }
    /*
    data.clear();
    dataSizes.clear();
    dataSizesCapacity.clear();
    indices.clear();
    triangles.clear();
    */
}

void VertexData::finalize() {
    Renderer::deleteVAO(vao);
    if (epriv::RenderManager::OPENGL_VERSION >= 30) {
        //build the vao itself
        Engine::Renderer::genAndBindVAO(vao);
        sendDataToGPU(false, -1);
        format.bind(*this);
        Engine::Renderer::bindVAO(0);
    }else{
        sendDataToGPU(false, -1);
        format.bind(*this);
    }
}
void VertexData::bind() {
    if (vao) {
        Renderer::bindVAO(vao);
    }else{
        for (auto& buffer : buffers)
            buffer->bind();
        format.bind(*this);
    }
}
void VertexData::unbind() {
    if (vao) {
        Renderer::bindVAO(0);
    }else{
        format.unbind();
    }
}

void VertexData::setIndices(vector<unsigned short>& _data, const bool addToGPU, const bool orphan, const bool reCalcTriangles) {
    if (buffers.size() == 1)
        buffers.push_back(std::make_unique<ElementBufferObject>());
    auto& _buffer = *buffers[1];
    if (&_data != &indices) {
        indices.clear();
        indices.reserve(_data.size());
        for (auto& indice : _data) {
            indices.emplace_back(indice);
        }
    }
    if (reCalcTriangles) {
        auto& positions = getData<glm::vec3>(0);
        if (positions.size() >= 0) {
            triangles.clear();
            triangles.reserve(_data.size() / 3);
            size_t j = 0;
            Engine::epriv::Triangle tri;
            for (size_t i = 0; i < indices.size(); ++i) {
                ++j;
                auto& index = indices[i];
                if (j == 1) {
                    tri.position1 = positions[index];
                    tri.index1 = index;
                }else if (j == 2) {
                    tri.position2 = positions[index];
                    tri.index2 = index;
                }else if (j == 3) {
                    tri.position3 = positions[index];
                    tri.index3 = index;
                    tri.midpoint = tri.position1 + tri.position2 + tri.position3;
                    tri.midpoint /= 3.0f;
                    triangles.push_back(std::move(tri));
                    j = 0;
                }
            }
        }
    }
    if (addToGPU) {
        _buffer.generate();
        _buffer.bind();
        !orphan ? _buffer.setData(indices, BufferDataDrawType::Static) : _buffer.setDataOrphan(indices);
    }
}
void VertexData::sendDataToGPU(const bool orphan, const int attributeIndex) {
    //Interleaved format makes use of attributeIndex = -1
    /*
    Interleaved,    // | pos uv norm | pos uv norm | pos uv norm    | ... etc ... 
    NonInterleaved, // | pos pos pos | uv  uv  uv  | norm norm norm | ... etc ... 
    */
    auto& _vBuffer = *buffers[0];
    _vBuffer.generate(); _vBuffer.bind();

    size_t accumulator = 0;
    size_t size = 0;
    vector<uint8_t> gpu_data_buffer;
    if (format.interleavingType == VertexAttributeLayout::Interleaved) {
        size = (format.attributes[0].stride * dataSizes[0]);
        gpu_data_buffer.reserve(size);
        for (size_t i = 0; i < dataSizes[0]; ++i) {
            for (size_t attribute_index = 0; attribute_index < data.size(); ++attribute_index) {
                const auto& sizeofT     = format.attributes[attribute_index].typeSize;
                auto* gpu_destination   = &(gpu_data_buffer.data())[accumulator];
                const auto at           = i * sizeofT;
                auto& src_data          = data[attribute_index];
                auto* cpu_source        = &(src_data.data())[at];
                std::memcpy(gpu_destination, cpu_source, sizeofT);
                accumulator            += sizeofT;
            }
        }
        (!orphan) ? _vBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(gpu_data_buffer.data());
    }else{
        if (attributeIndex == -1) {
            for (size_t attribute_index = 0; attribute_index < data.size(); ++attribute_index)
                size += format.attributes[attribute_index].typeSize * dataSizes[attribute_index];
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < data.size(); ++attribute_index) {
                const auto blockSize  = dataSizes[attribute_index] * format.attributes[attribute_index].typeSize;
                auto* gpu_destination = &(gpu_data_buffer.data())[accumulator];
                auto& src_data        = data[attribute_index];
                auto* cpu_source      = &(src_data.data())[0];
                std::memcpy(gpu_destination, cpu_source, blockSize);
                accumulator          += blockSize;
            }
            (!orphan) ? _vBuffer.setData(size, gpu_data_buffer.data(), BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(gpu_data_buffer.data());
        }else{
            size += (format.attributes[attributeIndex].typeSize * dataSizes[attributeIndex]);
            gpu_data_buffer.reserve(size);
            for (size_t attribute_index = 0; attribute_index < data.size(); ++attribute_index) {
                if (attribute_index != attributeIndex) {
                    accumulator += dataSizes[attribute_index] * format.attributes[attribute_index].typeSize;
                }else{
                    auto* gpu_destination     = &(gpu_data_buffer.data())[0];
                    auto& src_data            = data[attribute_index];
                    auto* cpu_source          = &(src_data.data())[0];
                    std::memcpy(gpu_destination, cpu_source, size);
                    break;
                }
            }
            _vBuffer.setData(size, accumulator, gpu_data_buffer.data());
        }
    }
    if (attributeIndex == -1) {
        auto& _iBuffer = *buffers[1];
        _iBuffer.generate();
        _iBuffer.bind();
        _iBuffer.setData(indices, BufferDataDrawType::Static);
    }
}