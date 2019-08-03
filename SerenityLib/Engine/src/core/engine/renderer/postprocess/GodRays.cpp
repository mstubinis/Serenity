#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>


using namespace Engine;
using namespace Engine::epriv;
using namespace std;

GodRays GodRays::godRays;

GodRays::GodRays() {
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
    sun = nullptr;
}
GodRays::~GodRays() {
}
void GodRays::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight,const glm::vec2& lightScrnPos,const float& alpha) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();
    Renderer::sendUniform4("RaysInfo", exposure, decay, density, weight);
    Renderer::sendUniform2("lightPositionOnScreen", lightScrnPos.x / static_cast<float>(fboWidth), lightScrnPos.y / static_cast<float>(fboHeight));
    Renderer::sendUniform1("samples", samples);
    Renderer::sendUniform1("alpha", alpha);
    Renderer::sendTexture("firstPass", gbuffer.getTexture(GBufferType::Misc), 0);

    const unsigned int& screen_width = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& screen_height = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(screen_width, screen_height);
}
const bool Renderer::godRays::enabled() {
    return GodRays::godRays.godRays_active;
}
void Renderer::godRays::enable(const bool b) {
    GodRays::godRays.godRays_active = b;
}
void Renderer::godRays::disable() { 
    GodRays::godRays.godRays_active = false;
}
const float Renderer::godRays::getExposure() {
    return GodRays::godRays.exposure;
}
const float Renderer::godRays::getFactor() {
    return GodRays::godRays.factor;
}
const float Renderer::godRays::getDecay() {
    return GodRays::godRays.decay;
}
const float Renderer::godRays::getDensity() {
    return GodRays::godRays.density;
}
const float Renderer::godRays::getWeight() {
    return GodRays::godRays.weight;
}
const unsigned int Renderer::godRays::getSamples() {
    return GodRays::godRays.samples;
}
const float Renderer::godRays::getFOVDegrees() {
    return GodRays::godRays.fovDegrees;
}
const float Renderer::godRays::getAlphaFalloff() {
    return GodRays::godRays.alphaFalloff;
}
void Renderer::godRays::setExposure(const float e) {
    GodRays::godRays.exposure = e;
}
void Renderer::godRays::setFactor(const float f) {
    GodRays::godRays.factor = f;
}
void Renderer::godRays::setDecay(const float d) {
    GodRays::godRays.decay = d;
}
void Renderer::godRays::setDensity(const float d) {
    GodRays::godRays.density = d;
}
void Renderer::godRays::setWeight(const float w) {
    GodRays::godRays.weight = w;
}
void Renderer::godRays::setSamples(const unsigned int s) {
    GodRays::godRays.samples = glm::max((unsigned int)0, s);
}
void Renderer::godRays::setFOVDegrees(const float d) {
    GodRays::godRays.fovDegrees = d;
}
void Renderer::godRays::setAlphaFalloff(const float a) {
    GodRays::godRays.alphaFalloff = a;
}
void Renderer::godRays::setSun(Entity* entity) { 
    GodRays::godRays.sun = entity;
}
Entity* Renderer::godRays::getSun() { 
    return GodRays::godRays.sun;
}