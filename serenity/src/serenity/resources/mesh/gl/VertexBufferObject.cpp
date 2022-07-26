#include <serenity/resources/mesh/gl/VertexBufferObject.h>

VertexBufferObject::~VertexBufferObject() {
    destroy();
}
void VertexBufferObject::generate() noexcept {
    if (!m_BufferGLHandle) {
        glGenBuffers(1, &m_BufferGLHandle);
    }
}
void VertexBufferObject::destroy() noexcept {
    if (m_BufferGLHandle) {
        glDeleteBuffers(1, &m_BufferGLHandle);
        m_BufferGLHandle = 0;
    }
}
void VertexBufferObject::bind() const noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, m_BufferGLHandle);
}

void VertexBufferObject::setData(size_t sizeInBytes, const void* data, BufferDataDrawType drawType) noexcept {
    m_DrawType = drawType;
    if (m_DrawType == BufferDataDrawType::Unassigned) {
        return;
    }
    if (sizeInBytes > m_Capacity) {
        m_Capacity = sizeInBytes;
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, m_DrawType);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, data);
    }
}
void VertexBufferObject::setData(size_t sizeInBytes, size_t startingIndex, const void* data) const noexcept {
    if (m_DrawType == BufferDataDrawType::Unassigned) {
        return;
    }
    glBufferSubData(GL_ARRAY_BUFFER, startingIndex, sizeInBytes, data);
}
void VertexBufferObject::setDataOrphan(const void* data) const noexcept {
    if (m_DrawType == BufferDataDrawType::Unassigned || m_Capacity == 0) {
        return;
    }
    glBufferData(GL_ARRAY_BUFFER, m_Capacity, nullptr, m_DrawType);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_Capacity, data);
}

void VertexBufferObject::specifyVertexAttribute(size_t index, int numComponents, size_t type, bool normalized, size_t stride, size_t offset) {
    glEnableVertexAttribArray(GLuint(index));
    glVertexAttribPointer(GLuint(index), numComponents, static_cast<GLenum>(type), static_cast<GLboolean>(normalized), GLsizei(stride), (void*)offset);
    //glDisableVertexAttribArray(GLuint(index));
}