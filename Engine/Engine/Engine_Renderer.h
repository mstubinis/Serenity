#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <vector>

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
struct FontRenderInfo: public TextureRenderInfo{
	std::string text;
	FontRenderInfo():TextureRenderInfo(){
		text = "";
	}
	FontRenderInfo(std::string _font, std::string _text, glm::vec2 _pos, glm::vec3 _col, glm::vec2 _scl, float _rot, float _depth):TextureRenderInfo(_font,_pos,_col,_scl,_rot,_depth){
		text = _text;
	}
};

class GBuffer;
class Texture;

namespace Engine{
	namespace Renderer{
		namespace Detail{
			class RenderManagement{
				private:
					static Texture* RandomMapSSAO;
					static GBuffer* m_gBuffer;

					static std::vector<FontRenderInfo> m_FontsToBeRendered;
					static std::vector<TextureRenderInfo> m_TexturesToBeRendered;

					static void _initQuad();

					static void _renderText();
					static void _renderTextures();

					static void _geometryPass(bool debug);
					static void _lightingPass();

					static void _passLighting();
					static void _passSSAO();
					static void _passBlurHorizontal(GLuint texture);
					static void _passBlurVertical(GLuint texture);
					static void _passFinal();
				public:
					static void render(bool debug=false);

					static void init();
					static void destruct();

					static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
					static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }

			};
		};
	};
};
#endif