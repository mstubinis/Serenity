#pragma once
#ifndef ENGINE_RENDERER_TEXT_STRING_BUILDER_COLOR_H
#define ENGINE_RENDERER_TEXT_STRING_BUILDER_COLOR_H

#include <glm/ext.hpp>
#include <string_view>
#include <string>
#include <serenity/renderer/text/TextRendererIncludes.h>


class TextStringBuilderColor {
    private:
        glm::u8vec4 m_Color = glm::u8vec4{ static_cast<uint8_t>(255) };
        std::string m_Msg;
    public:
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, float r, float g, float b, float a)
            : m_Color{ static_cast<uint8_t>(glm::clamp(r, 0.0f, 1.0f) * 255.0f),
                       static_cast<uint8_t>(glm::clamp(g, 0.0f, 1.0f) * 255.0f),
                       static_cast<uint8_t>(glm::clamp(b, 0.0f, 1.0f) * 255.0f),
                       static_cast<uint8_t>(glm::clamp(a, 0.0f, 1.0f) * 255.0f) }
            , m_Msg{ std::forward<STRING>(msg) }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            : m_Color{ r, g, b, a }
            , m_Msg{ std::forward<STRING>(msg) }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, float r, float g, float b)
            : TextStringBuilderColor{ std::forward<STRING>(msg), r, g, b, 1.0f }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, uint8_t r, uint8_t g, uint8_t b)
            : TextStringBuilderColor{ std::forward<STRING>(msg), r, g, b, static_cast<uint8_t>(255) }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, float fillColorFloat)
            : TextStringBuilderColor{ std::forward<STRING>(msg), fillColorFloat, fillColorFloat, fillColorFloat, fillColorFloat }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, uint8_t fillColorByte)
            : TextStringBuilderColor{ std::forward<STRING>(msg), fillColorByte, fillColorByte, fillColorByte, fillColorByte }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, const glm::vec4& color)
            : TextStringBuilderColor{ std::forward<STRING>(msg), color.r, color.g, color.b, color.a }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, const glm::vec3& color)
            : TextStringBuilderColor{ std::forward<STRING>(msg), color.r, color.g, color.b, 1.0f }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, const glm::u8vec4& color)
            : TextStringBuilderColor{ std::forward<STRING>(msg), color.r, color.g, color.b, color.a }
        {}
        template<class STRING>
        explicit constexpr TextStringBuilderColor(STRING&& msg, const glm::u8vec3& color)
            : TextStringBuilderColor{ std::forward<STRING>(msg), color.r, color.g, color.b, static_cast<uint8_t>(255) }
        {}

        operator std::string() const;

        inline std::string str() const { return operator std::string(); }

        inline std::string operator+(const std::string& rhs) const { return str() + rhs; }
};

inline std::ostream& operator <<(std::ostream& stream, const TextStringBuilderColor& t) {
    return stream << t.str();
}
inline std::string operator +(const std::string& str, const TextStringBuilderColor& t) {
    return str + t.str();
}


#endif