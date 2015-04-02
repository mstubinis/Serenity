#include "Light.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

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
Light::Light(LIGHT_TYPE type){
	m_Type = type;
    m_Color = glm::vec3(1,1,1);
    m_AmbientIntensity = 0.2f;
    m_DiffuseIntensity = 0.4f;

	Resources->Lights.push_back(this);
}
Light::~Light(){
}
void Light::Translate(float x, float y, float z){ m_Position.x += x; m_Position.y += y; m_Position.z += z; }
void Light::Translate(glm::vec3 translation){ Light::Translate(translation.x,translation.y,translation.z); }
void Light::Set_Position(float x, float y, float z){ m_Position.x = x; m_Position.y = y; m_Position.z = z; }
void Light::Set_Position(glm::vec3 position){ Light::Set_Position(position.x,position.y,position.z); }
void Light::Set_Color(float x, float y, float z){ m_Color.x = x; m_Color.y = y; m_Color.z = z; }
void Light::Set_Color(glm::vec3 color){ Light::Set_Color(color.x,color.y,color.z); }
void Light::Update(float dt)
{
}
void Light::Render(LIGHT_TYPE type,GLuint shader){ if(type != m_Type) return; }
void Light::RenderDebug(GLuint shader){
	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, m_Position);

	glm::mat4 MVP = Resources->Current_Camera()->Calculate_Projection(model);

	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),1,1,1);
	Resources->Get_Mesh("DEBUGLight")->Render();
}
DirectionalLight::DirectionalLight(glm::vec3 dir): Light(LIGHT_TYPE_DIRECTIONAL){
	m_Direction = dir;
}
DirectionalLight::~DirectionalLight()
{
}
void DirectionalLight::Render(LIGHT_TYPE type, GLuint shader){
	if(type != m_Type) return;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Direction;
	} m_DirLightLocation;

	m_DirLightLocation.Color = glGetUniformLocation(shader,"gDirectionalLight.Base.Color");
	m_DirLightLocation.AmbientIntensity = glGetUniformLocation(shader,"gDirectionalLight.Base.AmbientIntensity");
	m_DirLightLocation.DiffuseIntensity = glGetUniformLocation(shader,"gDirectionalLight.Base.DiffuseIntensity");
	m_DirLightLocation.Direction = glGetUniformLocation(shader,"gDirectionalLight.Direction");

	glUniform3f(m_DirLightLocation.Color, m_Color.x, m_Color.y, m_Color.z);
	glUniform3f(m_DirLightLocation.Direction, m_Direction.x, m_Direction.y,m_Direction.z);
    glUniform1f(m_DirLightLocation.AmbientIntensity, m_AmbientIntensity);
    glUniform1f(m_DirLightLocation.DiffuseIntensity, m_DiffuseIntensity);

    glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 0.6f);
    glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 0.6f);

	Init_Quad();
}

PointLight::PointLight(glm::vec3 pos): Light(LIGHT_TYPE_POINT){
	m_Position = pos;
	m_Constant = 0.3f;
	m_Linear = 0.01f;
	m_Exp = 0.3f;
}
PointLight::~PointLight(){
}
void PointLight::Render(LIGHT_TYPE type,GLuint shader){
	if(type != m_Type) return;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		struct {
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_pointLightLocation;
	for (auto light:Resources->Lights) {

		m_pointLightLocation.Color = glGetUniformLocation(shader,"gPointLight.Base.Color");
		m_pointLightLocation.AmbientIntensity = glGetUniformLocation(shader,"gPointLight.Base.AmbientIntensity");
		m_pointLightLocation.Position = glGetUniformLocation(shader,"gPointLight.Position");
		m_pointLightLocation.DiffuseIntensity = glGetUniformLocation(shader,"gPointLight.Base.DiffuseIntensity");
		m_pointLightLocation.Atten.Constant = glGetUniformLocation(shader,"gPointLight.Atten.Constant");
		m_pointLightLocation.Atten.Linear = glGetUniformLocation(shader,"gPointLight.Atten.Linear");
        m_pointLightLocation.Atten.Exp = glGetUniformLocation(shader,"gPointLight.Atten.Exp");

		glUniform3f(m_pointLightLocation.Color, m_Color.x, m_Color.y, m_Color.z);
        glUniform1f(m_pointLightLocation.AmbientIntensity, m_AmbientIntensity);
        glUniform1f(m_pointLightLocation.DiffuseIntensity, m_DiffuseIntensity);
		glUniform3f(m_pointLightLocation.Position, m_Position.x, m_Position.y, m_Position.z);
		glUniform1f(m_pointLightLocation.Atten.Constant, m_Constant);
        glUniform1f(m_pointLightLocation.Atten.Linear, m_Linear);
        glUniform1f(m_pointLightLocation.Atten.Exp, m_Exp);

        glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 0.6f);
        glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 0.6f);

		Init_Quad();
   	}
}

SpotLight::SpotLight(glm::vec3 pos): PointLight(pos)
{
    m_Direction = glm::vec3(0,0,-1);
    m_Cutoff = 0;
}
SpotLight::~SpotLight()
{
}
void SpotLight::Render(LIGHT_TYPE type,GLuint shader)
{
}