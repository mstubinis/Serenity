#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/ShaderProgram.h>


using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Postprocess_GodRays Postprocess_GodRays::GodRays;

Postprocess_GodRays::Postprocess_GodRays() {
    godRays = true;
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
Postprocess_GodRays::~Postprocess_GodRays() {

}
void Postprocess_GodRays::pass(ShaderP& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight,const glm::vec2& lightScrnPos,const float& alpha) {
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
    return Postprocess_GodRays::GodRays.godRays;
}
void Renderer::godRays::enable(const bool b) {
    Postprocess_GodRays::GodRays.godRays = b;
}
void Renderer::godRays::disable() { 
    Postprocess_GodRays::GodRays.godRays = false;
}
const float Renderer::godRays::getExposure() {
    return Postprocess_GodRays::GodRays.exposure;
}
const float Renderer::godRays::getFactor() {
    return Postprocess_GodRays::GodRays.factor;
}
const float Renderer::godRays::getDecay() {
    return Postprocess_GodRays::GodRays.decay;
}
const float Renderer::godRays::getDensity() {
    return Postprocess_GodRays::GodRays.density;
}
const float Renderer::godRays::getWeight() {
    return Postprocess_GodRays::GodRays.weight;
}
const unsigned int Renderer::godRays::getSamples() {
    return Postprocess_GodRays::GodRays.samples;
}
const float Renderer::godRays::getFOVDegrees() {
    return Postprocess_GodRays::GodRays.fovDegrees;
}
const float Renderer::godRays::getAlphaFalloff() {
    return Postprocess_GodRays::GodRays.alphaFalloff;
}
void Renderer::godRays::setExposure(const float e) {
    Postprocess_GodRays::GodRays.exposure = e;
}
void Renderer::godRays::setFactor(const float f) {
    Postprocess_GodRays::GodRays.factor = f;
}
void Renderer::godRays::setDecay(const float d) {
    Postprocess_GodRays::GodRays.decay = d;
}
void Renderer::godRays::setDensity(const float d) {
    Postprocess_GodRays::GodRays.density = d;
}
void Renderer::godRays::setWeight(const float w) {
    Postprocess_GodRays::GodRays.weight = w;
}
void Renderer::godRays::setSamples(const unsigned int s) {
    Postprocess_GodRays::GodRays.samples = glm::max((unsigned int)0, s);
}
void Renderer::godRays::setFOVDegrees(const float d) {
    Postprocess_GodRays::GodRays.fovDegrees = d;
}
void Renderer::godRays::setAlphaFalloff(const float a) {
    Postprocess_GodRays::GodRays.alphaFalloff = a;
}
void Renderer::godRays::setSun(Entity* entity) { 
    Postprocess_GodRays::GodRays.sun = entity;
}
Entity* Renderer::godRays::getSun() { 
    return Postprocess_GodRays::GodRays.sun;
}