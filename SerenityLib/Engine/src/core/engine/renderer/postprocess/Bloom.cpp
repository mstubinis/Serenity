#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>

#include <glm/common.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Bloom Bloom::bloom;

Bloom::Bloom() {
    num_passes    = 3;
    bloom_active  = true;
    blur_radius   = 1.24f;
    blur_strength = 0.62f;
    scale         = 0.27f;
    threshold     = 0.55f;
    exposure      = 1.6f;
}
Bloom::~Bloom() {
}
void Bloom::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();

    Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTextureEnum), 0);

    const unsigned int& screen_width = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& screen_height = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(screen_width, screen_height);
}


const float Renderer::bloom::getThreshold() {
    return Bloom::bloom.threshold;
}
void Renderer::bloom::setThreshold(const float t) {
    Bloom::bloom.threshold = t;
}
const float Renderer::bloom::getExposure() {
    return Bloom::bloom.exposure;
}
void Renderer::bloom::setExposure(const float e) {
    Bloom::bloom.exposure = e;
}
const bool Renderer::bloom::enabled() {
    return Bloom::bloom.bloom_active;
}
const unsigned int Renderer::bloom::getNumPasses() {
    return Bloom::bloom.num_passes;
}
void Renderer::bloom::setNumPasses(const unsigned int p) {
    Bloom::bloom.num_passes = p;
}
void Renderer::bloom::enable(const bool b) {
    Bloom::bloom.bloom_active = b;
}
void Renderer::bloom::disable() {
    Bloom::bloom.bloom_active = false;
}
const float Renderer::bloom::getBlurRadius() {
    return Bloom::bloom.blur_radius;
}
const float Renderer::bloom::getBlurStrength() {
    return Bloom::bloom.blur_strength;
}
void Renderer::bloom::setBlurRadius(const float r) {
    Bloom::bloom.blur_radius = glm::max(0.0f, r);
}
void Renderer::bloom::setBlurStrength(const float r) {
    Bloom::bloom.blur_strength = glm::max(0.0f, r);
}
const float Renderer::bloom::getScale() {
    return Bloom::bloom.scale;
}
void Renderer::bloom::setScale(const float s) {
    Bloom::bloom.scale = glm::max(0.0f, s);
}