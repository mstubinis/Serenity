#include "Light.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

SunLight::SunLight(glm::v3 pos,std::string name,unsigned int type,Scene* scene):ObjectDisplay("","",pos,glm::vec3(1),name,scene){
    m_Type = type;
	m_Active = true;
    m_AmbientIntensity = 0.05f;
    m_DiffuseIntensity = 1.0f;
    m_SpecularPower = 50;

    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    scene->getLights()[m_Name] = this;
}
SunLight::~SunLight(){
}
void SunLight::setName(std::string name){
    if(name == m_Name) return;

    std::string oldName = m_Name; m_Name = name;

    Resources::Detail::ResourceManagement::m_CurrentScene->getLights()[name] = this;
    if(Resources::Detail::ResourceManagement::m_CurrentScene->getLights().count(oldName)){
        Resources::Detail::ResourceManagement::m_CurrentScene->getLights().erase(oldName);
    }
    Resources::Detail::ResourceManagement::m_Objects[name] = boost::shared_ptr<Object>(this);
    if(Resources::Detail::ResourceManagement::m_Objects.count(oldName)){
        Resources::Detail::ResourceManagement::m_Objects[oldName].reset();
        Resources::Detail::ResourceManagement::m_Objects.erase(oldName);
    }
}
void SunLight::update(float dt){
    ObjectBasic::update(dt);
}
void SunLight::render(GLuint shader,bool debug){ 
	ObjectDisplay::render(shader,debug); 
}
void SunLight::draw(GLuint shader, bool debug,bool godsRays){ 
	ObjectDisplay::draw(shader,debug,godsRays); 
}
void SunLight::sendGenericAttributesToShader(GLuint shader){
    glUniform1i(glGetUniformLocation(shader,"LightType"), int(m_Type));
    glUniform3f(glGetUniformLocation(shader,"LightColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"LightAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"LightDiffuseIntensity"), m_DiffuseIntensity);
    glUniform1f(glGetUniformLocation(shader,"LightSpecularPower"), m_SpecularPower);
}
void SunLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
    glm::vec3 pos = glm::vec3(getPosition());
    glUniform3f(glGetUniformLocation(shader,"LightPosition"), pos.x, pos.y, pos.z);

    glm::vec3 campos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);
}
DirectionalLight::DirectionalLight(std::string name, glm::vec3 dir,Scene* scene): SunLight(glm::v3(0),name,LIGHT_TYPE_DIRECTIONAL,scene){
    m_Direction = glm::normalize(dir);
}
DirectionalLight::~DirectionalLight(){
}
void DirectionalLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
    glUniform3f(glGetUniformLocation(shader,"LightDirection"), m_Direction.x, m_Direction.y,m_Direction.z);
    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);
}

PointLight::PointLight(std::string name, glm::v3 pos,Scene* scene): SunLight(pos,name,LIGHT_TYPE_POINT,scene){
    m_Constant = 0.3f;
    m_Linear = 0.2f;
    m_Exp = 0.3f;
    m_PointLightRadius = calculatePointLightRadius();
}
PointLight::~PointLight(){
}
float PointLight::calculatePointLightRadius(){
    float MaxChannel = glm::max(glm::max(m_Color.x, m_Color.y), m_Color.z);
    float ret = (-m_Linear + glm::sqrt(m_Linear * m_Linear -
        4 * m_Exp * (m_Exp - 256 * MaxChannel * m_DiffuseIntensity))) / (2 * m_Exp);
    return ret;
}
void PointLight::setConstant(float c){ 
    m_Constant = c;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::setLinear(float l){ 
    m_Linear = l;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::setExponent(float e){ 
    m_Exp = e;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::lighten(GLuint shader){
	if(!m_Active) return;

    Camera* camera = Resources::getActiveCamera();
    glm::v3 pos = getPosition();
    if((!camera->sphereIntersectTest(pos,m_PointLightRadius)) || (camera->getDistance(this) > 1100 * m_PointLightRadius))
        return;
    sendGenericAttributesToShader(shader);
    glUniform3f(glGetUniformLocation(shader,"LightPosition"),float(pos.x),float(pos.y),float(pos.z));

    glm::vec3 campos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    glUniform1f(glGetUniformLocation(shader,"LightConstant"), m_Constant);
    glUniform1f(glGetUniformLocation(shader,"LightLinear"), m_Linear);
    glUniform1f(glGetUniformLocation(shader,"LightExp"), m_Exp);

    this->m_Orientation = Resources::getActiveCamera()->getOrientation();

    glm::mat4 m(1);
    m = glm::translate(m,glm::vec3(pos));
    m *= glm::mat4_cast(m_Orientation);
    m = glm::scale(m,glm::vec3(m_PointLightRadius));

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m));

    if(glm::distance(campos,glm::vec3(pos)) > m_PointLightRadius){
        glCullFace(GL_BACK);
    }
    else{
        glCullFace(GL_FRONT);
    }

    Resources::getMesh("PointLightBounds")->render();

    glCullFace(GL_BACK);
}
SpotLight::SpotLight(std::string name, glm::v3 pos,Scene* scene): SunLight(pos,name,LIGHT_TYPE_SPOT){
    m_Direction = glm::vec3(0,0,-1);
    m_Cutoff = 0;
}
SpotLight::~SpotLight(){
}
void SpotLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
}