#ifndef FONT_H
#define FONT_H
#include <GL/glew.h>
#include <GL/GL.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class Mesh;

struct FontGlyph {
	 unsigned int id;
	 unsigned int x;
	 unsigned int y;
	 unsigned int width;
	 unsigned int height;
	 int xoffset;
	 int yoffset;
	 unsigned int xadvance;
	 Mesh* char_mesh;
	 glm::mat4 m_Model;
	 glm::mat4 m_World;
};
class FontData{
	private:
		GLuint Font_Texture;
		std::unordered_map<unsigned char,FontGlyph*> Font_Glyphs;
		void LoadTextFile(std::string filename);
	public:
		FontData(std::string filename);
		~FontData();

		GLuint Get_Glyph_Texture();
		FontGlyph* Get_Glyph_Data(unsigned char);
};
class Font{
	private:
		FontData* m_FontData;
	public:
		Font(std::string);
		~Font();

		void RenderText(std::string text, 
			            glm::vec2& pos, 
						glm::vec3 color, 
						float angle = 0, 
						glm::vec2 scl = glm::vec2(1,1), 
						float depth = 0
					    );
};
#endif