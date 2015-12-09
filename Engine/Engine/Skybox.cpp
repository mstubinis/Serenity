#include "Skybox.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Scene.h"

using namespace Engine;

Skybox::Skybox(std::string name,unsigned int numSunFlares,Scene* scene){
	m_Model = glm::mat4(1);

	glActiveTexture(GL_TEXTURE0);
	std::string front = "Textures/Skyboxes/" + name + "/Front.jpg";
	std::string back = "Textures/Skyboxes/" + name + "/Back.jpg";
	std::string left = "Textures/Skyboxes/" + name + "/Left.jpg";
	std::string right = "Textures/Skyboxes/" + name + "/Right.jpg";
	std::string top = "Textures/Skyboxes/" + name + "/Top.jpg";
	std::string bottom = "Textures/Skyboxes/" + name + "/Bottom.jpg";
	std::string names[6] = {front,back,left,right,top,bottom};

	m_Texture = new Texture(names,GL_TEXTURE_CUBE_MAP);
	m_Mesh = Resources::getMesh("Skybox");

	if(numSunFlares > 0){
		for(unsigned int i = 0; i < numSunFlares; i++){
			SkyboxSunFlare flare;

			float x = (((rand() % 100) - 50.0f) / 100.0f); x*=999999;
			float y = (((rand() % 100) - 50.0f) / 100.0f); y*=999999;
			float z = (((rand() % 100) - 50.0f) / 100.0f); z*=999999;

			float r = (rand() % 100) / 100.0f;
			float g = (rand() % 100) / 100.0f;
			float b = (rand() % 100) / 100.0f;

			float scl = ((rand() % 100)) / 300.0f; scl += 0.25;

			flare.position = glm::vec3(x,y,z);
			flare.scale = scl;
			flare.color = glm::vec3(r,g,b);
			m_SunFlares.push_back(flare);
		}
	}

	if(scene == nullptr) scene = Resources::getCurrentScene();
	if(scene->getSkybox() == nullptr)
		scene->setSkybox(this);
}
Skybox::~Skybox(){
	delete m_Texture;
}
void Skybox::_updateMatrix(){
	m_Model = glm::mat4(1);
	m_Model = glm::translate(m_Model, Resources::getActiveCamera()->getPosition());
	m_Model = glm::scale(m_Model,glm::vec3(999999,999999,999999));
}

glm::vec2 Skybox::getScreenCoordinates(glm::vec3 objPos){
	glm::vec2 windowSize = glm::vec2(Resources::getWindowSize().x,Resources::getWindowSize().y);
	glm::mat4 MV = Resources::getActiveCamera()->getView();
	glm::vec4 viewport = glm::vec4(0,0,windowSize.x,windowSize.y);
	glm::vec3 screen = glm::project(objPos,MV,Resources::getActiveCamera()->getProjection(),viewport);

	//check if point is behind
	float dot = glm::dot(Resources::getActiveCamera()->getViewVector(),glm::vec3(objPos-Resources::getActiveCamera()->getPosition()));

	if(dot < 0.0f){
		return glm::vec2(screen.x,windowSize.y-screen.y);
	}
	float fX = windowSize.x - screen.x;
	float fY = screen.y;

	if(fX < windowSize.x/2){ fX = -100; }
	else if(fX > windowSize.x/2){ fX = windowSize.x+100; }
	if(fY < windowSize.y/2){ fY = -100; }
	else if(fY > windowSize.y/2){ fY = windowSize.y+100; }

	return glm::vec2(fX,fY);
}


void Skybox::render(){
	GLuint shader = Resources::getShader("Deferred_Skybox")->getShaderProgram();
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()*m_Model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture->getTextureAddress());
	glUniform1i(glGetUniformLocation(shader, "Texture"), 0);

	glBindBuffer( GL_ARRAY_BUFFER, m_Mesh->getBuffers()[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, m_Mesh->getPoints().size());
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	if(m_SunFlares.size() > 0){
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Texture* texture = Resources::getTexture("Textures/Skyboxes/StarFlare");
		shader = Resources::getShader("Deferred_HUD")->getShaderProgram();
		glUseProgram(shader);
		for(auto flare:m_SunFlares){
			glm::vec2 pos = getScreenCoordinates(Resources::getActiveCamera()->getPosition() - flare.position);
			glm::vec4 col = glm::vec4(flare.color.x,flare.color.y,flare.color.z,1);
			glm::vec2 scl = glm::vec2(flare.scale,flare.scale);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getTextureAddress());
			glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
			glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);

			glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);
			glUniform4f(glGetUniformLocation(shader, "Object_Color"),col.x,col.y,col.z,1);

			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, glm::vec3(pos.x,
													Resources::getWindowSize().y-pos.y,
													-0.5 - 1));
			model = glm::rotate(model,0.0f,glm::vec3(0,0,1));
			model = glm::scale(model, glm::vec3(texture->getWidth(),texture->getHeight(),1));
			model = glm::scale(model, glm::vec3(scl.x,scl.y,1));
			glm::mat4 world = Resources::getCamera("HUD")->getProjection() * model; //we dont want the view matrix as we want to assume this "World" matrix originates from (0,0,0)

			glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(world));
			glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(model));

			Resources::getMesh("Plane")->render();
		}
		glUseProgram(0);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
}