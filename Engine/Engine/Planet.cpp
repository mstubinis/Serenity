#include "Engine_Resources.h"
#include "Light.h"
#include "Planet.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"

using namespace Engine;

Planet::Planet(std::string mat, PlanetType type, glm::vec3 pos,float scl, std::string name,Scene* scene):Object("Planet",mat,pos,glm::vec3(scl,scl,scl),name,true,scene){
	m_AtmosphereHeight = 0.025f;
	m_Type = type;
}
Planet::~Planet(){
}
void Planet::update(float dt){
	Object::update(dt);
}
void Planet::render(Mesh* mesh, Material* mat,bool debug){
	if(mesh == nullptr)
		return;
	if(!Resources::getActiveCamera()->sphereIntersectTest(this))
		return;
	if(Resources::getActiveCamera()->getDistance(this) > 450 * getRadius())
		return;

	GLuint shader = Resources::getShader("AS_GroundFromSpace")->getShaderProgram();

	float innerRadius = m_Radius;
	float outerRadius = innerRadius + (innerRadius * m_AtmosphereHeight);

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));

	glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

	glUniform3f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);

	for(auto component:mat->getComponents())
		mat->bindTexture(component.second,shader);

	if(m_AtmosphereHeight > 0){
		glUniform1i(glGetUniformLocation(shader,"hasAtmosphere"),1);
		glUniform1i(glGetUniformLocation(shader,"HasAtmosphere"),1);
	}
	else{
		glUniform1i(glGetUniformLocation(shader,"hasAtmosphere"),0);
		glUniform1i(glGetUniformLocation(shader,"HasAtmosphere"),0);
	}

	float Km = 0.0025f;
	float Kr = 0.0015f;
	float ESun = 20.0f;

	glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
	glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);

	glm::vec3 camPos = Resources::getActiveCamera()->getPosition() - getPosition();
	glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

	glm::vec3 lightDir = Resources::getCurrentScene()->getLights().begin()->second->getPosition() - getPosition();
	lightDir = glm::normalize(lightDir);
	glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

	glm::vec3 v3InvWaveLength = glm::vec3(1.0f / glm::pow(0.65f, 4.0f),1.0f / glm::pow(0.57f, 4.0f),1.0f / glm::pow(0.475f, 4.0f));

	glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

	float camHeight = glm::length(camPos);
	float camHeight2 = camHeight*camHeight;

	glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);


	glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

	glUniform1f(glGetUniformLocation(shader,"fKrESun"), Kr * ESun);
	glUniform1f(glGetUniformLocation(shader,"fKmESun"), Km * ESun);

	glUniform1f(glGetUniformLocation(shader,"fKr4PI"), Kr * 4 * 3.14159f);
	glUniform1f(glGetUniformLocation(shader,"fKm4PI"), Km * 4 * 3.14159f);

	float fScaledepth = 0.25f;
	float fScale = 1.0f / (outerRadius - innerRadius);

	glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),fScaledepth);
	glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
	glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / fScaledepth);

	glUniform1f(glGetUniformLocation(shader,"fExposure"), 2.0f);


	mesh->render();

	if(m_AtmosphereHeight <= 0.0f)
		return;

	if(camHeight > outerRadius){ 
		shader = Resources::getShader("AS_SkyFromSpace")->getShaderProgram();
	}
	else{
		shader = Resources::getShader("AS_SkyFromAtmosphere")->getShaderProgram();
	}


	glUseProgram(shader);

	glCullFace(GL_FRONT);

	glEnable(GL_BLEND);
   	glBlendEquation(GL_FUNC_ADD);
   	glBlendFunc(GL_ONE, GL_ONE);

	glm::mat4 obj = glm::mat4(1);
	obj = glm::translate(obj,getPosition());
	obj *= glm::mat4_cast(m_Orientation);
	obj = glm::scale(obj,glm::vec3(outerRadius,outerRadius,outerRadius));
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->calculateProjection(obj)));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(obj));
	glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

	glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
	glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);

	glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

	glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

	glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

	glUniform1f(glGetUniformLocation(shader,"fCameraHeight"),camHeight);
	glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);

	glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

	glUniform1f(glGetUniformLocation(shader,"fKrESun"), Kr * ESun);
	glUniform1f(glGetUniformLocation(shader,"fKmESun"), Km * ESun);

	glUniform1f(glGetUniformLocation(shader,"fKr4PI"), Kr * 4 * 3.14159f);
	glUniform1f(glGetUniformLocation(shader,"fKm4PI"), Km * 4 * 3.14159f);

	glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),fScaledepth);
	glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
	glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / fScaledepth);

	// Gravity
	float g = -0.98f;
	glUniform1f(glGetUniformLocation(shader,"g"),g);
	glUniform1f(glGetUniformLocation(shader,"g2"), g*g);
	glUniform1f(glGetUniformLocation(shader,"fExposure"),2.0f);

	Resources::getMesh("Planet")->render();

	glUseProgram(0);

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
}
void Planet::render(bool debug){ render(m_Mesh,m_Material,debug); }


Star::Star(glm::vec3 starColor, glm::vec3 lightColor, glm::vec3 pos,float scl, std::string name,Scene* scene): Planet("Star",PLANET_TYPE_STAR,pos,scl,name,scene){
	m_Light = new SunLight(glm::vec3(0,0,0),"Sun Light",LIGHT_TYPE_SUN,scene);
	m_Light->setColor(lightColor);
	setColor(starColor);
	m_Material->setShadeless(true);

	addChild(m_Light);
}
Star::~Star(){
	delete m_Light;
}
void Star::render(Mesh* mesh, Material* mat,bool debug){
	Object::render(mesh,mat,debug);
}
void Star::render(bool debug){ Star::render(m_Mesh,m_Material,debug); }
