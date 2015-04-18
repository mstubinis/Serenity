#include "Font.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Engine;

FontData::FontData(std::string filename){
	LoadTextFile(filename);
	std::string texture_filename = filename.substr(0,filename.size()-4);
	texture_filename += "_0.png";
	Resources::loadTextureIntoGLuint(Font_Texture,texture_filename);
}
FontData::~FontData(){
	for(auto glyph:Font_Glyphs){
		delete glyph.second->char_mesh;
		delete glyph.second;
	}
}
FontGlyph* FontData::Get_Glyph_Data(unsigned char c){ return Font_Glyphs[c]; }
GLuint FontData::Get_Glyph_Texture(){ return Font_Texture; }
void FontData::LoadTextFile(std::string filename){
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
	Font_Glyphs = _Font_Chars;
}

Font::Font(std::string filename){
	m_FontData = new FontData(filename);
}
Font::~Font(){
	delete m_FontData;
}
void Font::RenderText(std::string text, glm::vec2& pos, glm::vec3 color,float angle, glm::vec2 scl, float depth){
	GLuint shader = Resources::getShader("Deferred_HUD")->Get_Shader_Program();
	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FontData->Get_Glyph_Texture());
	glUniform1i(glGetUniformLocation(shader,"DiffuseMap"), 0);
	glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"), 1);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),1);

	glUseProgram(shader );
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),color.x,color.y,color.z);

	float y_offset = 0;
	float x = pos.x;
	pos.y = Resources::getWindow()->getSize().y - pos.y;
	for(auto c:text){
		if(c == '\n'){
			y_offset += m_FontData->Get_Glyph_Data('X')->height+4;
			x = pos.x;
		}
		else{
			FontGlyph* glyph = m_FontData->Get_Glyph_Data(c);

			glyph->m_Model = glm::mat4(1);
			glyph->m_Model = glm::translate(glyph->m_Model, glm::vec3(x + glyph->xoffset ,pos.y - (glyph->height + glyph->yoffset) - y_offset,-0.5 - depth));
			glyph->m_Model = glm::rotate(glyph->m_Model, angle,glm::vec3(0,0,1));
			glyph->m_Model = glm::scale(glyph->m_Model, glm::vec3(scl.x,scl.y,1));
			glyph->m_World = Resources::getCamera("HUD")->Projection() * glyph->m_Model; //we dont want the view matrix as we want to assume this "World" matrix originates from (0,0,0)

			glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(glyph->m_World));
			glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(glyph->m_Model));

			glyph->char_mesh->Render();
			x += glyph->xadvance;
		}
	}
	glUseProgram(0);
}