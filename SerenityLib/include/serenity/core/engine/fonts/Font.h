#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

class Texture;
struct FT_GlyphSlotRec_;

#include <serenity/core/engine/resources/Resource.h>
#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/fonts/FontIncludes.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <serenity/core/engine/system/TypeDefs.h>
#include <serenity/core/engine/dependencies/glm.h>

struct CharGlyph final {
    uint32_t  char_id   = 0;
    uint32_t  x         = 0;
    uint32_t  y         = 0;
    uint32_t  width     = 0;
    uint32_t  height    = 0;
    uint32_t  xadvance  = 0;
    int       xoffset   = 0;
    int       yoffset   = 0;

    //mesh specific
    std::vector<glm::vec3> pts;
    std::vector<glm::vec2> uvs;
};
class Font final: public Resource {
    public:
        static constexpr unsigned int MAX_CHARACTERS_RENDERED_PER_FRAME    = 4096;
    private:
        Handle                                        m_FontTexture    = Handle{};
        float                                         m_MaxHeight      = 0.0f;
        float                                         m_LineHeight     = 8.0f;
        std::unordered_map<uint8_t, CharGlyph>   m_CharGlyphs;

        void init_simple(const std::string& filename, int height, int width);
        void init_freetype(const std::string& filename, int height, int width);
        void init(const std::string& filename, int height, int width);

        std::vector<std::vector<uint8_t>> generate_bitmap(const FT_GlyphSlotRec_&);

    public:
        Font() = default;
        Font(const std::string& filename, int height, int width, float line_height);
        Font(const Font& other) = delete;
        Font& operator=(const Font& other) = delete;
        Font(Font&& other) noexcept;
        Font& operator=(Font&& other) noexcept;
        ~Font();

        float getTextWidth(std::string_view text) const;
        float getTextHeight(std::string_view text) const;
        float getTextHeightDynamic(std::string_view text) const;

        inline constexpr float getMaxHeight() const noexcept { return m_MaxHeight; }
        inline constexpr Handle getGlyphTexture() const noexcept { return m_FontTexture; }
        inline constexpr float getLineHeight() const noexcept { return m_LineHeight; }

        const CharGlyph& getGlyphData(uint8_t character) const;
};
#endif
