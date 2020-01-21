#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>

#include <glm/common.hpp>

using namespace std;

Engine::priv::Bloom Engine::priv::Bloom::bloom;

Engine::priv::Bloom::Bloom() {
    num_passes    = 3;
    bloom_active  = true;
    blur_radius   = 1.24f;
    blur_strength = 0.62f;
    scale         = 0.27f;
    threshold     = 0.55f;
    exposure      = 1.6f;
}
Engine::priv::Bloom::~Bloom() {
}
void Engine::priv::Bloom::pass(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();

    Engine::Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTextureEnum), 0);

    const unsigned int& screen_width = static_cast<unsigned int>(static_cast<float>(fboWidth) * divisor);
    const unsigned int& screen_height = static_cast<unsigned int>(static_cast<float>(fboHeight) * divisor);
    Engine::Renderer::renderFullscreenTriangle(screen_width, screen_height);
}


const float Engine::Renderer::bloom::getThreshold() {
    return Engine::priv::Bloom::bloom.threshold;
}
void Engine::Renderer::bloom::setThreshold(const float t) {
    Engine::priv::Bloom::bloom.threshold = t;
}
const float Engine::Renderer::bloom::getExposure() {
    return Engine::priv::Bloom::bloom.exposure;
}
void Engine::Renderer::bloom::setExposure(const float e) {
    Engine::priv::Bloom::bloom.exposure = e;
}
const bool Engine::Renderer::bloom::enabled() {
    return Engine::priv::Bloom::bloom.bloom_active;
}
const unsigned int Engine::Renderer::bloom::getNumPasses() {
    return Engine::priv::Bloom::bloom.num_passes;
}
void Engine::Renderer::bloom::setNumPasses(const unsigned int p) {
    Engine::priv::Bloom::bloom.num_passes = p;
}
void Engine::Renderer::bloom::enable(const bool b) {
    Engine::priv::Bloom::bloom.bloom_active = b;
}
void Engine::Renderer::bloom::disable() {
    Engine::priv::Bloom::bloom.bloom_active = false;
}
const float Engine::Renderer::bloom::getBlurRadius() {
    return Engine::priv::Bloom::bloom.blur_radius;
}
const float Engine::Renderer::bloom::getBlurStrength() {
    return Engine::priv::Bloom::bloom.blur_strength;
}
void Engine::Renderer::bloom::setBlurRadius(const float r) {
    Engine::priv::Bloom::bloom.blur_radius = glm::max(0.0f, r);
}
void Engine::Renderer::bloom::setBlurStrength(const float r) {
    Engine::priv::Bloom::bloom.blur_strength = glm::max(0.0f, r);
}
const float Engine::Renderer::bloom::getScale() {
    return Engine::priv::Bloom::bloom.scale;
}
void Engine::Renderer::bloom::setScale(const float s) {
    Engine::priv::Bloom::bloom.scale = glm::max(0.0f, s);
}