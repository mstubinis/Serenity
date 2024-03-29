#pragma once
#ifndef ENGINE_RENDERER_TEXT_TEXT_RENDERER_H
#define ENGINE_RENDERER_TEXT_TEXT_RENDERER_H

class  Mesh;
class  Font;
struct CharGlyph;
namespace Engine::priv {
    class RenderModule;
}

#include <serenity/containers/PartialArray.h>
#include <serenity/resources/font/Font.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Engine::priv {
    class TextRenderer {
        public:
            using PositionBuffer = Engine::partial_array<glm::vec3,   Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>; // 4 points per letter
            using UVBuffer       = Engine::partial_array<glm::vec2,   Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>; // 4 uvs per letter
            using ColorBuffer    = Engine::partial_array<glm::u8vec4, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>; // 4 colors per letter
            using IndiceBuffer   = Engine::partial_array<uint32_t,    Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6>;  // 6 ind per letter
        private:
            PositionBuffer   m_Text_Points;
            UVBuffer         m_Text_UVs;
            ColorBuffer      m_Text_Colors;
            IndiceBuffer     m_Text_Indices;

            void internal_process_line(std::string_view text, int startIdx, int endIdx, float& x, float& y, float z, const Font&, float newLineGlyphHeight, uint32_t& w, int lineSize);
            void internal_render_text_left(std::string_view text, const Font&, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_text_center(std::string_view text, const Font&, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_text_right(std::string_view text, const Font&, float newLineGlyphHeight, float& x, float& y, float z);
        public:
            void renderText(Mesh&, RenderModule&, const std::string& text, Font&, const glm::vec4& color, TextAlignment, float& x, float& y, float z);

    };
}
#endif