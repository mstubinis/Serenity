#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Camera.h>

#include <glm/vec4.hpp>
#include <random>

using namespace std;

Engine::priv::SSAO Engine::priv::SSAO::ssao;

Engine::priv::SSAO::SSAO() {
    m_ssao                 = true;
    m_ssao_do_blur         = true;
    m_ssao_samples         = 8;
    m_ssao_blur_num_passes = 2;
    m_ssao_blur_radius     = 0.66f;
    m_ssao_blur_strength   = 0.48f;
    m_ssao_scale           = 1.0f;
    m_ssao_intensity       = 1.8f;
    m_ssao_bias            = 0.048f;
    m_ssao_radius          = 0.175f;
    m_ssao_noise_texture   = 0;
}
Engine::priv::SSAO::~SSAO() {
    glDeleteTextures(1, &m_ssao_noise_texture);
}
void Engine::priv::SSAO::init() {
    uniform_real_distribution<float> rand(0.0f, 1.0f);
    default_random_engine gen;
    /*
    for (uint i = 0; i < SSAO_MAX_KERNEL_SIZE; ++i) {
        glm::vec3 sample(rand(gen) * 2.0f - 1.0f, rand(gen) * 2.0f - 1.0f, rand(gen));
        sample = glm::normalize(sample);
        sample *= rand(gen);
        float scale = static_cast<float>(i) / static_cast<float>(SSAO_MAX_KERNEL_SIZE);
        const float a = 0.1f;
        const float b = 1.0f;
        const float f = scale * scale;
        scale = a + f * (b - a);
        sample *= scale;
        m_ssao_Kernels[i] = sample;
    }
    */
    vector<glm::vec3> ssaoNoise;
    ssaoNoise.reserve(SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE);
    for (uint i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i) {
        ssaoNoise.emplace_back(rand(gen) * 2.0 - 1.0, rand(gen) * 2.0 - 1.0, 0.0f);
    }
    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, m_ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE, SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void Engine::priv::SSAO::passSSAO(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, Camera& camera) {
    program.bind();
    if (Renderer::GLSL_VERSION < 140) {
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
        Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
        Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
    }
    const float& divisor      = gbuffer.getSmallFBO()->divisor();
    const uint& screen_width  = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
    const uint& screen_height = static_cast<uint>(static_cast<float>(fboHeight) * divisor);

    Engine::Renderer::sendUniform2("ScreenSize", static_cast<float>(screen_width), static_cast<float>(screen_height));
    Engine::Renderer::sendUniform4("SSAOInfo", m_ssao_radius, m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Engine::Renderer::sendUniform4("SSAOInfoA", 0, 0, m_ssao_samples, SSAO_NORMALMAP_SIZE);//change to 4f eventually?

    Engine::Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::sendTexture("gRandomMap", m_ssao_noise_texture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);

    Engine::Renderer::renderFullscreenTriangle(screen_width, screen_height);
}
void Engine::priv::SSAO::passBlur(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const string& type, const unsigned int& texture) {
    program.bind();
    glm::vec2 hv(0.0f);
    if (type == "H") { hv = glm::vec2(1.0f, 0.0f); }
    else { hv = glm::vec2(0.0f, 1.0f); }
    glm::ivec2 Res(fboWidth, fboHeight);
    Engine::Renderer::sendUniform1("strengthModifier", m_ssao_blur_strength);
    Engine::Renderer::sendUniform2("Resolution", Res);
    Engine::Renderer::sendUniform4("Data", m_ssao_blur_radius, 0.0f, hv.x, hv.y);
    Engine::Renderer::sendTexture("image", gbuffer.getTexture(texture), 0);

    const float& divisor = gbuffer.getSmallFBO()->divisor();
    const uint& x        = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
    const uint& y        = static_cast<uint>(static_cast<float>(fboHeight) * divisor);
    Engine::Renderer::renderFullscreenTriangle(x, y);
}
const bool Engine::Renderer::ssao::enabled() {
    return Engine::priv::SSAO::ssao.m_ssao;
}
void Engine::Renderer::ssao::enable(const bool b) {
    Engine::priv::SSAO::ssao.m_ssao = b;
}
void Engine::Renderer::ssao::disable() {
    Engine::priv::SSAO::ssao.m_ssao = false;
}
void Engine::Renderer::ssao::enableBlur(const bool b) {
    Engine::priv::SSAO::ssao.m_ssao_do_blur = b;
}
void Engine::Renderer::ssao::disableBlur() {
    Engine::priv::SSAO::ssao.m_ssao_do_blur = false;
}
const float Engine::Renderer::ssao::getBlurRadius() {
    return Engine::priv::SSAO::ssao.m_ssao_blur_radius;
}
void Engine::Renderer::ssao::setBlurRadius(const float r) {
    Engine::priv::SSAO::ssao.m_ssao_blur_radius = glm::max(0.0f, r);
}
const float Engine::Renderer::ssao::getBlurStrength() {
    return Engine::priv::SSAO::ssao.m_ssao_blur_strength;
}
void Engine::Renderer::ssao::setBlurStrength(const float s) {
    Engine::priv::SSAO::ssao.m_ssao_blur_strength = glm::max(0.0f, s);
}
const float Engine::Renderer::ssao::getIntensity() {
    return Engine::priv::SSAO::ssao.m_ssao_intensity;
}
void Engine::Renderer::ssao::setIntensity(const float i) {
    Engine::priv::SSAO::ssao.m_ssao_intensity = glm::max(0.0f, i);
}
const float Engine::Renderer::ssao::getRadius() {
    return Engine::priv::SSAO::ssao.m_ssao_radius;
}
void Engine::Renderer::ssao::setRadius(const float r) {
    Engine::priv::SSAO::ssao.m_ssao_radius = glm::max(0.0f, r);
}
const float Engine::Renderer::ssao::getScale() {
    return Engine::priv::SSAO::ssao.m_ssao_scale;
}
void Engine::Renderer::ssao::setScale(const float s) {
    Engine::priv::SSAO::ssao.m_ssao_scale = glm::max(0.0f, s);
}
const float Engine::Renderer::ssao::getBias() {
    return Engine::priv::SSAO::ssao.m_ssao_bias;
}
void Engine::Renderer::ssao::setBias(const float b) {
    Engine::priv::SSAO::ssao.m_ssao_bias = b;
}
const unsigned int Engine::Renderer::ssao::getSamples() {
    return Engine::priv::SSAO::ssao.m_ssao_samples;
}
void Engine::Renderer::ssao::setSamples(const unsigned int s) {
    const auto samples = glm::max(0, static_cast<int>(s));
    Engine::priv::SSAO::ssao.m_ssao_samples = glm::clamp(samples, 0, SSAO_MAX_KERNEL_SIZE);
}