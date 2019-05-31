#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/ShaderProgram.h>


using namespace Engine;

epriv::Postprocess_GodRays epriv::Postprocess_GodRays::GodRays;

epriv::Postprocess_GodRays::Postprocess_GodRays() {
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
epriv::Postprocess_GodRays::~Postprocess_GodRays() {

}
void epriv::Postprocess_GodRays::pass(ShaderP& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight,const glm::vec2& lightScrnPos,const float& alpha) {
    program.bind();
    const float& _divisor = gbuffer.getSmallFBO()->divisor();
    Renderer::sendUniform4("RaysInfo", exposure, decay, density, weight);
    Renderer::sendUniform2("lightPositionOnScreen", lightScrnPos.x / float(fboWidth), lightScrnPos.y / float(fboHeight));
    Renderer::sendUniform1("samples", samples);
    Renderer::sendUniform1("alpha", alpha);
    Renderer::sendTexture("firstPass", gbuffer.getTexture(GBufferType::Misc), 0);

    const unsigned int& x = static_cast<unsigned int>(float(fboWidth) * _divisor);
    const unsigned int& y = static_cast<unsigned int>(float(fboHeight) * _divisor);
    Renderer::renderFullscreenTriangle(x, y);
}



bool Renderer::godRays::enabled() { 
    return epriv::Postprocess_GodRays::GodRays.godRays;
}
void Renderer::godRays::enable(bool b = true) { 
    epriv::Postprocess_GodRays::GodRays.godRays = b;
}
void Renderer::godRays::disable() { 
    epriv::Postprocess_GodRays::GodRays.godRays = false;
}
float Renderer::godRays::getExposure() { 
    return epriv::Postprocess_GodRays::GodRays.exposure;
}
float Renderer::godRays::getFactor() { 
    return epriv::Postprocess_GodRays::GodRays.factor;
}
float Renderer::godRays::getDecay() { 
    return epriv::Postprocess_GodRays::GodRays.decay;
}
float Renderer::godRays::getDensity() { 
    return epriv::Postprocess_GodRays::GodRays.density;
}
float Renderer::godRays::getWeight() { 
    return epriv::Postprocess_GodRays::GodRays.weight;
}
unsigned int Renderer::godRays::getSamples() { 
    return epriv::Postprocess_GodRays::GodRays.samples;
}
float Renderer::godRays::getFOVDegrees() { 
    return epriv::Postprocess_GodRays::GodRays.fovDegrees;
}
float Renderer::godRays::getAlphaFalloff() { 
    return epriv::Postprocess_GodRays::GodRays.alphaFalloff;
}
void Renderer::godRays::setExposure(float e) { 
    epriv::Postprocess_GodRays::GodRays.exposure = e;
}
void Renderer::godRays::setFactor(float f) { 
    epriv::Postprocess_GodRays::GodRays.factor = f;
}
void Renderer::godRays::setDecay(float d) { 
    epriv::Postprocess_GodRays::GodRays.decay = d;
}
void Renderer::godRays::setDensity(float d) { 
    epriv::Postprocess_GodRays::GodRays.density = d;
}
void Renderer::godRays::setWeight(float w) { 
    epriv::Postprocess_GodRays::GodRays.weight = w;
}
void Renderer::godRays::setSamples(unsigned int s) {
    epriv::Postprocess_GodRays::GodRays.samples = glm::max((unsigned int)0, s);
}
void Renderer::godRays::setFOVDegrees(float d) { 
    epriv::Postprocess_GodRays::GodRays.fovDegrees = d;
}
void Renderer::godRays::setAlphaFalloff(float a) { 
    epriv::Postprocess_GodRays::GodRays.alphaFalloff = a;
}
void Renderer::godRays::setSun(Entity* entity) { 
    epriv::Postprocess_GodRays::GodRays.sun = entity;
}
Entity* Renderer::godRays::getSun() { 
    return epriv::Postprocess_GodRays::GodRays.sun;
}