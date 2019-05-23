#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/ShaderProgram.h>

using namespace Engine;
using namespace std;

epriv::Postprocess_DepthOfField epriv::Postprocess_DepthOfField::DOF;

epriv::Postprocess_DepthOfField::Postprocess_DepthOfField() {
    bias        = 0.6f;
    focus       = 2.0f;
    blur_radius = 3.0f;
    dof         = false;
}
epriv::Postprocess_DepthOfField::~Postprocess_DepthOfField() {
}
void epriv::Postprocess_DepthOfField::pass(ShaderP& program,GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    program.bind();
    Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, fboWidth / (float)fboHeight);

    Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}


void Engine::Renderer::depthOfField::enable(bool b) {
    epriv::Postprocess_DepthOfField::DOF.dof = b;
}
void Engine::Renderer::depthOfField::disable() {
    epriv::Postprocess_DepthOfField::DOF.dof = false;
}
bool Engine::Renderer::depthOfField::enabled() {
    return epriv::Postprocess_DepthOfField::DOF.dof;
}
float Engine::Renderer::depthOfField::getFocus() {
    return epriv::Postprocess_DepthOfField::DOF.focus;
}
void Engine::Renderer::depthOfField::setFocus(float f) {
    epriv::Postprocess_DepthOfField::DOF.focus = glm::max(0.0f, f);
}
float Engine::Renderer::depthOfField::getBias() {
    return epriv::Postprocess_DepthOfField::DOF.bias;
}
void Engine::Renderer::depthOfField::setBias(float b) {
    epriv::Postprocess_DepthOfField::DOF.bias = b;
}
float Engine::Renderer::depthOfField::getBlurRadius() {
    return epriv::Postprocess_DepthOfField::DOF.blur_radius;
}
void Engine::Renderer::depthOfField::setBlurRadius(float r) {
    epriv::Postprocess_DepthOfField::DOF.blur_radius = glm::max(0.0f, r);
}