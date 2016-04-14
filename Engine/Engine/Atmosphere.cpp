#include "Atmosphere.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Light.h"
using namespace Engine;

Atmosphere::Atmosphere(std::string name,Scene* scene,bool followCamera):SkyboxEmpty(name,scene){
	m_FollowCamera = followCamera;
	m_Mesh = Resources::getMesh("Planet");
	m_Model = glm::mat4(1);
	m_Position = glm::vec3(0.0f,-10024.0f,0.0f);
	m_Scale = glm::vec3(10000,10000,10000);

	m_Atmosphere = AtmosphereInformation();

	m_Model = glm::translate(m_Model,m_Position);
	m_Model = glm::scale(m_Model,m_Scale);
	m_Model = glm::scale(m_Model,glm::vec3(1 + m_Atmosphere.height));
}
Atmosphere::~Atmosphere(){
}
void Atmosphere::update(){
	if(m_FollowCamera){
		glm::v3 cameraPos = Resources::getActiveCamera()->getPosition();
		m_Model[3][0] = (float)cameraPos.x + m_Position.x;
		m_Model[3][1] = (float)cameraPos.y + m_Position.y;
		m_Model[3][2] = (float)cameraPos.z + m_Position.z;
	}
}
float Atmosphere::getRadius(){
	return m_Mesh->getRadius() * glm::max(glm::max(m_Scale.x,m_Scale.y),m_Scale.z);
}
glm::v3 Atmosphere::getPosition(){ return glm::v3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
void Atmosphere::render(){
    GLuint shader = Resources::getShader("AS_SkyFromAtmosphere")->getShaderProgram();
	Camera* cam = Resources::getActiveCamera();
	SunLight* sun = static_cast<SunLight*>(Resources::getObject("Sun Light"));
    glUseProgram(shader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glm::v3 earthPosition = getPosition();
	glm::v3 camPos = glm::v3(cam->getPosition() - earthPosition);

    glm::vec3 lightDir = glm::vec3(sun->getPosition() - earthPosition);
    lightDir = glm::normalize(lightDir);

    float innerRadius = getRadius();
	float outerRadius = innerRadius + (innerRadius * m_Atmosphere.height);

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(cam->getViewProjection()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform1f(glGetUniformLocation(shader, "far"),cam->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

    glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
    glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);
    glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), float(camPos.x),float(camPos.y),float(camPos.z));
    glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

	glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), m_Atmosphere.inverseWavelength.x,m_Atmosphere.inverseWavelength.y,m_Atmosphere.inverseWavelength.z);
    float camHeight = float(glm::length(camPos));
    float camHeight2 = camHeight*camHeight;
	glUniform1f(glGetUniformLocation(shader,"fCameraHeight"),camHeight);
    glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);
    glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
    glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
    glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
    glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

	glUniform1f(glGetUniformLocation(shader,"fKrESun"), m_Atmosphere.Kr * m_Atmosphere.ESun);
    glUniform1f(glGetUniformLocation(shader,"fKmESun"), m_Atmosphere.Km * m_Atmosphere.ESun);
    glUniform1f(glGetUniformLocation(shader,"fKr4PI"), m_Atmosphere.Kr * 4 * 3.14159f);
    glUniform1f(glGetUniformLocation(shader,"fKm4PI"), m_Atmosphere.Km * 4 * 3.14159f);

    float fScale = 1.0f / (outerRadius - innerRadius);
	glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),m_Atmosphere.scaledepth);
    glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
    glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / m_Atmosphere.scaledepth);

    // Gravity
	glUniform1f(glGetUniformLocation(shader,"g"),m_Atmosphere.gravity);
    glUniform1f(glGetUniformLocation(shader,"g2"), m_Atmosphere.gravity*m_Atmosphere.gravity);
    glUniform1f(glGetUniformLocation(shader,"fExposure"),m_Atmosphere.exposure);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	m_Mesh->render();

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
    glUseProgram(0);
}