#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>


using namespace std;

Engine::priv::GodRays Engine::priv::GodRays::godRays;

Engine::priv::GodRays::GodRays() {
    godRays_active = true;
    clearColor = glm::vec4(0.030f, 0.023f, 0.032f, 1.0f);
    exposure = 0.03f;
    factor = 1.0f;
    decay = 0.97f;
    density = 1.5f;
    weight = 0.567f;
    samples = 80;
    fovDegrees = 75.0f;
    alphaFalloff = 2.0f;
}
Engine::priv::GodRays::~GodRays() {
}
void Engine::priv::GodRays::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight,const glm::vec2& lightScrnPos,const float& alpha) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();
    Engine::Renderer::sendUniform4("RaysInfo", exposure, decay, density, weight);
    Engine::Renderer::sendUniform2("lightPositionOnScreen", lightScrnPos.x / static_cast<float>(fboWidth), lightScrnPos.y / static_cast<float>(fboHeight));
    Engine::Renderer::sendUniform1("samples", samples);
    Engine::Renderer::sendUniform1("alpha", alpha);
    Engine::Renderer::sendTexture("firstPass", gbuffer.getTexture(GBufferType::Misc), 0);

    const unsigned int& screen_width = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& screen_height = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Engine::Renderer::renderFullscreenTriangle(screen_width, screen_height);
}
const bool Engine::Renderer::godRays::enabled() {
    return Engine::priv::GodRays::godRays.godRays_active;
}
void Engine::Renderer::godRays::enable(const bool b) {
    Engine::priv::GodRays::godRays.godRays_active = b;
}
void Engine::Renderer::godRays::disable() {
    Engine::priv::GodRays::godRays.godRays_active = false;
}
const float Engine::Renderer::godRays::getExposure() {
    return Engine::priv::GodRays::godRays.exposure;
}
const float Engine::Renderer::godRays::getFactor() {
    return Engine::priv::GodRays::godRays.factor;
}
const float Engine::Renderer::godRays::getDecay() {
    return Engine::priv::GodRays::godRays.decay;
}
const float Engine::Renderer::godRays::getDensity() {
    return Engine::priv::GodRays::godRays.density;
}
const float Engine::Renderer::godRays::getWeight() {
    return Engine::priv::GodRays::godRays.weight;
}
const unsigned int Engine::Renderer::godRays::getSamples() {
    return Engine::priv::GodRays::godRays.samples;
}
const float Engine::Renderer::godRays::getFOVDegrees() {
    return Engine::priv::GodRays::godRays.fovDegrees;
}
const float Engine::Renderer::godRays::getAlphaFalloff() {
    return Engine::priv::GodRays::godRays.alphaFalloff;
}
void Engine::Renderer::godRays::setExposure(const float e) {
    Engine::priv::GodRays::godRays.exposure = e;
}
void Engine::Renderer::godRays::setFactor(const float f) {
    Engine::priv::GodRays::godRays.factor = f;
}
void Engine::Renderer::godRays::setDecay(const float d) {
    Engine::priv::GodRays::godRays.decay = d;
}
void Engine::Renderer::godRays::setDensity(const float d) {
    Engine::priv::GodRays::godRays.density = d;
}
void Engine::Renderer::godRays::setWeight(const float w) {
    Engine::priv::GodRays::godRays.weight = w;
}
void Engine::Renderer::godRays::setSamples(const unsigned int s) {
    Engine::priv::GodRays::godRays.samples = glm::max(0U, s);
}
void Engine::Renderer::godRays::setFOVDegrees(const float d) {
    Engine::priv::GodRays::godRays.fovDegrees = d;
}
void Engine::Renderer::godRays::setAlphaFalloff(const float a) {
    Engine::priv::GodRays::godRays.alphaFalloff = a;
}
void Engine::Renderer::godRays::setSun(Entity* entity) {
    Resources::getCurrentScene()->setGodRaysSun(entity);
}
Entity* Engine::Renderer::godRays::getSun() {
    return Engine::Resources::getCurrentScene()->getGodRaysSun();
}