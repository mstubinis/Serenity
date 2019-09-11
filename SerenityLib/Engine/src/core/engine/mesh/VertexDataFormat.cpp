#include <core/engine/mesh/VertexData.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/GL.h>

VertexDataFormat::VertexDataFormat() {
    interleavingType = VertexAttributeLayout::Interleaved;
}
void VertexDataFormat::add(const int _size, const int _type, const bool _normalized, const int _stride, const size_t _offset, const size_t _typeSize) {
    attributes.emplace_back(_size, _type, _normalized, _stride, _offset, _typeSize);
}
void VertexDataFormat::bind(const VertexData& vertData) {
    if (interleavingType == VertexAttributeLayout::Interleaved) {
        for (size_t i = 0; i < attributes.size(); ++i) {
            const auto& attribute = attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.size, attribute.type, attribute.normalized, attribute.stride, (void*)attribute.offset);
        }
    }else{
        size_t accumulator = 0;
        for (size_t i = 0; i < attributes.size(); ++i) {
            const auto& attribute = attributes[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.size, attribute.type, attribute.normalized, 0, (void*)accumulator);
            accumulator += vertData.dataSizes[i] * attribute.typeSize;
        }
    }
}
void VertexDataFormat::unbind() { 
    for (size_t i = 0; i < attributes.size(); ++i)
        glDisableVertexAttribArray(i); 
}


VertexDataFormat VertexDataFormat::VertexDataPositionsOnly = [&]() {
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3);
    data.interleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0, sizeof(glm::vec3)); //positions
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataNoLighting = [&]() {
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2);
    data.interleavingType = VertexAttributeLayout::Interleaved;
    data.add(3, GL_FLOAT, false, stride, 0,  sizeof(glm::vec3)); //positions
    data.add(2, GL_FLOAT, false, stride, 12, sizeof(glm::vec2)); //uvs
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataBasic = [&]() {
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    data.interleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint)); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint)); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint)); //tangents
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataAnimated = [&]() {
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint) + sizeof(glm::vec4) + sizeof(glm::vec4);
    data.interleavingType = VertexAttributeLayout::Interleaved;  
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint));    //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint));    //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint));    //tangents
    data.add(4,       GL_FLOAT,              false, stride, 32, sizeof(glm::vec4)); //boneIDs
    data.add(4,       GL_FLOAT,              false, stride, 48, sizeof(glm::vec4)); //boneWeights
    return data;
}();
