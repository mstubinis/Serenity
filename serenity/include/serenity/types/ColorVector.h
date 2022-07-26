#pragma once
#ifndef ENGINE_TYPES_COLOR_VECTOR_H
#define ENGINE_TYPES_COLOR_VECTOR_H

#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <serenity/math/MathCompression.h>

namespace Engine {
    class color_vector_4 final {
        using color_type = glm::u8vec4;
        private:
            color_type m_Color = color_type{ 0 };
        public:
            explicit constexpr color_vector_4(float color) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = uint8_t(color * 255.0f);
                }
            }
            explicit constexpr color_vector_4(float r, float g, float b, float a) {
                m_Color.r = uint8_t(r * 255.0f);
                m_Color.g = uint8_t(g * 255.0f);
                m_Color.b = uint8_t(b * 255.0f);
                m_Color.a = uint8_t(a * 255.0f);
            }
            explicit constexpr color_vector_4(const glm::vec4& color) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = uint8_t(color[i] * 255.0f);
                }
            }
            explicit constexpr color_vector_4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
                m_Color.r = r;
                m_Color.g = g;
                m_Color.b = b;
                m_Color.a = a;
            }
            explicit constexpr color_vector_4(uint8_t inColor) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = inColor;
                }
            }
            [[nodiscard]] glm::vec4 unpackInt(uint32_t i) const noexcept {
                constexpr float one_over_255 = 0.003921568627451f;
                float x = float((i >> 24) & 255);
                float y = float((i >> 16) & 255);
                float z = float((i >> 8) & 255);
                float w = float(i & 255);
                return glm::vec4{
                    x * one_over_255,
                    y * one_over_255,
                    z * one_over_255,
                    w * one_over_255
                };
            }

            [[nodiscard]] inline uint16_t toPackedShort() const noexcept {
                return Engine::Compression::pack4ColorsInto16Int(m_Color.r, m_Color.g, m_Color.b, m_Color.a);
            }
            [[nodiscard]] inline constexpr uint32_t toPackedInt() const noexcept {
                return (m_Color.r << 24) | (m_Color.g << 16) | (m_Color.b << 8) | m_Color.a;
            }
            template<class PACKED_TYPE>
            [[nodiscard]] inline PACKED_TYPE toPacked() const noexcept {
                if constexpr (sizeof(PACKED_TYPE) == sizeof(uint16_t)) {
                    return Engine::Compression::pack4ColorsInto16Int(m_Color.r, m_Color.g, m_Color.b, m_Color.a);
                } else {
                    return (m_Color.r << 24) | (m_Color.g << 16) | (m_Color.b << 8) | m_Color.a;
                }
            }

            [[nodiscard]] glm::vec4 unpackShort(uint16_t i) const noexcept {
                return Engine::Compression::unpackFour16IntColorInto4Floats(i);
            }

            [[nodiscard]] inline constexpr color_type::value_type getColChar(uint32_t index) const noexcept { return m_Color[index]; }
            inline constexpr color_type::value_type operator[](uint32_t index) const noexcept { return getColChar(index); }
            [[nodiscard]] inline constexpr float getColFloat(uint32_t index) const noexcept { return m_Color[index] * 0.003921568627451f; }

            //return r as a color float from 0.0f to 1.0f
            [[nodiscard]] inline constexpr float r() const noexcept { return float(m_Color.r * 0.003921568627451f); }

            //return g as a color float from 0.0f to 1.0f
            [[nodiscard]] inline constexpr float g() const noexcept { return float(m_Color.g * 0.003921568627451f); }

            //return b as a color float from 0.0f to 1.0f
            [[nodiscard]] inline constexpr float b() const noexcept { return float(m_Color.b * 0.003921568627451f); }

            //return a as a color float from 0.0f to 1.0f
            [[nodiscard]] inline constexpr float a() const noexcept { return float(m_Color.a * 0.003921568627451f); }

            //return r as an unsigned char
            [[nodiscard]] inline constexpr uint8_t rc() const noexcept { return m_Color.r; }

            //return g as an unsigned char
            [[nodiscard]] inline constexpr uint8_t gc() const noexcept { return m_Color.g; }

            //return b as an unsigned char
            [[nodiscard]] inline constexpr uint8_t bc() const noexcept { return m_Color.b; }

            //return a as an unsigned char
            [[nodiscard]] inline constexpr uint8_t ac() const noexcept { return m_Color.a; }
    };
};
#endif