#include "VertexData.h"
#include <glm/glm.hpp>

VertexDataFormat VertexDataFormat::VertexDataBasic = [&]() {
    size_t _stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    VertexDataFormat data;
    data.add(3, GL_FLOAT, false, _stride, 0, sizeof(glm::vec3)); //positions
    data.add(2, GL_FLOAT, false, _stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 20, sizeof(GLuint)); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 24, sizeof(GLuint)); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 28, sizeof(GLuint)); //tangents
    return data;
}();
VertexDataFormat VertexDataFormat::VertexDataAnimated = [&]() {
    size_t _stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint) + sizeof(glm::vec4) + sizeof(glm::vec4);
    VertexDataFormat data;
    data.add(3, GL_FLOAT, false, _stride, 0, sizeof(glm::vec3)); //positions
    data.add(2, GL_FLOAT, false, _stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 20, sizeof(GLuint)); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 24, sizeof(GLuint)); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true, _stride, 28, sizeof(GLuint)); //tangents
    data.add(4, GL_FLOAT, false, _stride, 32, sizeof(glm::vec4)); //boneIDs
    data.add(4, GL_FLOAT, false, _stride, 48, sizeof(glm::vec4)); //boneWeights
    return data;
}();
