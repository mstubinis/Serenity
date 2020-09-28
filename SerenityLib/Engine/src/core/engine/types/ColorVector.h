#pragma once
#ifndef ENGINE_TYPES_COLOR_VECTOR_H
#define ENGINE_TYPES_COLOR_VECTOR_H

namespace Engine {
    class color_vector_4 final {
        using color_type = glm::vec<4, std::uint8_t, glm::packed_highp>;
        private:
            color_type color = color_type(0);
        public:
            explicit color_vector_4(float inColor) {
                for (int i = 0; i < 4; ++i)
                    color[i] = (std::uint8_t)(inColor * 255.0f);
            }
            explicit color_vector_4(float inR, float inG, float inB, float inA) {
                color.r = (std::uint8_t)(inR * 255.0f);
                color.g = (std::uint8_t)(inG * 255.0f);
                color.b = (std::uint8_t)(inB * 255.0f);
                color.a = (std::uint8_t)(inA * 255.0f);
            }
            explicit color_vector_4(const glm::vec4& inColor) {
                for (int i = 0; i < 4; ++i)
                    color[i] = (std::uint8_t)(inColor[i] * 255.0f);
            }
            explicit color_vector_4(std::uint8_t inR, std::uint8_t inG, std::uint8_t inB, std::uint8_t inA) {
                color.r = inR;
                color.g = inG;
                color.b = inB;
                color.a = inA;
            }
            explicit color_vector_4(std::uint8_t inColor) {
                for (int i = 0; i < 4; ++i)
                    color[i] = inColor;
            }
            inline CONSTEXPR std::uint32_t toPackedInt() const noexcept {
                return (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
            }
            glm::vec4 unpackInt(std::uint32_t i) const noexcept {
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


            inline CONSTEXPR std::uint16_t toPackedShort() const noexcept {
                int r = (int)(((float)color.r * 16.0f) / 256.0f);
                int g = (int)(((float)color.g * 16.0f) / 256.0f);
                int b = (int)(((float)color.b * 16.0f) / 256.0f);
                int a = (int)(((float)color.a * 16.0f) / 256.0f);
                return (r << 12) | (g << 8) | (b << 4) | a;
            }
            glm::vec4 unpackShort(std::uint16_t i) const noexcept {
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


            //return r as a color float from 0.0f to 1.0f
            inline CONSTEXPR float r() const noexcept { return (float)color.r * 0.003921568627451f; }

            //return g as a color float from 0.0f to 1.0f
            inline CONSTEXPR float g() const noexcept { return (float)color.g * 0.003921568627451f; }

            //return b as a color float from 0.0f to 1.0f
            inline CONSTEXPR float b() const noexcept { return (float)color.b * 0.003921568627451f; }

            //return a as a color float from 0.0f to 1.0f
            inline CONSTEXPR float a() const noexcept { return (float)color.a * 0.003921568627451f; }

            //return r as an unsigned char
            inline CONSTEXPR std::uint8_t rc() const noexcept { return color.r; }

            //return g as an unsigned char
            inline CONSTEXPR std::uint8_t gc() const noexcept { return color.g; }

            //return b as an unsigned char
            inline CONSTEXPR std::uint8_t bc() const noexcept { return color.b; }

            //return a as an unsigned char
            inline CONSTEXPR std::uint8_t ac() const noexcept { return color.a; }
    };
};
#endif