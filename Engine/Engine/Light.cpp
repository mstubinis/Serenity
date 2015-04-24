#include "Light.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

void _initQuad(){
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
SunLight::SunLight(glm::vec3 pos,std::string name,unsigned int type,Scene* scene):Object("DEBUGLight","",pos,glm::vec3(1,1,1),name,false,scene){
	m_Type = type;

    m_AmbientIntensity = 0.05f;
    m_DiffuseIntensity = 1.0f;
	m_Parent = nullptr;

	unsigned int count = 0;
	if(scene == nullptr){
		scene = Resources::getCurrentScene();
	}
	if (scene->getLights().size() > 0){
		while(scene->getLights().count(m_Name)){
			m_Name = name + " " + boost::lexical_cast<std::string>(count);
			count++;
		}
	}
	scene->getLights()[m_Name] = this;
}
SunLight::~SunLight(){
}
void SunLight::setName(std::string name){
	std::string oldName = m_Name;
	m_Name = name;
	Resources::Detail::ResourceManagement::m_CurrentScene->getLights()[name] = this;

	for(auto it = begin(Resources::Detail::ResourceManagement::m_CurrentScene->getLights()); it != end(Resources::Detail::ResourceManagement::m_CurrentScene->getLights());){
		if (it->first == oldName)it = Resources::Detail::ResourceManagement::m_CurrentScene->getLights().erase(it);
	    else++it;
	}
}
void SunLight::update(float dt){
	Object::update(dt);
}
void SunLight::render(GLuint shader){ 
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

	glm::vec3 pos = getPosition();
	glUniform3f(glGetUniformLocation(shader,"gLightPosition"), pos.x, pos.y, pos.z);

	glm::vec3 campos = Resources::getActiveCamera()->getPosition();
	glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	_initQuad();
}
void SunLight::renderDebug(GLuint shader){
	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),1,1,1);
	m_Mesh->render();
}
DirectionalLight::DirectionalLight(glm::vec3 dir,Scene* scene): SunLight(glm::vec3(0,0,0),"Directional Light",LIGHT_TYPE_DIRECTIONAL,scene){
	m_Direction = dir;
}
DirectionalLight::~DirectionalLight(){
}
void DirectionalLight::render(GLuint shader){
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
	glUniform3f(glGetUniformLocation(shader,"gDirection"), m_Direction.x, m_Direction.y,m_Direction.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	_initQuad();
}

PointLight::PointLight(glm::vec3 pos,Scene* scene): SunLight(pos,"Point Light",LIGHT_TYPE_POINT,scene){
	m_Constant = 0.3f;
	m_Linear = 0.2f;
	m_Exp = 0.3f;
}
PointLight::~PointLight(){
}
void PointLight::render(GLuint shader){
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

	glm::vec3 pos = getPosition();
	glUniform3f(glGetUniformLocation(shader,"gLightPosition"), pos.x, pos.y, pos.z);

	glm::vec3 campos = Resources::getActiveCamera()->getPosition();
	glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

	glUniform1f(glGetUniformLocation(shader,"gConstant"), m_Constant);
    glUniform1f(glGetUniformLocation(shader,"gLinear"), m_Linear);
    glUniform1f(glGetUniformLocation(shader,"gExp"), m_Exp);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	_initQuad();
}
SpotLight::SpotLight(glm::vec3 pos,Scene* scene): SunLight(pos,"Spot Light",LIGHT_TYPE_SPOT){
    m_Direction = glm::vec3(0,0,-1);
    m_Cutoff = 0;
}
SpotLight::~SpotLight(){
}
void SpotLight::render(GLuint shader)
{
}