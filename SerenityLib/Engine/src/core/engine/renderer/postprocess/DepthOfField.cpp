#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

DepthOfField DepthOfField::DOF;

DepthOfField::DepthOfField() {
    bias         = 0.6f;
    focus        = 2.0f;
    blur_radius  = 3.0f;
    dof          = false;
}
DepthOfField::~DepthOfField() {
}
void DepthOfField::pass(ShaderProgram& program,GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    program.bind();
    Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, fboWidth / static_cast<float>(fboHeight));

    Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}

void Renderer::depthOfField::enable(const bool b) {
    DepthOfField::DOF.dof = b;
}
void Renderer::depthOfField::disable() {
    DepthOfField::DOF.dof = false;
}
const bool Renderer::depthOfField::enabled() {
    return DepthOfField::DOF.dof;
}
const float Renderer::depthOfField::getFocus() {
    return DepthOfField::DOF.focus;
}
void Renderer::depthOfField::setFocus(const float f) {
    DepthOfField::DOF.focus = glm::max(0.0f, f);
}
const float Renderer::depthOfField::getBias() {
    return DepthOfField::DOF.bias;
}
void Renderer::depthOfField::setBias(const float b) {
    DepthOfField::DOF.bias = b;
}
const float Renderer::depthOfField::getBlurRadius() {
    return DepthOfField::DOF.blur_radius;
}
void Renderer::depthOfField::setBlurRadius(const float r) {
    DepthOfField::DOF.blur_radius = glm::max(0.0f, r);
}