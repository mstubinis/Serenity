#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Camera.h>

#include <glm/vec4.hpp>
#include <random>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

SSAO SSAO::ssao;

SSAO::SSAO() {
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
SSAO::~SSAO() {
    glDeleteTextures(1, &m_ssao_noise_texture);
}
void SSAO::init() {
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
    Renderer::genAndBindTexture(GL_TEXTURE_2D, m_ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE, SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void SSAO::passSSAO(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, Camera& camera) {
    program.bind();
    if (RenderManager::GLSL_VERSION < 140) {
        Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
        Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
        Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
        Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
    }
    const float& divisor      = gbuffer.getSmallFBO()->divisor();
    const uint& screen_width  = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
    const uint& screen_height = static_cast<uint>(static_cast<float>(fboHeight) * divisor);

    Renderer::sendUniform2("ScreenSize", static_cast<float>(screen_width), static_cast<float>(screen_height));
    Renderer::sendUniform4("SSAOInfo", m_ssao_radius, m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Renderer::sendUniform4("SSAOInfoA", 0, 0, m_ssao_samples, SSAO_NORMALMAP_SIZE);//change to 4f eventually?

    Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0);
    Renderer::sendTexture("gRandomMap", m_ssao_noise_texture, 1, GL_TEXTURE_2D);
    Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);

    Renderer::renderFullscreenTriangle(screen_width, screen_height);
}
void SSAO::passBlur(ShaderProgram& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight, const string& type, const unsigned int& texture) {
    program.bind();
    glm::vec2 hv(0.0f);
    if (type == "H") { hv = glm::vec2(1.0f, 0.0f); }
    else { hv = glm::vec2(0.0f, 1.0f); }
    glm::ivec2 Res(fboWidth, fboHeight);
    Renderer::sendUniform1("strengthModifier", m_ssao_blur_strength);
    Renderer::sendUniform2("Resolution", Res);
    Renderer::sendUniform4("Data", m_ssao_blur_radius, 0.0f, hv.x, hv.y);
    Renderer::sendTexture("image", gbuffer.getTexture(texture), 0);

    const float& divisor = gbuffer.getSmallFBO()->divisor();
    const uint& x        = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
    const uint& y        = static_cast<uint>(static_cast<float>(fboHeight) * divisor);
    Renderer::renderFullscreenTriangle(x, y);
}
const bool Renderer::ssao::enabled() {
    return SSAO::ssao.m_ssao;
}
void Renderer::ssao::enable(const bool b) {
    SSAO::ssao.m_ssao = b;
}
void Renderer::ssao::disable() {
    SSAO::ssao.m_ssao = false;
}
void Renderer::ssao::enableBlur(const bool b) {
    SSAO::ssao.m_ssao_do_blur = b;
}
void Renderer::ssao::disableBlur() {
    SSAO::ssao.m_ssao_do_blur = false;
}
const float Renderer::ssao::getBlurRadius() {
    return SSAO::ssao.m_ssao_blur_radius;
}
void Renderer::ssao::setBlurRadius(const float r) {
    SSAO::ssao.m_ssao_blur_radius = glm::max(0.0f, r);
}
const float Renderer::ssao::getBlurStrength() {
    return SSAO::ssao.m_ssao_blur_strength;
}
void Renderer::ssao::setBlurStrength(const float s) {
    SSAO::ssao.m_ssao_blur_strength = glm::max(0.0f, s);
}
const float Renderer::ssao::getIntensity() {
    return SSAO::ssao.m_ssao_intensity;
}
void Renderer::ssao::setIntensity(const float i) {
    SSAO::ssao.m_ssao_intensity = glm::max(0.0f, i);
}
const float Renderer::ssao::getRadius() {
    return SSAO::ssao.m_ssao_radius;
}
void Renderer::ssao::setRadius(const float r) {
    SSAO::ssao.m_ssao_radius = glm::max(0.0f, r);
}
const float Renderer::ssao::getScale() {
    return SSAO::ssao.m_ssao_scale;
}
void Renderer::ssao::setScale(const float s) {
    SSAO::ssao.m_ssao_scale = glm::max(0.0f, s);
}
const float Renderer::ssao::getBias() {
    return SSAO::ssao.m_ssao_bias;
}
void Renderer::ssao::setBias(const float b) {
    SSAO::ssao.m_ssao_bias = b;
}
const unsigned int Renderer::ssao::getSamples() {
    return SSAO::ssao.m_ssao_samples;
}
void Renderer::ssao::setSamples(const unsigned int s) {
    const auto samples = glm::max(0, static_cast<int>(s));
    SSAO::ssao.m_ssao_samples = glm::clamp(samples, 0, SSAO_MAX_KERNEL_SIZE);
}