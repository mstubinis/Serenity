#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Engine_Resources.h>

#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;


SunLight::SunLight(const glm_vec3& pos, const LightType::Type type, Scene* scene) : Entity(*scene){
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    m_Type = type;
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
bool SunLight::isShadowCaster() const {
    return m_IsShadowCaster;
}
void SunLight::setShadowCaster(bool castsShadow) {
    m_IsShadowCaster = castsShadow;
}
glm_vec3 SunLight::position() const {
    return getComponent<ComponentBody>()->getPosition();
}
const glm::vec4& SunLight::color() const {
    return m_Color;
}
bool SunLight::isActive() const {
    return m_Active;
}
LightType::Type SunLight::type() const {
    return m_Type;
}
float SunLight::getAmbientIntensity() const {
    return m_AmbientIntensity;
}
float SunLight::getDiffuseIntensity() const {
    return m_DiffuseIntensity;
}
float SunLight::getSpecularIntensity() const {
    return m_SpecularIntensity;
}

void SunLight::setColor(float r, float g, float b, float a) {
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

void SunLight::setAmbientIntensity(float a) {
    m_AmbientIntensity = a; 
}
void SunLight::setDiffuseIntensity(float d) {
    m_DiffuseIntensity = d; 
}
void SunLight::setSpecularIntensity(float s) {
    m_SpecularIntensity = s; 
}
void SunLight::activate(bool b) {
    m_Active = b; 
}
void SunLight::deactivate() { 
    m_Active = false; 
}
void SunLight::free() {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetSunLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}