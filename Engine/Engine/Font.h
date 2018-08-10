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

//class Mesh;
class Texture;
typedef unsigned int uint;
typedef unsigned char uchar;
struct FontGlyph final{
     uint id;
     uint x; uint y;
     uint width; uint height;
     int xoffset; int yoffset;
     uint xadvance;
     glm::mat4 m_Model;

	 //mesh specific
	 std::vector<glm::vec3> pts;
	 std::vector<glm::vec2> uvs;
};
class Font final: public EngineResource{
    private:
		class impl; std::unique_ptr<impl> m_i;
    public:
        Font(std::string);
        ~Font();
        void renderText(std::string text,glm::vec2& pos,glm::vec4 color = glm::vec4(1),float angle = 0,glm::vec2 scl = glm::vec2(1),float depth = 0.1);
        Texture* getGlyphTexture();
		//Mesh* getFontMesh();
        FontGlyph* getGlyphData(uchar);
};
#endif
