
#include <serenity/resources/mesh/VertexData.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/GL.h>

void VertexDataFormat::bind(const VertexData& vertData) const noexcept {
    if (m_InterleavingType == VertexAttributeLayout::Interleaved) {
        for (GLuint i = 0; i < m_Attributes.size(); ++i) {
            const auto& attribute = m_Attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.numComponents, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }else{
        size_t accumulator = 0;
        for (GLuint i = 0; i < m_Attributes.size(); ++i) {
            const auto& attribute = m_Attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.numComponents, attribute.type, attribute.normalized, 0, (void*)accumulator);
            accumulator += vertData.m_Data[i].m_Size * attribute.typeSize;
        }
    }
}
void VertexDataFormat::unbind() const noexcept {
    for (GLuint i = 0; i < m_Attributes.size(); ++i) {
        glDisableVertexAttribArray(i);
    }
}

VertexDataFormat VertexDataFormat::VertexDataPositionsOnly = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0, sizeof(glm::vec3)); //positions
    return data;
}();
VertexDataFormat VertexDataFormat::VertexData2D = []() {
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::u8vec4);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT,         false, stride, 0, sizeof(glm::vec3)); //positions
    data.add(2, GL_FLOAT,         false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(4, GL_UNSIGNED_BYTE, true,  stride, 20, sizeof(glm::u8vec4)); //colors (normalized to transfer color from [0.0 - 255.0] to [0.0 - 1.0]
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataNoLighting = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0,  sizeof(glm::vec3));   //positions
    data.add(2, GL_FLOAT, false, stride, 12, sizeof(glm::vec2));   //uvs
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataBasic = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint));    //normals
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint));    //binormals
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint));    //tangents
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataAnimated = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint) + sizeof(glm::vec4) + sizeof(glm::vec4);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint));    //normals
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint));    //binormals
    data.add(4, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint));    //tangents
    data.add(4,       GL_FLOAT,              false, stride, 32, sizeof(glm::vec4)); //boneIDs
    data.add(4,       GL_FLOAT,              false, stride, 48, sizeof(glm::vec4)); //boneWeights
    return data;
}();
