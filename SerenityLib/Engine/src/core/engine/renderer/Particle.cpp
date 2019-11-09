#include <core/engine/renderer/Particle.h>
#include <core/engine/renderer/ParticleEmitter.h>
#include <core/engine/renderer/ParticleEmissionProperties.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

struct DefaultParticleBindFunctor { void operator()(EngineResource* r) const {
    auto& i = *static_cast<ModelInstance*>(r);
    const auto& stage = i.stage();
    auto& scene = *Resources::getCurrentScene();
    Camera& camera = *scene.getActiveCamera();
    glm::vec3 camPos = camera.getPosition();
    Entity& parent = i.parent();
    auto& body = *(parent.getComponent<ComponentBody>());
    glm::mat4 parentModel = body.modelMatrixRendering();


    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());

    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    
    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct DefaultParticleUnbindFunctor { void operator()(EngineResource* r) const {
    //auto& i = *static_cast<ModelInstance*>(r);
}};


ParticleData::ParticleData(){
    m_Properties = &ParticleEmissionProperties::DefaultProperties;
    m_Velocity = m_Properties->m_InitialVelocityFunctor();
    m_Scale = m_Properties->m_InitialScaleFunctor();
    m_AngularVelocity = m_Properties->m_InitialAngularVelocityFunctor();
    m_Active = true;
    m_Timer = 0.0;
    m_Depth = 0.0f;
    m_Angle = 0.0f;
    m_Color = glm::vec4(1.0f);
}
ParticleData::ParticleData(ParticleEmissionProperties& properties){
    m_Properties = &properties;
    m_Active = true;
    m_Timer = 0.0;
    m_Depth = 0.0f;
    m_Angle = 0.0f;
    m_Color = glm::vec4(1.0f);

    auto& randMat = const_cast<Material&>(properties.getParticleMaterialRandom());

    m_Velocity = properties.m_InitialVelocityFunctor();
    m_Scale = properties.m_InitialScaleFunctor();
    m_AngularVelocity = properties.m_InitialAngularVelocityFunctor();
}
ParticleData::ParticleData(const ParticleData& other){
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Velocity = other.m_Velocity;
    m_Color = other.m_Color;
    m_Angle = other.m_Angle;
    m_Scale = other.m_Scale;
    m_Depth = other.m_Depth;
    m_AngularVelocity = other.m_AngularVelocity;
    m_Properties = other.m_Properties;
}
ParticleData& ParticleData::operator=(const ParticleData& other) {
    if (&other == this)
        return *this;
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Velocity = other.m_Velocity;
    m_Color = other.m_Color;
    m_Angle = other.m_Angle;
    m_Scale = other.m_Scale;
    m_Depth = other.m_Depth;
    m_AngularVelocity = other.m_AngularVelocity;
    m_Properties = other.m_Properties;
    return *this;
}
ParticleData::ParticleData(ParticleData&& other) noexcept{
    using std::swap;
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Velocity, other.m_Velocity);
    swap(m_Color, other.m_Color);
    swap(m_Angle, other.m_Angle);
    swap(m_Scale, other.m_Scale);
    swap(m_Depth, other.m_Depth);
    swap(m_AngularVelocity, other.m_AngularVelocity);
    swap(m_Properties, other.m_Properties);
}
ParticleData& ParticleData::operator=(ParticleData&& other) noexcept {
    using std::swap;
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Velocity, other.m_Velocity);
    swap(m_Color, other.m_Color);
    swap(m_Angle, other.m_Angle);
    swap(m_Scale, other.m_Scale);
    swap(m_Depth, other.m_Depth);
    swap(m_AngularVelocity, other.m_AngularVelocity);
    swap(m_Properties, other.m_Properties);
    return *this;
}




Particle::Particle() : EntityWrapper(*Resources::getCurrentScene()){}

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmissionProperties& properties, Scene& scene, const RenderStage::Stage stage) : EntityWrapper(scene){
    auto& randMat = const_cast<Material&>(properties.getParticleMaterialRandom());

    auto& particleBody = *addComponent<ComponentBody>();
    auto& particleModel = *addComponent<ComponentModel>(Mesh::Plane, &randMat, ShaderProgram::Forward, RenderStage::ForwardParticles);
    particleModel.setCustomBindFunctor(DefaultParticleBindFunctor());
    particleModel.setCustomUnbindFunctor(DefaultParticleUnbindFunctor());

    auto data = ParticleData(properties);
    init(data, emitterPosition, emitterRotation, particleBody, particleModel);
}
void Particle::init(ParticleData& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ComponentBody& particleBody, ComponentModel& particleModel) {
    m_Data = data;
    particleBody.setPosition(emitterPosition);
    m_Data.m_Velocity = Math::rotate_vec3(emitterRotation, m_Data.m_Velocity);
    particleModel.show();
}
Particle::~Particle() {

}
Particle::Particle(const Particle& other) : EntityWrapper(other) {
    m_Data = other.m_Data;
}
Particle& Particle::operator=(const Particle& other) {
    if (&other == this)
        return *this;
    m_Data = other.m_Data;
    return *this;
}
Particle::Particle(Particle&& other) noexcept : EntityWrapper(other) {
    using std::swap;
    swap(m_Data, other.m_Data);
}
Particle& Particle::operator=(Particle&& other) noexcept {
    using std::swap;
    swap(m_Data, other.m_Data);
    return *this;
}




void Particle::update(const double& dt) {
    if (m_Data.m_Active) {
        m_Data.m_Timer += dt;
        const auto fdt = static_cast<float>(dt);
        auto& prop = *m_Data.m_Properties;
        auto& bodyComponent = *getComponent<ComponentBody>();
        auto& modelComponent = *getComponent<ComponentModel>();
        auto& instance = modelComponent.getModel();
        auto& camera = *entity().scene().getActiveCamera();

        m_Data.m_Scale           += prop.m_ChangeInScaleFunctor(m_Data.m_Timer, dt);
        m_Data.m_Color            = prop.m_ColorFunctor(m_Data.m_Timer, dt);
        m_Data.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Angle           += m_Data.m_AngularVelocity;
        m_Data.m_Velocity        += prop.m_ChangeInVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Depth            = prop.m_DepthFunctor(m_Data.m_Timer, dt);

        bodyComponent.translate(m_Data.m_Velocity * fdt, false);
        bodyComponent.setRotation(camera.getOrientation());
        bodyComponent.rotate(0, 0, static_cast<decimal>(m_Data.m_Angle));
        instance.setColor(m_Data.m_Color);
        instance.setScale(m_Data.m_Scale.x, m_Data.m_Scale.y, 1.0f);
        //instance.rotate(0, 0, m_Data.m_Angle);

        auto vec = glm::normalize(bodyComponent.position() - camera.getPosition()) * static_cast<decimal>(m_Data.m_Depth);
        instance.setPosition(glm::vec3(vec));
        if (m_Data.m_Timer >= prop.m_Lifetime) {
            m_Data.m_Active = false;
            m_Data.m_Timer = 0.0;
            modelComponent.hide();
        }
    }
}
void Particle::update_multithreaded(const double& dt, mutex& mutex_) {
    if (m_Data.m_Active) {
        m_Data.m_Timer += dt;
        const auto fdt = static_cast<float>(dt);
        auto& prop = *m_Data.m_Properties;
        auto& camera = *entity().scene().getActiveCamera();
        mutex_.lock();
        auto& bodyComponent = *getComponent<ComponentBody>();
        auto& modelComponent = *getComponent<ComponentModel>();
        auto& bodyPos = bodyComponent.position();
        auto& cameraPos = camera.getPosition();
        mutex_.unlock();
        auto& instance = modelComponent.getModel();

        m_Data.m_Scale += prop.m_ChangeInScaleFunctor(m_Data.m_Timer, dt);
        m_Data.m_Color = prop.m_ColorFunctor(m_Data.m_Timer, dt);
        m_Data.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Angle += m_Data.m_AngularVelocity;
        m_Data.m_Velocity += prop.m_ChangeInVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Depth = prop.m_DepthFunctor(m_Data.m_Timer, dt);

        bodyComponent.translate(m_Data.m_Velocity * fdt, false);
        bodyComponent.setRotation(camera.getOrientation());
        bodyComponent.rotate(0, 0, static_cast<decimal>(m_Data.m_Angle));
        instance.setColor(m_Data.m_Color);
        instance.setScale(m_Data.m_Scale.x, m_Data.m_Scale.y, 1.0f);
        //instance.rotate(0, 0, m_Data.m_Angle);

        auto vec = glm::normalize(bodyPos - cameraPos) * static_cast<decimal>(m_Data.m_Depth);
        instance.setPosition(glm::vec3(vec));
        if (m_Data.m_Timer >= prop.m_Lifetime) {
            m_Data.m_Active = false;
            m_Data.m_Timer = 0.0;
            modelComponent.hide();
        }
    }
}
const bool& Particle::isActive() const {
    return m_Data.m_Active;
}
const glm::vec4& Particle::color() const {
    return m_Data.m_Color;
}
const glm::vec3& Particle::velocity() const {
    return m_Data.m_Velocity;
}
const double Particle::lifetime() const {
    return m_Data.m_Properties->m_Lifetime;
}