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
class Object;

struct GeometryRenderInfo{
	Object* object;
	Mesh* mesh;
	Material* material;
	GLuint shader;
	GeometryRenderInfo(Object* _object,Mesh* _mesh, Material* _material, GLuint _shader){
		object = _object;
		mesh = _mesh;
		material = _material;
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
					static std::vector<GeometryRenderInfo> m_ObjectsToBeRendered;
					static std::vector<GeometryRenderInfo> m_ForegroundObjectsToBeRendered;

					static void _renderObjects();
					static void _renderForegroundObjects();
					static void _renderText();
					static void _renderTextures();

					static void _geometryPass();
					static void _lightingPass();

					static void _passLighting();
					static void _passSSAO(unsigned int sampleCount, float intensity, float bias, float radius, float scale);
					static void _passEdge(GLuint texture,float radius = 1.0f);
					static void _passBloom(GLuint texture,GLuint texture1);
					static void _passBlur(std::string type,GLuint texture,float radius = 1.0f,float strengthModifier = 1.0f,std::string channels = "RGBA");
					static void _passFinal();
				public:
					static GBuffer* m_gBuffer;
					static Texture* RandomMapSSAO;
					static bool m_DrawDebug;

					static bool m_Enabled_Bloom;
					static bool m_Enabled_SSAO;

					static void render();

					static void init();
					static void destruct();

					static std::vector<GeometryRenderInfo>& getForegroundObjectRenderQueue(){ return m_ForegroundObjectsToBeRendered; }
					static std::vector<GeometryRenderInfo>& getObjectRenderQueue(){ return m_ObjectsToBeRendered; }
					static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
					static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
			};
			static void renderFullscreenQuad(unsigned int width, unsigned int height){
				//Projection setup
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glOrtho(0,width,0,height,0.1f,2);	
	
				//Model setup
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();

				// Render the quad
				glLoadIdentity();
				glColor3f(1,1,1);
				glTranslatef(0,0,-1);
	
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(0,0,0);
				glTexCoord2f(1,0);
				glVertex3f((float)width,0,0);
				glTexCoord2f(1,1);
				glVertex3f((float)width,(float)height,0);
				glTexCoord2f(0,1);
				glVertex3f(0,(float)height,0);
				glEnd();

				//Reset the matrices	
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			}
		};
		static bool isDebug(){ return Detail::RenderManagement::m_DrawDebug; }
		static void drawDebug(bool b){ Detail::RenderManagement::m_DrawDebug = b; }

		void renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth);
	};
};
#endif