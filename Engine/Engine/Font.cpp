#include "Font.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Engine;

FontData::FontData(std::string filename){
	_loadTextFile(filename);
	std::string texture_filename = filename.substr(0,filename.size()-4);
	texture_filename += "_0.png";
	m_FontTexture = new Texture(texture_filename);
}
FontData::~FontData(){
	for(auto glyph:m_FontGlyphs){
		delete glyph.second->char_mesh;
		delete glyph.second;
	}
}
FontGlyph* FontData::getGlyphData(unsigned char c){ return m_FontGlyphs[c]; }
void FontData::_loadTextFile(std::string filename){
	std::unordered_map<unsigned char,FontGlyph*> _Font_Chars;
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
	for(std::string line; std::getline(str, line, '\n');){
		if(line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' '){
			FontGlyph* font = new FontGlyph();
			std::string token;
			std::istringstream stream(line);
			while(std::getline(stream, token, ' ')) {
				size_t pos = token.find("=");

				std::string key = token.substr(0, pos);
				std::string value = token.substr(pos + 1, std::string::npos);

				if(key == "id")            font->id = stoi(value);
				else if(key == "x")        font->x = stoi(value);
				else if(key == "y")        font->y = stoi(value);
				else if(key == "width")    font->width = stoi(value);
				else if(key == "height")   font->height = stoi(value); 
				else if(key == "xoffset")  font->xoffset = stoi(value);
				else if(key == "yoffset")  font->yoffset = stoi(value);
				else if(key == "xadvance") font->xadvance = stoi(value);
			}
			font->m_Model = glm::mat4(1);
			font->m_World = glm::mat4(1);
			font->char_mesh = new Mesh(font->x,font->y,font->width,font->height);

			_Font_Chars[font->id] = font;
		}
	}
	m_FontGlyphs = _Font_Chars;
}

Font::Font(std::string filename){
	m_FontData = new FontData(filename);

	m_Name = filename.substr(0,filename.size()-4);
	Resources::Detail::ResourceManagement::m_Fonts[m_Name] = this;
}
Font::~Font(){
	delete m_FontData;
}
void Font::renderText(std::string text, glm::vec2& pos, glm::vec3 color,float angle, glm::vec2 scl, float depth){
	renderer->getFontRenderQueue().push_back(FontRenderInfo(m_Name,text,pos,color,scl,angle,depth));
}