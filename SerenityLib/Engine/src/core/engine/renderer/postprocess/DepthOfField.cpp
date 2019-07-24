#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/ShaderProgram.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Postprocess_DepthOfField Postprocess_DepthOfField::DOF;

Postprocess_DepthOfField::Postprocess_DepthOfField() {
    bias         = 0.6f;
    focus        = 2.0f;
    blur_radius  = 3.0f;
    dof          = false;
}
Postprocess_DepthOfField::~Postprocess_DepthOfField() {
}
void Postprocess_DepthOfField::pass(ShaderP& program,GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    program.bind();
    Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, fboWidth / static_cast<float>(fboHeight));

    Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}

void Renderer::depthOfField::enable(const bool b) {
    Postprocess_DepthOfField::DOF.dof = b;
}
void Renderer::depthOfField::disable() {
    Postprocess_DepthOfField::DOF.dof = false;
}
const bool Renderer::depthOfField::enabled() {
    return Postprocess_DepthOfField::DOF.dof;
}
const float Renderer::depthOfField::getFocus() {
    return Postprocess_DepthOfField::DOF.focus;
}
void Renderer::depthOfField::setFocus(const float f) {
    Postprocess_DepthOfField::DOF.focus = glm::max(0.0f, f);
}
const float Renderer::depthOfField::getBias() {
    return Postprocess_DepthOfField::DOF.bias;
}
void Renderer::depthOfField::setBias(const float b) {
    Postprocess_DepthOfField::DOF.bias = b;
}
const float Renderer::depthOfField::getBlurRadius() {
    return Postprocess_DepthOfField::DOF.blur_radius;
}
void Renderer::depthOfField::setBlurRadius(const float r) {
    Postprocess_DepthOfField::DOF.blur_radius = glm::max(0.0f, r);
}