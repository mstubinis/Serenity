#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Engine_Events.h"
#include "ShaderProgram.h"
#include "GBuffer.h"
#include "Camera.h"
#include "Light.h"
#include "Font.h"
#include "Engine_Physics.h"
#include "Scene.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "Skybox.h"
#include "Particles.h"

#include <glm/gtc/constants.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

bool Engine::Renderer::Detail::RenderManagement::m_DrawDebug = false;
bool Engine::Renderer::Detail::RenderManagement::m_Enabled_Bloom = true;
bool Engine::Renderer::Detail::RenderManagement::m_Enabled_SSAO = true;
Texture* Engine::Renderer::Detail::RenderManagement::RandomMapSSAO = nullptr;
GBuffer* Engine::Renderer::Detail::RenderManagement::m_gBuffer = nullptr;

std::vector<FontRenderInfo> _getRenderFontsDefault(){ std::vector<FontRenderInfo> k; return k; }
std::vector<FontRenderInfo> Engine::Renderer::Detail::RenderManagement::m_FontsToBeRendered = _getRenderFontsDefault();
std::vector<TextureRenderInfo> _getRenderTexturesDefault(){ std::vector<TextureRenderInfo> k; return k; }
std::vector<TextureRenderInfo> Engine::Renderer::Detail::RenderManagement::m_TexturesToBeRendered = _getRenderTexturesDefault();

void Engine::Renderer::Detail::RenderManagement::init(){
	Engine::Renderer::Detail::RenderManagement::RandomMapSSAO = new Texture("Textures/SSAONormal.png");
	Engine::Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void Engine::Renderer::Detail::RenderManagement::destruct(){
	delete Engine::Renderer::Detail::RenderManagement::m_gBuffer;
	delete Engine::Renderer::Detail::RenderManagement::RandomMapSSAO;
}
void Engine::Renderer::renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth){
	Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,color,glm::vec2(width,height),angle,depth));
}
void Engine::Renderer::Detail::RenderManagement::_renderTextures(){
	GLuint shader = Resources::getShader("Deferred_HUD")->getShaderProgram();
	glUseProgram(shader);
	for(auto item:m_TexturesToBeRendered){
		Texture* texture = nullptr;
		if(item.texture != ""){
			texture = Resources::Detail::ResourceManagement::m_Textures[item.texture];
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getTextureAddress());
			glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
			glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);
		}
		else{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
			glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),0);
		}
		glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);
		glUniform4f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z,item.col.w);

		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(item.pos.x,
												Resources::getWindowSize().y-item.pos.y,
												-0.5 - item.depth));
		model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
		if(item.texture != "")
			model = glm::scale(model, glm::vec3(texture->getWidth(),texture->getHeight(),1));
		model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));
		glm::mat4 world = Resources::getCamera("HUD")->getProjection() * model; //we dont want the view matrix as we want to assume this "World" matrix originates from (0,0,0)

		glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(model));

		Resources::getMesh("Plane")->render();
	}
	glUseProgram(0);
	m_TexturesToBeRendered.clear();
}
void Engine::Renderer::Detail::RenderManagement::_renderText(){
	GLuint shader = Resources::getShader("Deferred_HUD")->getShaderProgram();
	glUseProgram(shader);
	for(auto item:m_FontsToBeRendered){
		Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font->getFontData()->getGlyphTexture()->getTextureAddress());
		glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
		glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);
		glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);

		glUniform4f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z,item.col.w);

		float y_offset = 0;
		float x = item.pos.x;
		item.pos.y = Resources::getWindowSize().y - item.pos.y;
		for(auto c:item.text){
			if(c == '\n'){
				y_offset += (font->getFontData()->getGlyphData('X')->height+6) * item.scl.y;
				x = item.pos.x;
			}
			else{
				FontGlyph* glyph = font->getFontData()->getGlyphData(c);

				glyph->m_Model = glm::mat4(1);
				glyph->m_Model = glm::translate(glyph->m_Model, glm::vec3(x + glyph->xoffset ,item.pos.y - (glyph->height + glyph->yoffset) - y_offset,-0.5 - item.depth));
				glyph->m_Model = glm::rotate(glyph->m_Model, item.rot,glm::vec3(0,0,1));
				glyph->m_Model = glm::scale(glyph->m_Model, glm::vec3(item.scl.x,item.scl.y,1));
				glyph->m_World = Resources::getCamera("HUD")->getProjection() * glyph->m_Model; //we dont want the view matrix as we want to assume this "World" matrix originates from (0,0,0)

				glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(glyph->m_World));
				glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(glyph->m_Model));

				glyph->char_mesh->render();
				x += (glyph->xadvance) * item.scl.x;
			}
		}
	}
	glUseProgram(0);
	m_FontsToBeRendered.clear();
}
void Engine::Renderer::Detail::RenderManagement::_geometryPass(bool debug){
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,1,0,1);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	Scene* s = Resources::getCurrentScene();
	s->renderSkybox();

	for(auto object:s->getObjects()){
		object.second->render(debug);
	}

	//render particles
	//glDepthMask(GL_FALSE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//GLuint shader = Resources::getShader("Deferred")->getShaderProgram();
	//glUseProgram(shader);
	//for(auto emitter:s->getParticleEmitters()){
		//for(auto particle:emitter.second->getParticles()){
			//particle.render(shader);
		//}
	//}
	//glUseProgram(0);
	//glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);

	if(debug){
		GLuint shader = Resources::getShader("Deferred")->getShaderProgram();
		glUseProgram(shader);
		glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
		glUniform3f(glGetUniformLocation(shader, "Object_Color"),1,1,1);
		for(auto light:s->getLights()){
			light.second->renderDebug(shader);
		}
		glUseProgram(0);
	}
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}
void Engine::Renderer::Detail::RenderManagement::_lightingPass(){

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT);
	Engine::Renderer::Detail::RenderManagement::_passLighting();

}
void Engine::Renderer::Detail::RenderManagement::render(bool debug){

	m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_GLOW,BUFFER_TYPE_POSITION);
	Engine::Renderer::Detail::RenderManagement::_geometryPass(debug);
	m_gBuffer->stop();

	m_gBuffer->start(BUFFER_TYPE_LIGHTING);
	Engine::Renderer::Detail::RenderManagement::_lightingPass();
	m_gBuffer->stop();

	if(Engine::Renderer::Detail::RenderManagement::m_Enabled_SSAO){
		//only write to the G channel of the glow buffer for SSAO pass
		m_gBuffer->start(BUFFER_TYPE_GLOW,"G");
		Engine::Renderer::Detail::RenderManagement::_passSSAO(1.0f,2.9f,0.01f,0.7f,1.2f);
		m_gBuffer->stop();

		//only blurr the G channel (SSAO) of the glow buffer for the Gaussian blur pass
		m_gBuffer->start(BUFFER_TYPE_FREE1,"G");
		Engine::Renderer::Detail::RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_GLOW,1.0f,1.0f,"G");
		m_gBuffer->stop();
		m_gBuffer->start(BUFFER_TYPE_GLOW,"G");
		Engine::Renderer::Detail::RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE1,1.0f,1.0f,"G");
		m_gBuffer->stop();
	}
	if(Engine::Renderer::Detail::RenderManagement::m_Enabled_Bloom){
		glDisable(GL_BLEND);
		m_gBuffer->start(BUFFER_TYPE_BLOOM);
		Engine::Renderer::Detail::RenderManagement::_passBloom(BUFFER_TYPE_GLOW,BUFFER_TYPE_DIFFUSE);
		m_gBuffer->stop();

		m_gBuffer->start(BUFFER_TYPE_FREE1);
		Engine::Renderer::Detail::RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_BLOOM,0.42f,3.8f);
		m_gBuffer->stop();
		m_gBuffer->start(BUFFER_TYPE_BLOOM);
		Engine::Renderer::Detail::RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE1,0.42f,3.8f);
		m_gBuffer->stop();
	}
	Engine::Renderer::Detail::RenderManagement::_passFinal();

	glEnable(GL_BLEND);
	if(debug)
		Physics::Detail::PhysicsManagement::render();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Engine::Renderer::Detail::RenderManagement::_renderTextures();
	Engine::Renderer::Detail::RenderManagement::_renderText();
}
void Engine::Renderer::Detail::RenderManagement::_passLighting(){
	GLuint shader = Resources::getShader("Deferred_Light")->getShaderProgram();
	glm::vec3 camPos = Resources::getActiveCamera()->getPosition();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));
	glUniformMatrix4fv(glGetUniformLocation(shader, "VPInverse" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->calculateViewProjInverted()));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
	glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
	glUniform1i( glGetUniformLocation(shader,"gPositionMap"), 1 );

	for (auto light:Resources::getCurrentScene()->getLights()) {
		light.second->render(shader);
   	}

	// Reset OpenGL state
	for(unsigned int i = 0; i < 2; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passSSAO(unsigned int sampleCount, float intensity, float bias, float radius, float scale){
	//glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_SSAO")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));
	glUniform1f(glGetUniformLocation(shader,"gIntensity"), intensity);
	glUniform1f(glGetUniformLocation(shader,"gBias"), bias);
	glUniform1f(glGetUniformLocation(shader,"gRadius"),radius);
	glUniform1f(glGetUniformLocation(shader,"gScale"), scale);
	glUniform1i(glGetUniformLocation(shader,"gSampleCount"), sampleCount);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
	glUniform1i(glGetUniformLocation(shader,"gNormalMap"), 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
	glUniform1i(glGetUniformLocation(shader,"gPositionMap"), 1 );

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,RandomMapSSAO->getTextureAddress());
	glUniform1i(glGetUniformLocation(shader,"gRandomMap"), 2 );

	Engine::Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passEdge(GLuint texture, float radius){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Edge")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));
	glUniform1f(glGetUniformLocation(shader,"radius"), radius);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Engine::Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBloom(GLuint texture, GLuint texture1){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Bloom")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture1));
	glUniform1i(glGetUniformLocation(shader,"texture1"), 1 );

	Engine::Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	for(unsigned int i = 0; i < 2; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, i);
	}
	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBlur(std::string type, GLuint texture, float radius,float strengthModifier,std::string channels){
	GLuint shader = Resources::getShader("Deferred_Blur_" + type)->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));
	glUniform1f(glGetUniformLocation(shader,"radius"), radius);
	glUniform1f(glGetUniformLocation(shader,"strengthModifier"), strengthModifier);

	if(channels.find("R") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"R"), 1);
	else                                        glUniform1i(glGetUniformLocation(shader,"R"), 0);
	if(channels.find("G") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"G"), 1);
	else                                        glUniform1i(glGetUniformLocation(shader,"G"), 0);
	if(channels.find("B") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"B"), 1);
	else                                        glUniform1i(glGetUniformLocation(shader,"B"), 0);
	if(channels.find("A") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"A"), 1);
	else                                        glUniform1i(glGetUniformLocation(shader,"A"), 0);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Engine::Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passFinal(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Final")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindowSize().x),static_cast<float>(Resources::getWindowSize().y));
	glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
	glUniform3f(glGetUniformLocation(shader,"gAmbientColor"),ambient.x,ambient.y,ambient.z);

	glUniform1i( glGetUniformLocation(shader,"HasSSAO"), static_cast<int>(RenderManagement::m_Enabled_SSAO));
	glUniform1i( glGetUniformLocation(shader,"HasBloom"), static_cast<int>(RenderManagement::m_Enabled_Bloom));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE));
	glUniform1i( glGetUniformLocation(shader,"gColorMap"), 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
	glUniform1i( glGetUniformLocation(shader,"gLightMap"), 1 );

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
	glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 2 );

	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_GLOW));
	glUniform1i( glGetUniformLocation(shader,"gGlowMap"), 3 );

	glActiveTexture(GL_TEXTURE4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_BLOOM));
	glUniform1i( glGetUniformLocation(shader,"gBloomMap"), 4 );

	Engine::Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	for(unsigned int i = 0; i < 5; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}