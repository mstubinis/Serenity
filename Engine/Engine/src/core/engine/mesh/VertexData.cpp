#include "VertexData.h"
#include <glm/glm.hpp>

VertexDataFormat VertexDataFormat::VertexDataBasic = [&]() {
    unsigned int _stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    VertexDataFormat data;
    data.add(3, GL_FLOAT, false, _stride, 0); //positions
    data.add(2, GL_FLOAT, false, _stride, 12); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 20); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 24); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 28); //tangents
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataAnimated = [&]() {
    unsigned int _stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint) + sizeof(glm::vec4) + sizeof(glm::vec4);
    VertexDataFormat data;
    data.add(3, GL_FLOAT, false, _stride, 0); //positions
    data.add(2, GL_FLOAT, false, _stride, 12); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 20); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 24); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 28); //tangents
    data.add(4, GL_FLOAT, false, _stride, 32); //boneIDs
    data.add(4, GL_FLOAT, false, _stride, 48); //boneWeights
    return data;
}();
