#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

class Texture;
struct FT_GlyphSlotRec_;

#include <core/engine/resources/Resource.h>
#include <core/engine/fonts/FontIncludes.h>

struct CharGlyph final {
    unsigned int char_id   = 0;
    unsigned int x         = 0;
    unsigned int y         = 0;
    unsigned int width     = 0;
    unsigned int height    = 0;
    unsigned int xadvance  = 0;
    int xoffset            = 0;
    int yoffset            = 0;

    //mesh specific
    std::vector<glm::vec3> pts;
    std::vector<glm::vec2> uvs;
};
class Font final: public Resource {
    public:
        static constexpr unsigned int MAX_CHARACTERS_RENDERED_PER_FRAME    = 4096;
    private:
        Texture*                                      m_FontTexture    = nullptr;
        float                                         m_MaxHeight      = 0.0f;
        float                                         m_LineHeight     = 8.0f;
        std::unordered_map<std::uint8_t, CharGlyph>   m_CharGlyphs;

        void init_simple(const std::string& filename, int height, int width);
        void init_freetype(const std::string& filename, int height, int width);
        void init(const std::string& filename, int height, int width);

        std::vector<std::vector<std::uint8_t>> generate_bitmap(const FT_GlyphSlotRec_&);

    public:
        Font() = default;
        Font(const std::string& filename, int height, int width, float line_height);
        Font(const Font& other) = delete;
        Font& operator=(const Font& other) = delete;
        Font(Font&& other) noexcept;
        Font& operator=(Font&& other) noexcept;
        ~Font();

        void renderText(
            const std::string& text,
            const glm::vec2& pos,
            const glm::vec4& color = glm::vec4(1),
            float angle = 0.0f,
            const glm::vec2& scl = glm::vec2(1.0f),
            float depth = 0.1f,
            TextAlignment = TextAlignment::Left,
            const glm::vec4& scissor = glm::vec4(-1.0f)
        );
        static void renderTextStatic(
            const std::string& text,
            const glm::vec2& pos,
            const glm::vec4& color = glm::vec4(1),
            float angle = 0.0f,
            const glm::vec2& scl = glm::vec2(1.0f),
            float depth = 0.1f,
            TextAlignment = TextAlignment::Left,
            const glm::vec4& scissor = glm::vec4(-1.0f)
        );

        float getTextWidth(std::string_view text) const;
        float getTextHeight(std::string_view text) const;
        float getTextHeightDynamic(std::string_view text) const;

        inline CONSTEXPR float getMaxHeight() const noexcept { return m_MaxHeight; }
        inline CONSTEXPR Texture* getGlyphTexture() const noexcept { return m_FontTexture; }
        inline CONSTEXPR float getLineHeight() const noexcept { return m_LineHeight; }

        const CharGlyph& getGlyphData(std::uint8_t character) const;
};
#endif
