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

typedef unsigned char uchar;

struct FontGlyph final{
     uint id;
     uint x; uint y;
     uint width; uint height;
     int xoffset; int yoffset;
     uint xadvance;

     //mesh specific
     std::vector<glm::vec3> pts;
     std::vector<glm::vec2> uvs;
};
class Font final: public EngineResource{
    public:
        static const uint MAX_CHARACTERS_RENDERED_PER_FRAME = 4096;
    private:
        Texture* m_FontTexture;
        std::unordered_map<uchar, FontGlyph> m_FontGlyphs;
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

        const Texture& getGlyphTexture() const;
        const FontGlyph& getGlyphData(const uchar& character) const;
};
#endif
