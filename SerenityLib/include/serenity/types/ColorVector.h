#pragma once
#ifndef ENGINE_TYPES_COLOR_VECTOR_H
#define ENGINE_TYPES_COLOR_VECTOR_H

#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>

namespace Engine {
    class color_vector_4 final {
        using color_type = glm::vec<4, uint8_t, glm::packed_highp>;
        private:
            color_type m_Color = color_type(0);
        public:
            explicit color_vector_4(float color) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = (uint8_t)(color * 255.0f);
                }
            }
            explicit color_vector_4(float r, float g, float b, float a) {
                m_Color.r = (uint8_t)(r * 255.0f);
                m_Color.g = (uint8_t)(g * 255.0f);
                m_Color.b = (uint8_t)(b * 255.0f);
                m_Color.a = (uint8_t)(a * 255.0f);
            }
            explicit color_vector_4(const glm::vec4& color) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = (uint8_t)(color[i] * 255.0f);
                }
            }
            explicit color_vector_4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
                m_Color.r = r;
                m_Color.g = g;
                m_Color.b = b;
                m_Color.a = a;
            }
            explicit color_vector_4(uint8_t inColor) {
                for (int i = 0; i < 4; ++i) {
                    m_Color[i] = inColor;
                }
            }
            inline constexpr uint32_t toPackedInt() const noexcept {
                return (m_Color.r << 24) | (m_Color.g << 16) | (m_Color.b << 8) | m_Color.a;
            }
            glm::vec4 unpackInt(uint32_t i) const noexcept {
                constexpr float one_over_255 = 0.003921568627451f;
                float xx = (float)((i >> 24) & 255);
                float yy = (float)((i >> 16) & 255);
                float zz = (float)((i >> 8) & 255);
                float ww = (float)(i & 255);
                return glm::vec4(
                    xx * one_over_255,
                    yy * one_over_255,
                    zz * one_over_255,
                    ww * one_over_255
                );
            }

            inline constexpr uint16_t toPackedShort() const noexcept {
                int r = (int)(((float)m_Color.r * 16.0f) / 256.0f);
                int g = (int)(((float)m_Color.g * 16.0f) / 256.0f);
                int b = (int)(((float)m_Color.b * 16.0f) / 256.0f);
                int a = (int)(((float)m_Color.a * 16.0f) / 256.0f);
                return (r << 12) | (g << 8) | (b << 4) | a;
            }
            glm::vec4 unpackShort(uint16_t i) const noexcept {
                constexpr float one_over_15 = 0.0666666666666f;
                float xx = (float)((i >> 12) & 15);
                float yy = (float)((i >> 8) & 15);
                float zz = (float)((i >> 4) & 15);
                float ww = (float)(i & 15);
                return glm::vec4(
                    xx * one_over_15,
                    yy * one_over_15,
                    zz * one_over_15,
                    ww * one_over_15
                );
            }

            inline constexpr color_type::value_type getColChar(uint32_t index) const noexcept { return m_Color[index]; }
            inline constexpr color_type::value_type operator[](uint32_t index) const noexcept { return getColChar(index); }
            inline constexpr float getColFloat(uint32_t index) const noexcept { return m_Color[index] * 0.003921568627451f; }

            //return r as a color float from 0.0f to 1.0f
            inline constexpr float r() const noexcept { return (float)m_Color.r * 0.003921568627451f; }

            //return g as a color float from 0.0f to 1.0f
            inline constexpr float g() const noexcept { return (float)m_Color.g * 0.003921568627451f; }

            //return b as a color float from 0.0f to 1.0f
            inline constexpr float b() const noexcept { return (float)m_Color.b * 0.003921568627451f; }

            //return a as a color float from 0.0f to 1.0f
            inline constexpr float a() const noexcept { return (float)m_Color.a * 0.003921568627451f; }

            //return r as an unsigned char
            inline constexpr uint8_t rc() const noexcept { return m_Color.r; }

            //return g as an unsigned char
            inline constexpr uint8_t gc() const noexcept { return m_Color.g; }

            //return b as an unsigned char
            inline constexpr uint8_t bc() const noexcept { return m_Color.b; }

            //return a as an unsigned char
            inline constexpr uint8_t ac() const noexcept { return m_Color.a; }
    };
};
#endif