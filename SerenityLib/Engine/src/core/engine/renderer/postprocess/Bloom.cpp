#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/ShaderProgram.h>

#include <glm/common.hpp>

using namespace Engine;
using namespace std;

epriv::Postprocess_Bloom epriv::Postprocess_Bloom::Bloom;

epriv::Postprocess_Bloom::Postprocess_Bloom() {
    num_passes    = 3;
    bloom         = true;
    blur_radius   = 1.24f;
    blur_strength = 0.62f;
    scale         = 0.27f;
    threshold     = 0.55f;
    exposure      = 1.6f;
}
epriv::Postprocess_Bloom::~Postprocess_Bloom() {
}
void epriv::Postprocess_Bloom::pass(ShaderP& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();

    Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTextureEnum), 0);

    const unsigned int& x = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& y = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(x, y);
}


float Renderer::bloom::getThreshold() { 
    return epriv::Postprocess_Bloom::Bloom.threshold;
}
void Renderer::bloom::setThreshold(const float t) {
    epriv::Postprocess_Bloom::Bloom.threshold = t;
}
float Renderer::bloom::getExposure() {
    return epriv::Postprocess_Bloom::Bloom.exposure;
}
void Renderer::bloom::setExposure(const float e) {
    epriv::Postprocess_Bloom::Bloom.exposure = e;
}
bool Renderer::bloom::enabled() {
    return epriv::Postprocess_Bloom::Bloom.bloom;
}
unsigned int Renderer::bloom::getNumPasses() {
    return epriv::Postprocess_Bloom::Bloom.num_passes;
}
void Renderer::bloom::setNumPasses(const unsigned int p) {
    epriv::Postprocess_Bloom::Bloom.num_passes = p;
}
void Renderer::bloom::enable(const bool b) {
    epriv::Postprocess_Bloom::Bloom.bloom = b;
}
void Renderer::bloom::disable() {
    epriv::Postprocess_Bloom::Bloom.bloom = false;
}
float Renderer::bloom::getBlurRadius() {
    return epriv::Postprocess_Bloom::Bloom.blur_radius;
}
float Renderer::bloom::getBlurStrength() {
    return epriv::Postprocess_Bloom::Bloom.blur_strength;
}
void Renderer::bloom::setBlurRadius(const float r) {
    epriv::Postprocess_Bloom::Bloom.blur_radius = glm::max(0.0f, r);
}
void Renderer::bloom::setBlurStrength(const float r) {
    epriv::Postprocess_Bloom::Bloom.blur_strength = glm::max(0.0f, r);
}
float Renderer::bloom::getScale() {
    return epriv::Postprocess_Bloom::Bloom.scale;
}
void Renderer::bloom::setScale(const float s) {
    epriv::Postprocess_Bloom::Bloom.scale = glm::max(0.0f, s);
}