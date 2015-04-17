#include "Skybox.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"

Skybox::Skybox(std::string name): Object("Skybox","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Skybox",true){
	m_Position = glm::vec3(0,0,0);
	m_Scale = glm::vec3(99999,99999,99999);
	m_WorldMatrix = m_Model = glm::mat4(1);

	m_ShaderProgram = Resources->Get_Shader_Program("Deferred_Skybox")->Get_Shader_Program();

	glActiveTexture(GL_TEXTURE0);
	std::string front = "Textures\\Skyboxes\\" + name + "\\Front.png";
	std::string back = "Textures\\Skyboxes\\" + name + "\\Back.png";
	std::string left = "Textures\\Skyboxes\\" + name + "\\Left.png";
	std::string right = "Textures\\Skyboxes\\" + name + "\\Right.png";
	std::string top = "Textures\\Skyboxes\\" + name + "\\Top.png";
	std::string bottom = "Textures\\Skyboxes\\" + name + "\\Bottom.png";
	std::string names[6] = {front,back,left,right,top,bottom};
	Resources->Load_Cubemap_Texture_Into_GLuint(m_Texture,names);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
}
Skybox::~Skybox(){
	glDeleteTextures(1,&m_Texture);
}
void Skybox::Update(float dt){
	m_Position = Resources->Current_Camera()->Position();

	m_Model = glm::mat4(1);
	m_Model = glm::translate(m_Model, m_Position);
	m_Model = glm::scale(m_Model,m_Scale);

	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(m_Model);
}
void Skybox::Render(bool debug){
	glDepthMask (GL_FALSE);
	glUseProgram( m_ShaderProgram );

	glUniformMatrix4fv(glGetUniformLocation( m_ShaderProgram, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation( m_ShaderProgram, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
	glUniform1i(glGetUniformLocation(m_ShaderProgram, "cubemap"), 0);

	glBindBuffer( GL_ARRAY_BUFFER, m_Mesh->Buffers()[0] );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glDrawArrays(GL_TRIANGLES, 0, m_Mesh->Points().size());
	glDisableVertexAttribArray(0);
	glDepthMask(GL_TRUE);
}