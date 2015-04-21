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

#include <glm/gtc/constants.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

bool Engine::Renderer::Detail::RenderManagement::m_DrawDebug = false;
Texture* Engine::Renderer::Detail::RenderManagement::RandomMapSSAO = nullptr;
GBuffer* Engine::Renderer::Detail::RenderManagement::m_gBuffer = nullptr;

std::vector<FontRenderInfo> _getRenderFontsDefault(){ std::vector<FontRenderInfo> k; return k; }
std::vector<FontRenderInfo> Engine::Renderer::Detail::RenderManagement::m_FontsToBeRendered = _getRenderFontsDefault();
std::vector<TextureRenderInfo> _getRenderTexturesDefault(){ std::vector<TextureRenderInfo> k; return k; }
std::vector<TextureRenderInfo> Engine::Renderer::Detail::RenderManagement::m_TexturesToBeRendered = _getRenderTexturesDefault();

void Engine::Renderer::Detail::RenderManagement::init(){
	Engine::Renderer::Detail::RenderManagement::RandomMapSSAO = new Texture("Textures/SSAONormal.png");
	Engine::Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);
}
void Engine::Renderer::Detail::RenderManagement::destruct(){
	delete Engine::Renderer::Detail::RenderManagement::m_gBuffer;
	delete Engine::Renderer::Detail::RenderManagement::RandomMapSSAO;
}

void Engine::Renderer::Detail::RenderManagement::_renderTextures(){
	GLuint shader = Resources::getShader("Deferred_HUD")->getShaderProgram();
	glUseProgram(shader);
	for(auto item:m_TexturesToBeRendered){
		Texture* texture = Resources::Detail::ResourceManagement::m_Textures[item.texture];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->getTextureAddress());
		glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
		glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);
		glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);

		glUniform3f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z);

		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(item.pos.x,Resources::getWindow()->getSize().y-item.pos.y,-0.5 - item.depth));
		model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
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

		glUniform3f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z);

		float y_offset = 0;
		float x = item.pos.x;
		item.pos.y = Resources::getWindow()->getSize().y - item.pos.y;
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
	
	for(auto object:Resources::getCurrentScene()->getObjects()){
		object.second->render(debug);
	}	
	if(debug){
		physicsEngine->render();
		GLuint shader = Resources::getShader("Deferred")->getShaderProgram();
		glUseProgram(shader);
		for(auto light:Resources::getCurrentScene()->getLights()){
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

	m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_POSITION);
	Engine::Renderer::Detail::RenderManagement::_geometryPass(debug);
	m_gBuffer->stop();

	m_gBuffer->start(BUFFER_TYPE_LIGHTING);
	Engine::Renderer::Detail::RenderManagement::_lightingPass();
	m_gBuffer->stop();

	m_gBuffer->start(BUFFER_TYPE_SSAO);
	Engine::Renderer::Detail::RenderManagement::_passSSAO();
	m_gBuffer->stop();
	m_gBuffer->start(BUFFER_TYPE_FREE1);
	Engine::Renderer::Detail::RenderManagement::_passBlurHorizontal(m_gBuffer->getTexture(BUFFER_TYPE_SSAO));
	m_gBuffer->stop();
	m_gBuffer->start(BUFFER_TYPE_SSAO);
	Engine::Renderer::Detail::RenderManagement::_passBlurVertical(m_gBuffer->getTexture(BUFFER_TYPE_FREE1));
	m_gBuffer->stop();

	Engine::Renderer::Detail::RenderManagement::_passFinal();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Engine::Renderer::Detail::RenderManagement::_renderTextures();
	Engine::Renderer::Detail::RenderManagement::_renderText();
}
void Engine::Renderer::Detail::RenderManagement::_passLighting(){
	GLuint shader = Resources::getShader("Deferred_Light")->getShaderProgram();
	glm::vec3 camPos = Resources::getActiveCamera()->getPosition();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));
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

void Engine::Renderer::Detail::RenderManagement::_passSSAO(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_SSAO")->getShaderProgram();
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VPInverse" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->calculateViewProjInverted()));

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));
	glUniform1f(glGetUniformLocation(shader,"gIntensity"), 2.9f);
	glUniform1f(glGetUniformLocation(shader,"gBias"), 0.01f);
	glUniform1f(glGetUniformLocation(shader,"gRadius"), 0.7f);
	glUniform1f(glGetUniformLocation(shader,"gScale"), 1.2f);

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

	Engine::Renderer::Detail::RenderManagement::_initQuad();

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBlurHorizontal(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Blur_Horizontal")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Engine::Renderer::Detail::RenderManagement::_initQuad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBlurVertical(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Blur_Vertical")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Engine::Renderer::Detail::RenderManagement::_initQuad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passFinal(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Final")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));

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
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_SSAO));
	glUniform1i( glGetUniformLocation(shader,"gSSAOMap"), 2 );

	Engine::Renderer::Detail::RenderManagement::_initQuad();

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_initQuad(){
	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,Resources::getWindow()->getSize().x,0,Resources::getWindow()->getSize().y,0.1f,2);	
	
	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Render the quad
	glLoadIdentity();
	glColor3f(1,1,1);
	glTranslatef(0,0,-1.0);
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0, 0 );
	glVertex3f(    0.0f, 0.0f, 0.0f);
	glTexCoord2f( 1, 0 );
	glVertex3f(   (float) Resources::getWindow()->getSize().x, 0.0f, 0.0f);
	glTexCoord2f( 1, 1 );
	glVertex3f(   (float) Resources::getWindow()->getSize().x, (float) Resources::getWindow()->getSize().y, 0.0f);
	glTexCoord2f( 0, 1 );
	glVertex3f(    0.0f,  (float) Resources::getWindow()->getSize().y, 0.0f);
	glEnd();

	//Reset the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}