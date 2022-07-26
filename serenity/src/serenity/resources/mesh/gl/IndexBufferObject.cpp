#include <serenity/resources/mesh/gl/IndexBufferObject.h>

IndexBufferObject::~IndexBufferObject() {
    destroy();
}
void IndexBufferObject::generate() noexcept {
    if (!m_BufferGLHandle) {
        glGenBuffers(1, &m_BufferGLHandle);
    }
}
void IndexBufferObject::destroy() noexcept {
    if (m_BufferGLHandle) {
        glDeleteBuffers(1, &m_BufferGLHandle);
        m_BufferGLHandle = 0;
    }
}
void IndexBufferObject::bind() const noexcept {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferGLHandle);
}

void IndexBufferObject::setData(size_t sizeInBytes, const void* data, BufferDataDrawType drawType) noexcept {
    m_DrawType = drawType;
    if (m_DrawType == BufferDataDrawType::Unassigned) {
        return;
    }
    if (sizeInBytes > m_Capacity) {
        m_Capacity = sizeInBytes;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, m_DrawType);
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeInBytes, data);
    }
}
void IndexBufferObject::setData(size_t sizeInBytes, size_t startingIndex, const void* data) const noexcept {
    if (m_DrawType == BufferDataDrawType::Unassigned) {
        return;
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, startingIndex, sizeInBytes, data);
}
void IndexBufferObject::setDataOrphan(const void* data) const noexcept {
    if (m_DrawType == BufferDataDrawType::Unassigned || m_Capacity == 0) {
        return;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Capacity, nullptr, m_DrawType);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Capacity, data);
}