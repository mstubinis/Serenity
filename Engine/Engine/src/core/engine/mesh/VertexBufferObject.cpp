#include <core/engine/mesh/VertexBufferObject.h>


BufferObject::BufferObject() :buffer(0) { 
}
void BufferObject::generate() { if (!buffer) { glGenBuffers(1, &buffer); } }
void BufferObject::destroy() { if (buffer) { glDeleteBuffers(1, &buffer); buffer = 0; } }



void VertexBufferObject::bind() { glBindBuffer(GL_ARRAY_BUFFER, buffer); }
void VertexBufferObject::bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { glBufferData(GL_ARRAY_BUFFER, _size, _data, _drawType); }
void VertexBufferObject::bufferSubData(size_t _size, const void* _data) { 
    glBufferSubData(GL_ARRAY_BUFFER, 0, _size, _data); 
}
void VertexBufferObject::bufferSubData(size_t _size, size_t _startingIndex, const void* _data) { 
    glBufferSubData(GL_ARRAY_BUFFER, _startingIndex, _size, _data); 
}


void ElementBufferObject::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); }
void ElementBufferObject::bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { glBufferData(GL_ELEMENT_ARRAY_BUFFER, _size, _data, _drawType); }
void ElementBufferObject::bufferSubData(size_t _size, const void* _data) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _size, _data); 
}
void ElementBufferObject::bufferSubData(size_t _size, size_t _startingIndex, const void* _data) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _startingIndex, _size, _data); 
}
