#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/resources/Engine_Resources.h>

#include <glm/vec4.hpp>
#include <random>

using namespace std;

Engine::priv::SSAO Engine::priv::SSAO::ssao;

Engine::priv::SSAO::~SSAO() {
    glDeleteTextures(1, &m_ssao_noise_texture);
    SAFE_DELETE(m_Shader_Program);
    SAFE_DELETE(m_Fragment_Shader);
    SAFE_DELETE(m_Vertex_Shader);

    SAFE_DELETE(m_Vertex_Shader_Blur);
    SAFE_DELETE(m_Fragment_Shader_Blur);
    SAFE_DELETE(m_Shader_Program_Blur);
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
const bool Engine::priv::SSAO::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

    m_GLSL_frag_code =
        "USE_LOG_DEPTH_FRAG_WORLD_POSITION\n"
        "\n"
        "uniform sampler2D gNormalMap;\n"
        "uniform sampler2D gRandomMap;\n"
        "uniform sampler2D gDepthMap;\n"
        "\n"
        "uniform vec2  ScreenSize;\n"
        "uniform vec4  SSAOInfo;\n"  //   x = radius     y = intensity    z = bias        w = scale
        "uniform ivec4 SSAOInfoA;\n"//    x = UNUSED     y = UNUSED       z = Samples     w = NoiseTextureSize
        "\n"
        "varying vec2 texcoords;\n"
        "void main(){\n"
        "    vec3 Pos = GetViewPosition(texcoords, CameraNear, CameraFar);\n"
        "    vec3 Normal = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);\n"
        "    Normal = GetViewNormalsFromWorld(Normal, CameraView);\n"
        "    vec2 RandVector = normalize(texture2D(gRandomMap, ScreenSize * texcoords / SSAOInfoA.w).xy);\n"
        //"  float CamZ = distance(Pos, CameraPosition);\n"
        "    float Radius = SSAOInfo.x / max(Pos.z, 100.0);\n"
        //"  float Radius = SSAOInfo.x / Pos.z;\n"
        "    gl_FragColor.a = SSAOExecute(texcoords, SSAOInfoA.z, SSAOInfoA.w, RandVector, Radius, Pos, Normal, SSAOInfo.y, SSAOInfo.z, SSAOInfo.w);\n"
        "}";

    m_GLSL_frag_code_blur =
        "uniform sampler2D image;\n"
        "uniform vec4 Data;\n"//radius, strengthModifier, H,V
        "uniform vec2 inverseResolution;\n"
        "\n"
        "varying vec2 texcoords;\n"
        "\n"
        "const int NUM_SAMPLES = 9;\n"
        "const float weight[NUM_SAMPLES] = float[](0.227,0.21,0.1946,0.162,0.12,0.08,0.054,0.03,0.016);\n"
        "\n"
        "void main(){\n"
        "    float Sum = 0.0;\n"
        "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
        "        vec2 offset = (inverseResolution * float(i)) * Data.x;\n"
        "        Sum += texture2D(image, texcoords + vec2(offset.x * Data.z, offset.y * Data.w)).a * weight[i] * Data.y;\n"
        "        Sum += texture2D(image, texcoords - vec2(offset.x * Data.z, offset.y * Data.w)).a * weight[i] * Data.y;\n"
        "    }\n"
        "    gl_FragColor.a = Sum;\n"
        "}";

    auto lambda_part_a = [&]() {
        m_Vertex_Shader   = NEW Shader(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = NEW Shader(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program  = NEW ShaderProgram("SSAO", *m_Vertex_Shader, *m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);


    auto lambda_part_a_blur = [&]() {
        m_Vertex_Shader_Blur   = NEW Shader(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader_Blur = NEW Shader(m_GLSL_frag_code_blur, ShaderType::Fragment, false);
    };
    auto lambda_part_b_blur = [&]() {
        m_Shader_Program_Blur  = NEW ShaderProgram("SSAO_Blur", *m_Vertex_Shader_Blur, *m_Fragment_Shader_Blur);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a_blur, lambda_part_b_blur);

    return true;
}
void Engine::priv::SSAO::passSSAO(GBuffer& gbuffer, const Viewport& viewport, const Camera& camera) {
    m_Shader_Program->bind();
    const auto& dimensions    = viewport.getViewportDimensions();
    if (Renderer::GLSL_VERSION < 140) {
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
        Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
        Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
    }
    const float divisor       = gbuffer.getSmallFBO().divisor();
    const float screen_width  = dimensions.z * divisor;
    const float screen_height = dimensions.w * divisor;

    Engine::Renderer::sendUniform2("ScreenSize", screen_width, screen_height);
    Engine::Renderer::sendUniform4("SSAOInfo", m_ssao_radius, m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Engine::Renderer::sendUniform4("SSAOInfoA", 0, 0, static_cast<int>(m_ssao_samples), static_cast<int>(SSAO_NORMALMAP_SIZE));//change to 4f eventually?

    Engine::Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::sendTexture("gRandomMap", m_ssao_noise_texture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);

    Engine::Renderer::renderFullscreenQuad();
}
void Engine::priv::SSAO::passBlur(GBuffer& gbuffer, const Viewport& viewport, const string& type, const unsigned int texture) {
    m_Shader_Program_Blur->bind();
    const auto& dimensions = viewport.getViewportDimensions();
    glm::vec2 hv(0.0f);
    if (type == "H") { 
        hv = glm::vec2(1.0f, 0.0f); 
    }else{ 
        hv = glm::vec2(0.0f, 1.0f); 
    }
    const glm::vec2 inverseResolution(1.0f / dimensions.z, 1.0f / dimensions.w);
    Engine::Renderer::sendUniform4("Data", m_ssao_blur_radius, m_ssao_blur_strength, hv.x, hv.y);
    Engine::Renderer::sendUniform2("inverseResolution", inverseResolution);
    Engine::Renderer::sendTexture("image", gbuffer.getTexture(texture), 0);

    Engine::Renderer::renderFullscreenQuad();
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
    const unsigned int samples = glm::max(0U, s);
    Engine::priv::SSAO::ssao.m_ssao_samples = glm::clamp(samples, 0U, SSAO_MAX_KERNEL_SIZE);
}