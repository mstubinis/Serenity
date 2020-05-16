#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

class Texture;
struct FT_GlyphSlotRec_;

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/engine/fonts/FontIncludes.h>

struct CharGlyph final {
    unsigned int char_id;
    unsigned int x         = 0;
    unsigned int y         = 0;
    unsigned int width;
    unsigned int height;
    unsigned int xadvance  = 0;
    int xoffset            = 0;
    int yoffset            = 0;

    //mesh specific
    std::vector<glm::vec3> pts;
    std::vector<glm::vec2> uvs;
};
class Font final: public EngineResource {
    public:
        static const unsigned int MAX_CHARACTERS_RENDERED_PER_FRAME    = 4096;
    private:
        Texture* m_FontTexture                                         = nullptr;
        float    m_MaxHeight                                           = 0.0f;
        float    m_LineHeight                                          = 8.0f;
        std::unordered_map<unsigned char, CharGlyph> m_CharGlyphs;

        void init_simple(const std::string& filename, int height, int width);
        void init_freetype(const std::string& filename, int height, int width);
        void init(const std::string& filename, int height, int width);

        std::vector<std::vector<unsigned char>> generate_bitmap(const FT_GlyphSlotRec_&);

    public:
        Font(const std::string& filename, int height, int width, float line_height);
        ~Font();

        void renderText(
            const std::string& text,
            const glm::vec2& pos,
            const glm::vec4& color = glm::vec4(1),
            const float angle = 0.0f,
            const glm::vec2& scl = glm::vec2(1.0f),
            const float depth = 0.1f,
            const TextAlignment::Type = TextAlignment::Left,
            const glm::vec4& scissor = glm::vec4(-1.0f)
        );
        static void renderTextStatic(
            const std::string& text,
            const glm::vec2& pos,
            const glm::vec4& color = glm::vec4(1),
            const float angle = 0.0f,
            const glm::vec2& scl = glm::vec2(1.0f),
            const float depth = 0.1f,
            const TextAlignment::Type = TextAlignment::Left,
            const glm::vec4& scissor = glm::vec4(-1.0f)
        );


        const float getTextWidth(std::string_view text) const;
        const float getTextHeight(std::string_view text) const;
        const float getLineHeight() const;
        const float getMaxHeight() const;

        Texture* getGlyphTexture() const;

        const CharGlyph& getGlyphData(const unsigned char character) const;
};
#endif
