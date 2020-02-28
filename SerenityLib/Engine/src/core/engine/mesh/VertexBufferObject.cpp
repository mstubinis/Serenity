#include <core/engine/renderer/Renderer.h>
#include <core/engine/mesh/VertexBufferObject.h>

using namespace std;

void BufferObject::generate() { 
    if (!buffer) { 
        glGenBuffers(1, &buffer); 
    } 
}
void BufferObject::destroy() { 
    if (buffer) { 
        glDeleteBuffers(1, &buffer);
        buffer = 0; 
    } 
}
void BufferObject::bind() {
    glBindBuffer(type, buffer);
}
void BufferObject::setData(size_t _size, const void* _data, BufferDataDrawType::Type _drawType) {
    drawType = _drawType;
    if (_size > capacity) {
        capacity = _size;
        glBufferData(type, _size, _data, drawType);
    }else{
        glBufferSubData(type, 0, _size, _data);
    }
}
void BufferObject::setDataOrphan(const void* data) {
    if (capacity == 0) {
        return;
    }
    glBufferData   (type, capacity, nullptr,  drawType);
    glBufferSubData(type, 0,        capacity, data);
}
void BufferObject::setData(size_t _size, size_t _startingIndex, const void* _data) {
    if (drawType == BufferDataDrawType::Unassigned) {
        return;
    }
    glBufferSubData(type, _startingIndex, _size, _data);
}

VertexBufferObject::VertexBufferObject() {
    type = BufferDataType::VertexArray;
}
ElementBufferObject::ElementBufferObject() {
    type = BufferDataType::ElementArray;
}