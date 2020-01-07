#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/engine/fonts/FontIncludes.h>

class Texture;
struct FontGlyph final{
     unsigned int id, x, y, width, height, xadvance;
     int xoffset; int yoffset;

     //mesh specific
     std::vector<glm::vec3> pts;
     std::vector<glm::vec2> uvs;
};
class Font final: public EngineResource{
    public:
        static const unsigned int MAX_CHARACTERS_RENDERED_PER_FRAME = 4096;
    private:
        Texture* m_FontTexture;
        float    m_MaxHeight;
        std::unordered_map<unsigned char, FontGlyph> m_FontGlyphs;
    public:
        Font(const std::string& filename);
        ~Font();

        void renderText(
            const std::string& text,
            const glm::vec2& pos,
            const glm::vec4& color = glm::vec4(1),
            const float& angle = 0.0f,
            const glm::vec2 & scl = glm::vec2(1.0f),
            const float& depth = 0.1f,
            const TextAlignment::Type & = TextAlignment::Left,
            const glm::vec4& scissor = glm::vec4(-1.0f)
        );
        const float getTextWidth(const std::string& text) const;
        const float getTextHeight(const std::string& text) const;

        const float& getMaxHeight() const;

        const Texture& getGlyphTexture() const;
        const FontGlyph& getGlyphData(const unsigned char& character) const;
};
#endif
