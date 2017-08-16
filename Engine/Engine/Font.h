#pragma once
#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H

#include "Engine_ResourceBasic.h"

#include <GLM/gtc/matrix_transform.hpp>
#include <string>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Mesh;
class Texture;
typedef unsigned int uint;
typedef unsigned char uchar;
struct FontGlyph final{
     uint id;
     uint x;
     uint y;
     uint width;
     uint height;
     int xoffset;
     int yoffset;
     uint xadvance;
     Mesh* char_mesh;
     glm::mat4 m_Model;
};
class FontData final{
    private:
        Texture* m_FontTexture;
        std::unordered_map<uchar,FontGlyph*> m_FontGlyphs;
        void _loadTextFile(std::string& filename);
    public:
        FontData(std::string& filename);
        ~FontData();
        Texture* getGlyphTexture() { return m_FontTexture; }
        FontGlyph* getGlyphData(uchar);
};
class Font final: public EngineResource{
    private:
        FontData* m_FontData;
    public:
        Font(std::string);
        ~Font();
        void renderText(std::string& text,glm::vec2& pos,glm::vec4 color = glm::vec4(1),float angle = 0,glm::vec2 scl = glm::vec2(1),float depth = 0);
        FontData* getFontData() { return m_FontData; }
};
#endif
