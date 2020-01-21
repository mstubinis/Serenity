#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>

using namespace std;

Engine::priv::HDR Engine::priv::HDR::hdr;

Engine::priv::HDR::HDR() {
    hdr_active = true;
    exposure   = 3.0f;
    algorithm  = HDRAlgorithm::Uncharted;
}
Engine::priv::HDR::~HDR() {
}
void Engine::priv::HDR::pass(ShaderProgram& program, Engine::priv::GBuffer& gbuffer,const unsigned int& fboWidth,const unsigned int& fboHeight,const bool& godRays,const bool& lighting,const float& godRaysFactor) {
    program.bind();

    Engine::Renderer::sendUniform4Safe("HDRInfo", exposure, static_cast<float>(hdr_active), godRaysFactor, static_cast<float>(algorithm));
    Engine::Renderer::sendUniform2Safe("Has", static_cast<int>(godRays), static_cast<int>(lighting));

    Engine::Renderer::sendTextureSafe("lightingBuffer", gbuffer.getTexture(GBufferType::Lighting), 0);
    Engine::Renderer::sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 1);
    Engine::Renderer::sendTextureSafe("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 2);
    Engine::Renderer::sendTextureSafe("gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 3);

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}
const bool Engine::Renderer::hdr::enabled() {
    return Engine::priv::HDR::hdr.hdr_active;
}
void Engine::Renderer::hdr::enable(const bool b) {
    Engine::priv::HDR::hdr.hdr_active = b;
}
void Engine::Renderer::hdr::disable() {
    Engine::priv::HDR::hdr.hdr_active = false;
}
const float Engine::Renderer::hdr::getExposure() {
    return Engine::priv::HDR::hdr.exposure;
}
void Engine::Renderer::hdr::setExposure(const float e) {
    Engine::priv::HDR::hdr.exposure = e;
}
void Engine::Renderer::hdr::setAlgorithm(const HDRAlgorithm::Algorithm a) {
    Engine::priv::HDR::hdr.algorithm = a;
}