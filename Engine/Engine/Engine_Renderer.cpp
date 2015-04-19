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

#include <glm/gtc/constants.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

Renderer::Renderer(){
	Resources::loadTextureIntoGLuint(RandomMapSSAO,"Textures/SSAONormal.png");

	glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	glDepthMask(GL_TRUE); glEnable(GL_DEPTH_TEST);

	m_gBuffer = new GBuffer(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);

	m_Font = new Font("Fonts/consolas.fnt");
}
Renderer::~Renderer(){
	delete m_gBuffer;
	delete m_Font;
}
void Renderer::_geometryPass(bool debug){
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
void Renderer::_lightingPass(){
	glEnable(GL_BLEND);
   	glBlendEquation(GL_FUNC_ADD);
   	glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT);
	this->_passLighting();
}
void Renderer::render(bool debug){

	m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_POSITION);
	this->_geometryPass(debug);
	m_gBuffer->stop();

	m_gBuffer->start(BUFFER_TYPE_LIGHTING);
	this->_lightingPass();
	m_gBuffer->stop();

	m_gBuffer->start(BUFFER_TYPE_SSAO);
	this->_passSSAO();
	m_gBuffer->stop();
	m_gBuffer->start(BUFFER_TYPE_FREE1);
	this->_passBlurHorizontal(m_gBuffer->getTexture(BUFFER_TYPE_SSAO));
	m_gBuffer->stop();
	m_gBuffer->start(BUFFER_TYPE_SSAO);
	this->_passBlurVertical(m_gBuffer->getTexture(BUFFER_TYPE_FREE1));
	m_gBuffer->stop();

	this->_passFinal();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//render hud stuff
	glm::vec3 pos = Resources::getCurrentScene()->getLights().begin()->second->getScreenCoordinates();
	m_Font->RenderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +
		               "\nFPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())),glm::vec2(pos.x,pos.y),glm::vec3(1,1,1),0);
}
void Renderer::_passLighting(){
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

void Renderer::_passSSAO(){
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
	glBindTexture(GL_TEXTURE_2D,RandomMapSSAO);
	glUniform1i(glGetUniformLocation(shader,"gRandomMap"), 2 );

	_initQuad();

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::_passBlurHorizontal(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Blur_Horizontal")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	_initQuad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Renderer::_passBlurVertical(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources::getShader("Deferred_Blur_Vertical")->getShaderProgram();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Resources::getWindow()->getSize().x),static_cast<float>(Resources::getWindow()->getSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	_initQuad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Renderer::_passFinal(){
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

	_initQuad();

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::_initQuad(){
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