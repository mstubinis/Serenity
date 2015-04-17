#include "Light.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"

void Init_Quad(){
	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,Window->getSize().x,0,Window->getSize().y,0.1f,2);	
	
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
	glVertex3f(   (float) Window->getSize().x, 0.0f, 0.0f);
	glTexCoord2f( 1, 1 );
	glVertex3f(   (float) Window->getSize().x, (float) Window->getSize().y, 0.0f);
	glTexCoord2f( 0, 1 );
	glVertex3f(    0.0f,  (float) Window->getSize().y, 0.0f);
	glEnd();

	//Reset the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
SunLight::SunLight(glm::vec3 pos,std::string name,unsigned int type):Object("DEBUGLight","",pos,glm::vec3(1,1,1),name,false){
	m_Type = type;

    m_AmbientIntensity = 0.05f;
    m_DiffuseIntensity = 1.0f;
	m_Parent = nullptr;

	Resources->Lights.push_back(this);
}
SunLight::~SunLight(){}
void SunLight::Update(float dt){
	Object::Update(dt);
}
void SunLight::Render(GLuint shader){ 
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

	glm::vec3 pos = Position();
	glUniform3f(glGetUniformLocation(shader,"gLightPosition"), pos.x, pos.y, pos.z);

	glm::vec3 campos = Resources->Current_Camera()->Position();
	glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	Init_Quad();
}
void SunLight::RenderDebug(GLuint shader){
	glm::mat4 MVP = Resources->Current_Camera()->Calculate_Projection(m_Model);

	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),1,1,1);
	m_Mesh->Render();
}
DirectionalLight::DirectionalLight(glm::vec3 dir): SunLight(glm::vec3(0,0,0),"Directional Light",LIGHT_TYPE_DIRECTIONAL){
	m_Direction = dir;
}
DirectionalLight::~DirectionalLight()
{
}
void DirectionalLight::Render(GLuint shader){
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
	glUniform3f(glGetUniformLocation(shader,"gDirection"), m_Direction.x, m_Direction.y,m_Direction.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	Init_Quad();
}

PointLight::PointLight(glm::vec3 pos): SunLight(pos,"Point Light",LIGHT_TYPE_POINT){
	m_Constant = 0.3f;
	m_Linear = 0.2f;
	m_Exp = 0.3f;
}
PointLight::~PointLight(){
}
void PointLight::Render(GLuint shader){
	glUniform1i(glGetUniformLocation(shader,"gLightType"), static_cast<int>(m_Type));

	glUniform3f(glGetUniformLocation(shader,"gColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"gAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"gDiffuseIntensity"), m_DiffuseIntensity);

	glm::vec3 pos = Position();
	glUniform3f(glGetUniformLocation(shader,"gLightPosition"), pos.x, pos.y, pos.z);

	glm::vec3 campos = Resources->Current_Camera()->Position();
	glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

	glUniform1f(glGetUniformLocation(shader,"gConstant"), m_Constant);
    glUniform1f(glGetUniformLocation(shader,"gLinear"), m_Linear);
    glUniform1f(glGetUniformLocation(shader,"gExp"), m_Exp);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 1.0f);
	glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 150.0f);

	Init_Quad();
}
SpotLight::SpotLight(glm::vec3 pos): SunLight(pos,"Spot Light",LIGHT_TYPE_SPOT){
    m_Direction = glm::vec3(0,0,-1);
    m_Cutoff = 0;
}
SpotLight::~SpotLight()
{
}
void SpotLight::Render(GLuint shader)
{
}