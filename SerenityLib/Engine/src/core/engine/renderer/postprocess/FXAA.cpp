#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>

using namespace std;

Engine::priv::FXAA Engine::priv::FXAA::fxaa;

Engine::priv::FXAA::FXAA() {
    reduce_min = 0.0078125f; // (1 / 128)
    reduce_mul = 0.125f;     // (1 / 8)
    span_max   = 8.0f;
}
Engine::priv::FXAA::~FXAA() {
}
void Engine::priv::FXAA::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();

    Engine::Renderer::sendUniform1("FXAA_REDUCE_MIN", reduce_min);
    Engine::Renderer::sendUniform1("FXAA_REDUCE_MUL", reduce_mul);
    Engine::Renderer::sendUniform1("FXAA_SPAN_MAX", span_max);

    Engine::Renderer::sendUniform2("invRes", 1.0f / static_cast<float>(fboWidth), 1.0f / static_cast<float>(fboHeight));
    Engine::Renderer::sendTexture("inTexture", gbuffer.getTexture(sceneTextureEnum), 0);
    Engine::Renderer::sendTextureSafe("edgeTexture", gbuffer.getTexture(GBufferType::Misc), 1);
    Engine::Renderer::sendTexture("depthTexture", gbuffer.getTexture(GBufferType::Depth), 2);

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}
void Engine::Renderer::fxaa::setReduceMin(const float r) {
    Engine::priv::FXAA::fxaa.reduce_min = glm::max(0.0f, r);
}
void Engine::Renderer::fxaa::setReduceMul(const float r) {
    Engine::priv::FXAA::fxaa.reduce_mul = glm::max(0.0f, r);
}
void Engine::Renderer::fxaa::setSpanMax(const float r) {
    Engine::priv::FXAA::fxaa.span_max = glm::max(0.0f, r);
}
const float Engine::Renderer::fxaa::getReduceMin() {
    return Engine::priv::FXAA::fxaa.reduce_min;
}
const float Engine::Renderer::fxaa::getReduceMul() {
    return Engine::priv::FXAA::fxaa.reduce_mul;
}
const float Engine::Renderer::fxaa::getSpanMax() {
    return Engine::priv::FXAA::fxaa.span_max;
}