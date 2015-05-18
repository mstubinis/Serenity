#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <unordered_map>

class GBuffer;
class Texture;
class Mesh;
class Material;

struct TextureRenderInfo{
	std::string texture;
	glm::vec2 pos;
	glm::vec4 col;
	glm::vec2 scl;
	float rot;
	float depth;
	TextureRenderInfo(){
		texture = ""; pos = scl = glm::vec2(0,0); col = glm::vec4(1,1,1,1); rot = depth = 0;
	}
	TextureRenderInfo(std::string _texture, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth){
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
	FontRenderInfo(std::string _font, std::string _text, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth):TextureRenderInfo(_font,_pos,_col,_scl,_rot,_depth){
		text = _text;
	}
};

namespace Engine{
	namespace Renderer{
		namespace Detail{
			class RenderManagement{
				private:
					static std::vector<FontRenderInfo> m_FontsToBeRendered;
					static std::vector<TextureRenderInfo> m_TexturesToBeRendered;

					static void _initQuad(unsigned int width, unsigned int height);

					static void _renderText();
					static void _renderTextures();

					static void _geometryPass(bool debug);
					static void _lightingPass();

					static void _passLighting();
					static void _passSSAO(unsigned int sampleCount, float intensity, float bias, float radius, float scale);
					static void _passEdge(GLuint texture,float radius = 1.0f);
					static void _passBloom(GLuint texture,GLuint texture1);
					static void _passBlurHorizontal(GLuint texture,float radius = 1.0f,float strengthModifier = 1.0f,std::string channels = "RGBA");
					static void _passBlurVertical(GLuint texture, float radius = 1.0f,float strengthModifier = 1.0f,std::string channels = "RGBA");
					static void _passFinal();
				public:
					static GBuffer* m_gBuffer;
					static Texture* RandomMapSSAO;
					static bool m_DrawDebug;
					static void render(bool debug=false);

					static void init();
					static void destruct();

					static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
					static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
			};
		};
		static bool isDebug(){ return Detail::RenderManagement::m_DrawDebug; }
		static void drawDebug(bool b){ Detail::RenderManagement::m_DrawDebug = b; }

		void renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth);
	};
};
#endif