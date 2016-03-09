#ifndef ENGINE_FONT_H
#define ENGINE_FONT_H
#include <GL/glew.h>
#include <GL/GL.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class Mesh;
class Texture;

struct FontGlyph final{
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
class FontData final{
	private:
		Texture* m_FontTexture;
		std::unordered_map<unsigned char,FontGlyph*> m_FontGlyphs;
		void _loadTextFile(std::string filename);
	public:
		FontData(std::string filename);
		~FontData();

		Texture* getGlyphTexture() { return m_FontTexture; }
		FontGlyph* getGlyphData(unsigned char);
};
class Font final{
	private:
		FontData* m_FontData;
		std::string m_Name;
	public:
		Font(std::string);
		~Font();

		void renderText(std::string text, 
			            glm::vec2& pos, 
						glm::vec4 color, 
						float angle = 0, 
						glm::vec2 scl = glm::vec2(1,1), 
						float depth = 0
					    );

		FontData* getFontData() { return m_FontData; }
		const std::string getName() const { return m_Name; }
};
#endif