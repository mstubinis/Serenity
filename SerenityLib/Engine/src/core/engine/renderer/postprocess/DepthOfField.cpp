#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>

using namespace std;

Engine::priv::DepthOfField Engine::priv::DepthOfField::DOF;

Engine::priv::DepthOfField::DepthOfField() {
    bias         = 0.6f;
    focus        = 2.0f;
    blur_radius  = 3.0f;
    dof          = false;
}
Engine::priv::DepthOfField::~DepthOfField() {
}
void Engine::priv::DepthOfField::pass(ShaderProgram& program,GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    program.bind();
    Engine::Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, fboWidth / static_cast<float>(fboHeight));

    Engine::Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}

void Engine::Renderer::depthOfField::enable(const bool b) {
    Engine::priv::DepthOfField::DOF.dof = b;
}
void Engine::Renderer::depthOfField::disable() {
    Engine::priv::DepthOfField::DOF.dof = false;
}
const bool Engine::Renderer::depthOfField::enabled() {
    return Engine::priv::DepthOfField::DOF.dof;
}
const float Engine::Renderer::depthOfField::getFocus() {
    return Engine::priv::DepthOfField::DOF.focus;
}
void Engine::Renderer::depthOfField::setFocus(const float f) {
    Engine::priv::DepthOfField::DOF.focus = glm::max(0.0f, f);
}
const float Engine::Renderer::depthOfField::getBias() {
    return Engine::priv::DepthOfField::DOF.bias;
}
void Engine::Renderer::depthOfField::setBias(const float b) {
    Engine::priv::DepthOfField::DOF.bias = b;
}
const float Engine::Renderer::depthOfField::getBlurRadius() {
    return Engine::priv::DepthOfField::DOF.blur_radius;
}
void Engine::Renderer::depthOfField::setBlurRadius(const float r) {
    Engine::priv::DepthOfField::DOF.blur_radius = glm::max(0.0f, r);
}