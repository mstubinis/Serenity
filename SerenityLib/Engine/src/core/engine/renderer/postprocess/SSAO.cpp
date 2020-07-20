#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace std;
using namespace Engine::priv;

Engine::priv::SSAO Engine::priv::SSAO::STATIC_SSAO;

Engine::priv::SSAO::~SSAO() {
    glDeleteTextures(1, &m_ssao_noise_texture);
}
void Engine::priv::SSAO::internal_generate_kernel(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
    for (std::uint32_t i = 0; i < SSAO_MAX_KERNEL_SIZE; ++i) {
        glm::vec3 sample(rand_dist(gen) * 2.0f - 1.0f, rand_dist(gen) * 2.0f - 1.0f, rand_dist(gen));
        sample      = glm::normalize(sample);
        sample     *= rand_dist(gen);
        float scale = (float)i / (float)SSAO_MAX_KERNEL_SIZE;
        float a     = 0.1f;
        float b     = 1.0f;
        float f     = scale * scale;
        scale       = a + f * (b - a);
        sample     *= scale;
        //m_ssao_Kernels[i] = sample;
    }
}
void Engine::priv::SSAO::internal_generate_noise(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
    std::vector<glm::vec3> ssaoNoise;
    ssaoNoise.reserve(SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE);
    for (std::uint32_t i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i) {
        ssaoNoise.emplace_back(rand_dist(gen) * 2.0 - 1.0, rand_dist(gen) * 2.0 - 1.0, 0.0f);
    }
    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, m_ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE, SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void Engine::priv::SSAO::init() {
    uniform_real_distribution<float> rand_dist(0.0f, 1.0f);
    default_random_engine gen;

    //internal_generate_kernel(rand_dist, gen);
    internal_generate_noise(rand_dist, gen);

    Engine::Renderer::ssao::setLevel(m_SSAOLevel);
}
bool Engine::priv::SSAO::init_shaders() {
    if (!m_GLSL_frag_code.empty()) {
        return false;
    }
    m_GLSL_frag_code =
        "USE_LOG_DEPTH_FRAG_WORLD_POSITION\n"
        "\n"
        "uniform SAMPLER_TYPE_2D gNormalMap;\n"
        "uniform SAMPLER_TYPE_2D gRandomMap;\n"
        "uniform SAMPLER_TYPE_2D gDepthMap;\n"
        "\n"
        "uniform vec2  ScreenSize;\n"
        "uniform vec4  SSAOInfo;\n"  //   x = radius     y = intensity    z = bias        w = scale
        "uniform ivec4 SSAOInfoA;\n"//    x = UNUSED     y = UNUSED       z = Samples     w = NoiseTextureSize
        "\n"
        "varying vec2 texcoords;\n"
        "void main(){\n"
        "    vec3 Pos = GetViewPosition(USE_SAMPLER_2D(gDepthMap), texcoords, CameraNear, CameraFar);\n"
        "    vec3 Normal = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), texcoords).rg);\n"
        "    Normal = GetViewNormalsFromWorld(Normal, CameraView);\n"
        "    vec2 RandVector = normalize(texture2D(USE_SAMPLER_2D(gRandomMap), ScreenSize * texcoords / SSAOInfoA.w).xy);\n"
        //"  float CamZ = distance(Pos, CameraPosition);\n"
        "    float Radius = SSAOInfo.x / max(Pos.z, 100.0);\n"
        //"  float Radius = SSAOInfo.x / Pos.z;\n"
        "    gl_FragColor.a = SSAOExecute(USE_SAMPLER_2D(gDepthMap), texcoords, SSAOInfoA.z, SSAOInfoA.w, RandVector, Radius, Pos, Normal, SSAOInfo.y, SSAOInfo.z, SSAOInfo.w);\n"
        "}";

    m_GLSL_frag_code_blur =
        "uniform sampler2D image;\n"
        "uniform vec4 Data;\n"//radius, strengthModifier, H,V
        "\n"
        "varying vec2 texcoords;\n"
        "\n"
        "const int NUM_SAMPLES = 9;\n"
        "const float weight[NUM_SAMPLES] = float[](0.227,0.21,0.1946,0.162,0.12,0.08,0.054,0.03,0.016);\n"
        "\n"
        "void main(){\n"
        "    float Sum = 0.0;\n"
        "    vec2 inverseResolution = vec2(1.0) / vec2(ScreenInfo.z, ScreenInfo.w);\n"
        "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
        "        vec2 offset = (inverseResolution * float(i)) * Data.x;\n";

    string varName = "image";
    if (OpenGLExtensions::isBindlessTexturesSupported()) {
        varName = "imageSampler";
        m_GLSL_frag_code_blur += "sampler2D " + varName + " = USE_SAMPLER_2D(image);\n";
    }

    m_GLSL_frag_code_blur +=
        "        Sum += texture2D(" + varName + ", texcoords + vec2(offset.x * Data.z, offset.y * Data.w)).a * weight[i] * Data.y;\n"
        "        Sum += texture2D(" + varName + ", texcoords - vec2(offset.x * Data.z, offset.y * Data.w)).a * weight[i] * Data.y;\n";


    m_GLSL_frag_code_blur +=
        "    }\n"
        "    gl_FragColor.a = Sum;\n"
        "}";

    auto lambda_part_a = [&]() {
        m_Vertex_Shader   = std::make_unique<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = std::make_unique<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program  = std::make_unique<ShaderProgram>("SSAO", *m_Vertex_Shader, *m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);


    auto lambda_part_a_blur = [&]() {
        m_Vertex_Shader_Blur   = std::make_unique<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader_Blur = std::make_unique<Shader>(m_GLSL_frag_code_blur, ShaderType::Fragment, false);
    };
    auto lambda_part_b_blur = [&]() {
        m_Shader_Program_Blur  = std::make_unique<ShaderProgram>("SSAO_Blur", *m_Vertex_Shader_Blur, *m_Fragment_Shader_Blur);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a_blur, lambda_part_b_blur);

    return true;
}
void Engine::priv::SSAO::passSSAO(GBuffer& gbuffer, const Viewport& viewport, const Camera& camera, const Engine::priv::Renderer& renderer) {
    renderer.bind(m_Shader_Program.get());
    auto& dimensions    = viewport.getViewportDimensions();
    float divisor       = gbuffer.getSmallFBO().divisor();
    float screen_width  = dimensions.z * divisor;
    float screen_height = dimensions.w * divisor;

    Engine::Renderer::sendUniform2("ScreenSize", screen_width, screen_height);
    Engine::Renderer::sendUniform4("SSAOInfo", m_ssao_radius, m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Engine::Renderer::sendUniform4("SSAOInfoA", 0, 0, (int)m_ssao_samples, (int)SSAO_NORMALMAP_SIZE); //change to 4f eventually?

    Engine::Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::sendTexture("gRandomMap", m_ssao_noise_texture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);

    Engine::Renderer::renderFullscreenQuad();
}
void Engine::priv::SSAO::passBlur(GBuffer& gbuffer, const Viewport& viewport, string_view type, unsigned int texture, const Engine::priv::Renderer& renderer) {
    renderer.bind(m_Shader_Program_Blur.get());
    glm::vec2 hv(0.0f);
    if (type == "H") { 
        hv = glm::vec2(1.0f, 0.0f); 
    }else{ 
        hv = glm::vec2(0.0f, 1.0f); 
    }
    Engine::Renderer::sendUniform4("Data", m_ssao_blur_radius, m_ssao_blur_strength, hv.x, hv.y);
    Engine::Renderer::sendTexture("image", gbuffer.getTexture(texture), 0);

    Engine::Renderer::renderFullscreenQuad();
}

void Engine::Renderer::ssao::setLevel(const SSAOLevel::Level level) {
    Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel = level;
    switch (level) {
        case SSAOLevel::Off: {
            break;
        }case SSAOLevel::Low: {
            setSamples(4);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);       
            break;
        }case SSAOLevel::Medium: {
            setSamples(8);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        }case SSAOLevel::High: {
            setSamples(12);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        }case SSAOLevel::Ultra: {
            setSamples(16);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        }default: {
            break;
        }
    }
}
void Engine::Renderer::ssao::enableBlur(bool b) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = b;
}
void Engine::Renderer::ssao::disableBlur() {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = false;
}
float Engine::Renderer::ssao::getBlurRadius() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius;
}
void Engine::Renderer::ssao::setBlurRadius(float r) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius = glm::max(0.0f, r);
}
float Engine::Renderer::ssao::getBlurStrength() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength;
}
void Engine::Renderer::ssao::setBlurStrength(float s) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength = glm::max(0.0f, s);
}
float Engine::Renderer::ssao::getIntensity() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity;
}
void Engine::Renderer::ssao::setIntensity(float i) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity = glm::max(0.0f, i);
}
float Engine::Renderer::ssao::getRadius() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius;
}
void Engine::Renderer::ssao::setRadius(float r) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius = glm::max(0.0f, r);
}
float Engine::Renderer::ssao::getScale() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale;
}
void Engine::Renderer::ssao::setScale(float s) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale = glm::max(0.0f, s);
}
float Engine::Renderer::ssao::getBias() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias;
}
void Engine::Renderer::ssao::setBias(float b) {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias = b;
}
unsigned int Engine::Renderer::ssao::getSamples() {
    return Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples;
}
void Engine::Renderer::ssao::setSamples(unsigned int s) {
    s = glm::max(0U, s);
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples = glm::clamp(s, 0U, SSAO_MAX_KERNEL_SIZE);
}