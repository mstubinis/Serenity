#include "Skybox.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"

using namespace Engine;

Skybox::Skybox(std::string name,Scene* scene): Object("Skybox","",glm::vec3(0,0,0),glm::vec3(1,1,1),"***Skybox " + name,true,scene){
	m_Position = glm::vec3(0,0,0);
	m_Scale = glm::vec3(999999999,999999999,999999999);
	m_Model = glm::mat4(1);

	m_Shader = Resources::getShader("Deferred_Skybox")->getShaderProgram();

	glActiveTexture(GL_TEXTURE0);
	std::string front = "Textures\\Skyboxes\\" + name + "\\Front.png";
	std::string back = "Textures\\Skyboxes\\" + name + "\\Back.png";
	std::string left = "Textures\\Skyboxes\\" + name + "\\Left.png";
	std::string right = "Textures\\Skyboxes\\" + name + "\\Right.png";
	std::string top = "Textures\\Skyboxes\\" + name + "\\Top.png";
	std::string bottom = "Textures\\Skyboxes\\" + name + "\\Bottom.png";
	std::string names[6] = {front,back,left,right,top,bottom};

	m_Texture = new Texture(names,GL_TEXTURE_CUBE_MAP);
}
Skybox::~Skybox(){
	delete m_Texture;
}
void Skybox::update(float dt){
	m_Position = Resources::getActiveCamera()->getPosition();

	m_Model = glm::mat4(1);
	m_Model = glm::translate(m_Model, m_Position);
	m_Model = glm::scale(m_Model,m_Scale);
}
void Skybox::render(bool debug){
	glDisable(GL_DEPTH_TEST);
	glUseProgram( m_Shader );

	glUniformMatrix4fv(glGetUniformLocation( m_Shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
	glUniformMatrix4fv(glGetUniformLocation( m_Shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture->getTextureAddress());
	glUniform1i(glGetUniformLocation(m_Shader, "cubemap"), 0);

	glBindBuffer( GL_ARRAY_BUFFER, m_Mesh->getBuffers()[0] );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glDrawArrays(GL_TRIANGLES, 0, m_Mesh->getPoints().size());
	glDisableVertexAttribArray(0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}