#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>

#include <glm/common.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Postprocess_Bloom Postprocess_Bloom::Bloom;

Postprocess_Bloom::Postprocess_Bloom() {
    num_passes    = 3;
    bloom         = true;
    blur_radius   = 1.24f;
    blur_strength = 0.62f;
    scale         = 0.27f;
    threshold     = 0.55f;
    exposure      = 1.6f;
}
Postprocess_Bloom::~Postprocess_Bloom() {
}
void Postprocess_Bloom::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();

    Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    //Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTextureEnum), 0);

    const unsigned int& screen_width = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& screen_height = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(screen_width, screen_height);
}


const float Renderer::bloom::getThreshold() {
    return Postprocess_Bloom::Bloom.threshold;
}
void Renderer::bloom::setThreshold(const float t) {
    Postprocess_Bloom::Bloom.threshold = t;
}
const float Renderer::bloom::getExposure() {
    return Postprocess_Bloom::Bloom.exposure;
}
void Renderer::bloom::setExposure(const float e) {
    Postprocess_Bloom::Bloom.exposure = e;
}
const bool Renderer::bloom::enabled() {
    return Postprocess_Bloom::Bloom.bloom;
}
const unsigned int Renderer::bloom::getNumPasses() {
    return Postprocess_Bloom::Bloom.num_passes;
}
void Renderer::bloom::setNumPasses(const unsigned int p) {
    Postprocess_Bloom::Bloom.num_passes = p;
}
void Renderer::bloom::enable(const bool b) {
    Postprocess_Bloom::Bloom.bloom = b;
}
void Renderer::bloom::disable() {
    Postprocess_Bloom::Bloom.bloom = false;
}
const float Renderer::bloom::getBlurRadius() {
    return Postprocess_Bloom::Bloom.blur_radius;
}
const float Renderer::bloom::getBlurStrength() {
    return Postprocess_Bloom::Bloom.blur_strength;
}
void Renderer::bloom::setBlurRadius(const float r) {
    Postprocess_Bloom::Bloom.blur_radius = glm::max(0.0f, r);
}
void Renderer::bloom::setBlurStrength(const float r) {
    Postprocess_Bloom::Bloom.blur_strength = glm::max(0.0f, r);
}
const float Renderer::bloom::getScale() {
    return Postprocess_Bloom::Bloom.scale;
}
void Renderer::bloom::setScale(const float s) {
    Postprocess_Bloom::Bloom.scale = glm::max(0.0f, s);
}