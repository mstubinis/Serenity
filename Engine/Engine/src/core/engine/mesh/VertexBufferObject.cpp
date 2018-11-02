#include <core/engine/mesh/VertexBufferObject.h>


//tips for better usage
/*

glBufferData *MUST* be called first for any buffer allocation to actually allocate a region of GPU memory. From there you can orphan / subData / map / etc

glBufferSubData works on a small portion of the total allocation
glBufferSubData can work from a starting index

calling glBufferData a second time after the initial call should be used in only these situations:
    - called with NULL as data, and using the same allocation size to orphan a buffer
    - called to change the size of the allocation

use glBufferSubData to change portions of the data.

look up proper usage of glMapBuffer and glMapBufferRange

*/





BufferObject::BufferObject() :buffer(0), capacity(0){ drawType = BufferDataType::Unassigned; }
void BufferObject::generate() { if (!buffer) { glGenBuffers(1, &buffer); } }
void BufferObject::destroy() { if (buffer) { glDeleteBuffers(1, &buffer); buffer = 0; } }



void VertexBufferObject::bind() { 
    glBindBuffer(GL_ARRAY_BUFFER, buffer); 
}
void VertexBufferObject::bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { 
    drawType = _drawType;
    capacity = _size;
    glBufferData(GL_ARRAY_BUFFER, _size, _data, _drawType); 
}
void VertexBufferObject::bufferDataOrphan(const void* _data) {
    if (capacity == 0) return;
    glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, drawType);
    glBufferSubData(GL_ARRAY_BUFFER, 0, capacity, _data);
}
void VertexBufferObject::bufferSubData(size_t _size, size_t _startingIndex, const void* _data) { 
    glBufferSubData(GL_ARRAY_BUFFER, _startingIndex, _size, _data); 
}




void ElementBufferObject::bind() { 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); 
}
void ElementBufferObject::bufferData(size_t _size, const void* _data, BufferDataType::Type _drawType) { 
    drawType = _drawType;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _size, _data, _drawType); 
}
void ElementBufferObject::bufferDataOrphan(const void* _data) {
    if (capacity == 0) return;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, capacity, 0, drawType);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, capacity, _data);
}
void ElementBufferObject::bufferSubData(size_t _size, size_t _startingIndex, const void* _data) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _startingIndex, _size, _data); 
}
