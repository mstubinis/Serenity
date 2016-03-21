#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Engine_Resources.h"

class GBuffer;

struct GeometryRenderInfo final{
	Object* object;
	GLuint shader;
	GeometryRenderInfo(Object* _object, GLuint _shader){
		object = _object;
		shader = _shader;
	}
};
struct TextureRenderInfo{
	std::string texture;
	glm::vec2 pos;
	glm::vec4 col;
	glm::vec2 scl;
	float rot;
	float depth;
	TextureRenderInfo(){
		texture = ""; pos = scl = glm::vec2(0); col = glm::vec4(1); rot = depth = 0;
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
struct FontRenderInfo final: public TextureRenderInfo{
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
		struct RendererInfo final{
			static bool ssao;
			static unsigned int ssao_samples;
			static float ssao_bias;
			static float ssao_scale;
			static float ssao_radius;
			static float ssao_intensity;

			static bool bloom;
			static bool lighting;
			static bool debug;
		};
		namespace Settings{
			static void setSSAOIntensity(float i){ Renderer::RendererInfo::ssao_intensity = i; }
			static void setSSAORadius(float r){ Renderer::RendererInfo::ssao_radius = r; }
			static void setSSAOScale(float s){ Renderer::RendererInfo::ssao_scale = s; }
			static void setSSAOBias(float b){ Renderer::RendererInfo::ssao_bias = b; }
			static void setSSAOSamples(unsigned int s){ Renderer::RendererInfo::ssao_samples = s; }

			static void enableLighting(bool enabled = true){ Renderer::RendererInfo::lighting = enabled; }
			static void enableBloom(bool enabled = true){ Renderer::RendererInfo::bloom = enabled; }
			static void enableSSAO(bool enabled = true){ Renderer::RendererInfo::ssao = enabled;  }
			static void enableDebugDrawing(bool enabled = true){ Renderer::RendererInfo::debug = enabled;  }
		};
		namespace Detail{
			class RenderManagement final{
				private:
					static std::vector<FontRenderInfo> m_FontsToBeRendered;
					static std::vector<TextureRenderInfo> m_TexturesToBeRendered;
					static std::vector<GeometryRenderInfo> m_ObjectsToBeRendered;
					static std::vector<GeometryRenderInfo> m_ForegroundObjectsToBeRendered;

					static void _renderObjects();
					static void _renderForegroundObjects();
					static void _renderText();
					static void _renderTextures();

					static void _geometryPass();
					static void _lightingPass();

					static void _passSSAO();
					static void _passEdge(GLuint texture,float radius = 1.0f);
					static void _passBloom(GLuint texture,GLuint texture1);
					static void _passBlur(std::string type,GLuint texture,float radius = 1.0f,float strengthModifier = 1.0f,std::string channels = "RGBA");
					static void _passFinal();
				public:
					static GBuffer* m_gBuffer;
					static Texture* RandomMapSSAO;
					static glm::mat4 m_2DProjectionMatrix;

					static void render();
					static void hardClear();

					static void init();
					static void initOpenGL();
					static void destruct();

					static std::vector<GeometryRenderInfo>& getForegroundObjectRenderQueue(){ return m_ForegroundObjectsToBeRendered; }
					static std::vector<GeometryRenderInfo>& getObjectRenderQueue(){ return m_ObjectsToBeRendered; }
					static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
					static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
			};
			static void renderFullscreenQuad(unsigned int width, unsigned int height,float scale = 1.0f){
				//Projection setup
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glOrtho(0,Engine::Resources::getWindowSize().x,0,Engine::Resources::getWindowSize().y,0.1f,2);	
	
				//Model setup
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();

				// Render the quad
				glLoadIdentity();
				glColor3f(1,1,1);
				glTranslatef(0,0,-1);
	
				//glScalef(scale,scale,1.0f);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(0,0,0);
				glTexCoord2f(0,0);
				glVertex3f((float)width*scale,0,0);
				glTexCoord2f(0,0);
				glVertex3f((float)width*scale,(float)height*scale,0);
				glTexCoord2f(0,0);
				glVertex3f(0,(float)height*scale,0);
				glEnd();

				//Reset the matrices	
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			}
		};
		void renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth);
	};
};
#endif