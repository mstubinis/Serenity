#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

class Texture;
struct FT_GlyphSlotRec_;

#include <serenity/resources/Resource.h>
#include <serenity/resources/Handle.h>
#include <serenity/resources/font/FontIncludes.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <serenity/system/TypeDefs.h>
#include <serenity/dependencies/glm.h>

struct CharGlyph final {
    // mesh specific
    std::vector<glm::vec3> pts;
    std::vector<glm::vec2> uvs;

    uint32_t  char_id   = 0;
    uint32_t  x         = 0;
    uint32_t  y         = 0;
    uint32_t  width     = 0;
    uint32_t  height    = 0;
    uint32_t  xadvance  = 0;
    int       xoffset   = 0;
    int       yoffset   = 0;
};
class Font final: public Resource<Font> {
    public:
        static constexpr uint32_t MAX_CHARACTERS_RENDERED_PER_FRAME    = 4096;
    private:
        std::vector<CharGlyph>                   m_CharGlyphs;
        Handle                                   m_FontTexture         = {};
        float                                    m_MaxHeight           = 0.0f;
        float                                    m_LineHeight          = 8.0f;

        void init_simple(const std::string& filename, int height, int width);
        void init_freetype(const std::string& filename, int height, int width);
        void init(const std::string& filename, int height, int width);
    public:
        Font() = default;
        Font(const std::string& filename, int height, int width, float line_height);

        Font(const Font&)                = delete;
        Font& operator=(const Font&)     = delete;
        Font(Font&&) noexcept;
        Font& operator=(Font&&) noexcept;

        [[nodiscard]] float getTextWidth(std::string_view text) const;
        [[nodiscard]] float getTextHeight(std::string_view text) const;
        [[nodiscard]] float getTextHeightDynamic(std::string_view text) const;

        [[nodiscard]] inline constexpr float getMaxHeight() const noexcept { return m_MaxHeight; }
        [[nodiscard]] inline constexpr Handle getGlyphTexture() const noexcept { return m_FontTexture; }
        [[nodiscard]] inline constexpr float getLineHeight() const noexcept { return m_LineHeight; }

        [[nodiscard]] const CharGlyph& getGlyphData(uint8_t character) const;
};
#endif
