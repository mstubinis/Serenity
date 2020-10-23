#pragma once
#ifndef ENGINE_MATH_MATH_COMPRESSION_H
#define ENGINE_MATH_MATH_COMPRESSION_H

namespace Engine::Compression {

    uint8_t pack2NibblesIntoChar(float val1, float val2);
    glm::vec2    unpack2NibblesFromChar(uint8_t compressedValue);
    float        pack2NibblesIntoCharBasic(float val1, float val2);
    glm::vec2    unpack2NibblesFromCharBasic(float compressedValue);

    uint32_t pack3NormalsInto32Int(float x, float y, float z);
    glm::vec3     unpack3NormalsFrom32Int(uint32_t compressedValue);
    uint32_t pack3NormalsInto32Int(const glm::vec3& values);

    float     pack3FloatsInto1Float(float val1, float val2, float val3);
    float     pack3FloatsInto1Float(const glm::vec3& values);
    glm::vec3 unpack3FloatsInto1Float(float compressedValue);

    float     pack3FloatsInto1FloatUnsigned(float val1, float val2, float val3);
    float     pack3FloatsInto1FloatUnsigned(const glm::vec3& values);
    glm::vec3 unpack3FloatsInto1FloatUnsigned(float compressedValue);

    float     pack2FloatsInto1Float(float val1, float val2);
    float     pack2FloatsInto1Float(const glm::vec2& values);
    glm::vec2 unpack2FloatsInto1Float(float compressedValue);
};

#endif