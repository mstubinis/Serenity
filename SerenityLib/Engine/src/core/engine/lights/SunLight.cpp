#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Engine_Resources.h>

#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;


SunLight::SunLight(const glm_vec3& pos, const LightType::Type type, Scene* scene) : Entity(*scene){
    if (!scene) {
        scene = Resources::getCurrentScene();
    }

    m_Type              = type;

    if (type == LightType::Sun) {
        auto& sunLights = priv::InternalScenePublicInterface::GetSunLights(*scene);
        sunLights.push_back(this);
    }
    auto& allLights = priv::InternalScenePublicInterface::GetLights(*scene);
    allLights.push_back(this);

    addComponent<ComponentBody>();
    SunLight::setPosition(pos);
}
SunLight::~SunLight() {

}
const glm_vec3 SunLight::position() {
    return getComponent<ComponentBody>()->position(); 
}
const glm::vec4& SunLight::color() const {
    return m_Color;
}
const bool SunLight::isActive() const {
    return m_Active;
}
const uint SunLight::type() const {
    return m_Type;
}
const float SunLight::getAmbientIntensity() const {
    return m_AmbientIntensity;
}
const float SunLight::getDiffuseIntensity() const {
    return m_DiffuseIntensity;
}
const float SunLight::getSpecularIntensity() const {
    return m_SpecularIntensity;
}

void SunLight::setColor(const float r, const float g, const float b, const float a) {
    m_Color = glm::vec4(r, g, b, a); 
}
void SunLight::setColor(const glm::vec4& col) {
    m_Color = col; 
}
void SunLight::setColor(const glm::vec3& col) {
    m_Color.r = col.r; 
    m_Color.g = col.g; 
    m_Color.b = col.b; 
}
void SunLight::setPosition(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setPosition(x, y, z); 
}
void SunLight::setPosition(const decimal& position) {
    getComponent<ComponentBody>()->setPosition(position, position, position);
}
void SunLight::setPosition(const glm_vec3& position) {
    getComponent<ComponentBody>()->setPosition(position);
}

void SunLight::setAmbientIntensity(const float a) {
    m_AmbientIntensity = a; 
}
void SunLight::setDiffuseIntensity(const float d) {
    m_DiffuseIntensity = d; 
}
void SunLight::setSpecularIntensity(const float s) {
    m_SpecularIntensity = s; 
}
void SunLight::activate(const bool b) {
    m_Active = b; 
}
void SunLight::deactivate() { 
    m_Active = false; 
}
void SunLight::destroy() {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetSunLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}