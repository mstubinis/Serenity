#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/ShaderProgram.h>

#include <glm/common.hpp>

using namespace Engine;
using namespace std;

epriv::Postprocessing_Bloom epriv::Postprocessing_Bloom::Bloom;

epriv::Postprocessing_Bloom::Postprocessing_Bloom() {
    num_passes    = 3;
    bloom         = true;
    blur_radius   = 1.24f;
    blur_strength = 0.62f;
    scale         = 0.27f;
    threshold     = 0.55f;
    exposure      = 1.6f;
}
epriv::Postprocessing_Bloom::~Postprocessing_Bloom() {
}
void epriv::Postprocessing_Bloom::pass(ShaderP& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();

    Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTextureEnum), 0);

    unsigned int x = unsigned int(float(fboWidth) * divisor);
    unsigned int y = unsigned int(float(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(x, y);
}


float Renderer::bloom::getThreshold() { 
    return epriv::Postprocessing_Bloom::Bloom.threshold;
}
void Renderer::bloom::setThreshold(float t) {
    epriv::Postprocessing_Bloom::Bloom.threshold = t;
}
float Renderer::bloom::getExposure() {
    return epriv::Postprocessing_Bloom::Bloom.exposure;
}
void Renderer::bloom::setExposure(float e) {
    epriv::Postprocessing_Bloom::Bloom.exposure = e;
}
bool Renderer::bloom::enabled() {
    return epriv::Postprocessing_Bloom::Bloom.bloom;
}
unsigned int Renderer::bloom::getNumPasses() {
    return epriv::Postprocessing_Bloom::Bloom.num_passes;
}
void Renderer::bloom::setNumPasses(unsigned int p) {
    epriv::Postprocessing_Bloom::Bloom.num_passes = p;
}
void Renderer::bloom::enable(bool b) {
    epriv::Postprocessing_Bloom::Bloom.bloom = b;
}
void Renderer::bloom::disable() {
    epriv::Postprocessing_Bloom::Bloom.bloom = false;
}
float Renderer::bloom::getBlurRadius() {
    return epriv::Postprocessing_Bloom::Bloom.blur_radius;
}
float Renderer::bloom::getBlurStrength() {
    return epriv::Postprocessing_Bloom::Bloom.blur_strength;
}
void Renderer::bloom::setBlurRadius(float r) {
    epriv::Postprocessing_Bloom::Bloom.blur_radius = glm::max(0.0f, r);
}
void Renderer::bloom::setBlurStrength(float r) {
    epriv::Postprocessing_Bloom::Bloom.blur_strength = glm::max(0.0f, r);
}
float Renderer::bloom::getScale() {
    return epriv::Postprocessing_Bloom::Bloom.scale;
}
void Renderer::bloom::setScale(float s) {
    epriv::Postprocessing_Bloom::Bloom.scale = glm::max(0.0f, s);
}