#include <core/engine/mesh/VertexBufferObject.h>

using namespace std;

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


BufferObject::BufferObject() :buffer(0), capacity(0) { 
    drawType = BufferDataDrawType::Unassigned; 
}
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
    capacity = _size;
    glBufferData(type, _size, _data, drawType);
}
void BufferObject::setDataOrphan(const void* _data) {
    if (capacity == 0) return;
    glBufferData(type, capacity, nullptr, drawType);
    glBufferSubData(type, 0, capacity, _data);
}
void BufferObject::setData(size_t _size, size_t _startingIndex, const void* _data) {
    if (drawType == BufferDataDrawType::Unassigned) return;
    glBufferSubData(type, _startingIndex, _size, _data);
}
void BufferObject::setData(vector<char>& _data, BufferDataDrawType::Type _drawType) { setData(_data.size() * sizeof(char), _data.data(), _drawType); }
void BufferObject::setData(size_t _startingIndex, vector<char>& _data) { setData(_data.size() * sizeof(char), _startingIndex, _data.data()); }
void BufferObject::setDataOrphan(vector<char>& _data) { if (_data.size() * sizeof(char) != capacity) return; setDataOrphan(_data.data()); }




VertexBufferObject::VertexBufferObject() {
    type = BufferDataType::VertexArray;
}
ElementBufferObject::ElementBufferObject() {
    type = BufferDataType::ElementArray;
}
