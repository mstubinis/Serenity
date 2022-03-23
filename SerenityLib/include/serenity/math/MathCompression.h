#pragma once
#ifndef ENGINE_MATH_MATH_COMPRESSION_H
#define ENGINE_MATH_MATH_COMPRESSION_H

#include <serenity/dependencies/glm.h>

namespace Engine::Compression {
    [[nodiscard]] uint8_t      pack2NibblesIntoChar(float val1, float val2);
    [[nodiscard]] glm::vec2    unpack2NibblesFromChar(uint8_t compressedValue);
    [[nodiscard]] float        pack2NibblesIntoCharBasic(float val1, float val2);
    [[nodiscard]] glm::vec2    unpack2NibblesFromCharBasic(float compressedValue);

    [[nodiscard]] uint32_t     pack3NormalsInto32Int(float x, float y, float z);
    [[nodiscard]] glm::vec3    unpack3NormalsFrom32Int(uint32_t compressedValue);
    [[nodiscard]] uint32_t     pack3NormalsInto32Int(const glm::vec3& values);

    [[nodiscard]] float     pack3FloatsInto1Float(float val1, float val2, float val3);
    [[nodiscard]] float     pack3FloatsInto1Float(const glm::vec3& values);
    [[nodiscard]] glm::vec3 unpack3FloatsInto1Float(float compressedValue);

    [[nodiscard]] float     pack3FloatsInto1FloatUnsigned(float val1, float val2, float val3);
    [[nodiscard]] float     pack3FloatsInto1FloatUnsigned(const glm::vec3& values);
    [[nodiscard]] glm::vec3 unpack3FloatsInto1FloatUnsigned(float compressedValue);

    [[nodiscard]] float     pack2FloatsInto1Float(float val1, float val2);
    [[nodiscard]] float     pack2FloatsInto1Float(const glm::vec2& values);
    [[nodiscard]] glm::vec2 unpack2FloatsInto1Float(float compressedValue);

    [[nodiscard]] uint16_t pack4ColorsInto16Int(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    [[nodiscard]] glm::u8vec4 unpackFour16IntColorInto4Chars(uint16_t i) noexcept;
    [[nodiscard]] glm::vec4 unpackFour16IntColorInto4Floats(uint16_t i) noexcept;
};

#endif