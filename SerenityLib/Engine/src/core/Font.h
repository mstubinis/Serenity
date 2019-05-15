#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
    private:
        Texture* m_FontTexture;
        std::unordered_map<uchar, FontGlyph> m_FontGlyphs;
    public:
        Font(std::string);
        ~Font();
        void renderText(std::string text,glm::vec2 pos,glm::vec4 color = glm::vec4(1),float angle = 0.0f,glm::vec2 scl = glm::vec2(1.0f),float depth = 0.1f);
        Texture& getGlyphTexture();
        FontGlyph& getGlyphData(uchar);
};
#endif
