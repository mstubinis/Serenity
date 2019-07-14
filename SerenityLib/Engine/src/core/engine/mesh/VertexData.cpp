#include <core/engine/mesh/VertexData.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;
using namespace std;

VertexData::VertexData(const VertexDataFormat& _format) :format(const_cast<VertexDataFormat&>(_format)), vao(0) {
    data.reserve(_format.attributes.size());
    for (size_t i = 0; i < data.capacity(); ++i) { 
		data.emplace_back(nullptr); 
	}
    dataSizes.reserve(_format.attributes.size());
    for (size_t i = 0; i < dataSizes.capacity(); ++i) { 
		dataSizes.emplace_back(0); 
	}
    buffers.push_back(std::make_unique<VertexBufferObject>());
}
VertexData::~VertexData() {
    Renderer::deleteVAO(vao);
    for (size_t i = 0; i < data.size(); ++i) {
        free(data[i]);
    }
    vector_clear(data);
    vector_clear(dataSizes);
    vector_clear(indices);
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
void VertexData::setDataIndices(vector<ushort>& _data, const bool addToGPU, const bool orphan) {
    if (buffers.size() == 1)
        buffers.push_back(std::make_unique<ElementBufferObject>());
    auto& _buffer = *buffers[1];
    indices.swap(_data);
    if (addToGPU) {
        _buffer.generate();
        _buffer.bind();
        !orphan ? _buffer.setData(indices, BufferDataDrawType::Static) : _buffer.setDataOrphan(indices);
    }
}
void VertexData::sendDataToGPU(const bool orphan, const int attributeIndex) {
    //Interleaved format makes use of attributeIndex = -1

    auto& _vBuffer = *buffers[0];
    _vBuffer.generate(); _vBuffer.bind();

    char* buffer = nullptr;
    size_t accumulator = 0;
    size_t size = 0;
    if (format.interleavingType == VertexAttributeLayout::Interleaved) {
        size = (format.attributes[0].stride * dataSizes[0]);
        buffer = (char*)malloc(size);
        for (size_t i = 0; i < dataSizes[0]; ++i) {
            for (size_t j = 0; j < data.size(); ++j) {
                const auto& sizeofT = format.attributes[j].typeSize;
                //                dst                      source
                std::memmove(&buffer[accumulator], &(data[j])[i * sizeofT], sizeofT);
                accumulator += sizeofT;
            }
        }
        !orphan ? _vBuffer.setData(size, buffer, BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(buffer);
    }else{
        if (attributeIndex == -1) {
            for (size_t i = 0; i < data.size(); ++i)
                size += format.attributes[i].typeSize * dataSizes[i];
            buffer = (char*)malloc(size);
            for (size_t i = 0; i < data.size(); ++i) {
                const auto& blockSize = dataSizes[i] * format.attributes[i].typeSize;
                //                   dst               source
                std::memmove(&buffer[accumulator], &(data[i])[0], blockSize);
                accumulator += blockSize;
            }
            !orphan ? _vBuffer.setData(size, buffer, BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(buffer);
        }else{
            size += (format.attributes[attributeIndex].typeSize * dataSizes[attributeIndex]);
            buffer = (char*)malloc(size);
            for (size_t i = 0; i < data.size(); ++i) {
                if (i != attributeIndex) {
                    accumulator += dataSizes[i] * format.attributes[i].typeSize;
                }else{
                    //              dst          source
                    std::memmove(&buffer[0], &(data[i])[0], size);
                    break;
                }
            }
            _vBuffer.setData(size, accumulator, buffer);
        }
    }
    free(buffer);
    if (attributeIndex == -1) {
        auto& _iBuffer = *buffers[1];
        _iBuffer.generate();
        _iBuffer.bind();
        _iBuffer.setData(indices, BufferDataDrawType::Static);
    }
}