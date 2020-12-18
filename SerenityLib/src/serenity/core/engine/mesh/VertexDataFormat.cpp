
#include <serenity/core/engine/mesh/VertexData.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/GL.h>

void VertexDataFormat::bind(const VertexData& vertData) const noexcept {
    if (m_InterleavingType == VertexAttributeLayout::Interleaved) {
        for (size_t i = 0; i < m_Attributes.size(); ++i) {
            const auto& attribute = m_Attributes[i];
            glEnableVertexAttribArray((GLuint)i);
            glVertexAttribPointer((GLuint)i, attribute.size, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }else{
        size_t accumulator = 0;
        for (size_t i = 0; i < m_Attributes.size(); ++i) {
            const auto& attribute = m_Attributes[i];
            glEnableVertexAttribArray((GLuint)i);
            glVertexAttribPointer((GLuint)i, attribute.size, attribute.type, attribute.normalized, 0, (void*)accumulator);
            accumulator += vertData.m_Data[i].m_Size * attribute.typeSize;
        }
    }
}

VertexDataFormat VertexDataFormat::VertexDataPositionsOnly = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0, sizeof(glm::vec3)); //positions
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataNoLighting = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0,  sizeof(glm::vec3)); //positions
    data.add(2, GL_FLOAT, false, stride, 12, sizeof(glm::vec2)); //uvs
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataBasic = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint)); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint)); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint)); //tangents
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataAnimated = []() {
    VertexDataFormat data;
    const size_t stride     = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint) + sizeof(glm::vec4) + sizeof(glm::vec4);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint));    //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint));    //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint));    //tangents
    data.add(4,       GL_FLOAT,              false, stride, 32, sizeof(glm::vec4)); //boneIDs
    data.add(4,       GL_FLOAT,              false, stride, 48, sizeof(glm::vec4)); //boneWeights
    return data;
}();
