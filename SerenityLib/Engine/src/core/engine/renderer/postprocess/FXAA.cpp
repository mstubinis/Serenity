#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

FXAA FXAA::fxaa;

FXAA::FXAA() {
    reduce_min = 0.0078125f; // (1 / 128)
    reduce_mul = 0.125f;     // (1 / 8)
    span_max   = 8.0f;
}
FXAA::~FXAA() {
}
void FXAA::pass(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum) {
    program.bind();

    Renderer::sendUniform1("FXAA_REDUCE_MIN", reduce_min);
    Renderer::sendUniform1("FXAA_REDUCE_MUL", reduce_mul);
    Renderer::sendUniform1("FXAA_SPAN_MAX", span_max);

    Renderer::sendUniform2("invRes", 1.0f / static_cast<float>(fboWidth), 1.0f / static_cast<float>(fboHeight));
    Renderer::sendTexture("inTexture", gbuffer.getTexture(sceneTextureEnum), 0);
    Renderer::sendTextureSafe("edgeTexture", gbuffer.getTexture(GBufferType::Misc), 1);
    Renderer::sendTexture("depthTexture", gbuffer.getTexture(GBufferType::Depth), 2);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}
void Renderer::fxaa::setReduceMin(const float r) {
    FXAA::fxaa.reduce_min = glm::max(0.0f, r);
}
void Renderer::fxaa::setReduceMul(const float r) {
    FXAA::fxaa.reduce_mul = glm::max(0.0f, r);
}
void Renderer::fxaa::setSpanMax(const float r) {
    FXAA::fxaa.span_max = glm::max(0.0f, r);
}
const float Renderer::fxaa::getReduceMin() {
    return FXAA::fxaa.reduce_min;
}
const float Renderer::fxaa::getReduceMul() {
    return FXAA::fxaa.reduce_mul;
}
const float Renderer::fxaa::getSpanMax() {
    return FXAA::fxaa.span_max;
}