#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <vector>

struct FontRenderInfo{
	std::string font;
	std::string text;
	glm::vec2 pos;
	glm::vec3 col;
	glm::vec2 scl;
	float rot;
	float depth;
	FontRenderInfo(){
		text = font = ""; pos = scl = glm::vec2(0,0); col = glm::vec3(1,1,1); rot = depth = 0;
	}
	FontRenderInfo(std::string _font, std::string _text, glm::vec2 _pos, glm::vec3 _col, glm::vec2 _scl, float _rot, float _depth){
		font = _font;
		text = _text;
		pos = _pos;
		col = _col;
		scl = _scl;
		rot = _rot;
		depth = _depth;
	}
};
struct TextureRenderInfo{
	std::string texture;
	glm::vec2 pos;
	glm::vec3 col;
	glm::vec2 scl;
	float rot;
	float depth;
	TextureRenderInfo(){
		texture = ""; pos = scl = glm::vec2(0,0); col = glm::vec3(1,1,1); rot = depth = 0;
	}
	TextureRenderInfo(std::string _texture, glm::vec2 _pos, glm::vec3 _col, glm::vec2 _scl, float _rot, float _depth){
		texture = _texture;
		pos = _pos;
		col = _col;
		scl = _scl;
		rot = _rot;
		depth = _depth;
	}
};

class Object;
class GBuffer;
class Font;
class Texture;
class Renderer{
	private:
		std::vector<FontRenderInfo> m_FontsToBeRendered;
		std::vector<TextureRenderInfo> m_TexturesToBeRendered;

		Texture* RandomMapSSAO;
		GBuffer* m_gBuffer;
		Font* m_Font;

		void _initQuad();

		void _renderText();
		void _renderTextures();

		void _geometryPass(bool debug);
		void _lightingPass();

		void _passLighting();
		void _passSSAO();
		void _passBlurHorizontal(GLuint texture);
		void _passBlurVertical(GLuint texture);
		void _passFinal();

	public:
		Renderer();
		~Renderer();

		void render(bool debug = false);

		std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
		std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
};
extern Renderer* renderer;
#endif