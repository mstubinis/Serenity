
#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/renderer/opengl/BindTextureRAII.h>

using namespace Engine::priv;

Engine::priv::SSAO Engine::priv::SSAO::STATIC_SSAO;

namespace {

    //std::array<glm::vec3, SSAO_MAX_KERNEL_SIZE>       SSAO_KERNELS;

    std::string  GLSL_FRAG_CODE;
    std::string  GLSL_FRAG_CODE_BLUR;
    uint32_t     GL_NOISE_TEXTURE = 0;
    Handle       m_Vertex_Shader;
    Handle       m_Fragment_Shader;
    Handle       m_Shader_Program;
    Handle       m_Vertex_Shader_Blur;
    Handle       m_Fragment_Shader_Blur;
    Handle       m_Shader_Program_Blur;

    constexpr int SSAO_NORMALMAP_SIZE  = 16;

    void internal_generate_kernel(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
        /*
        for (uint32_t i = 0; i < SSAO_KERNELS.size(); ++i) {
            glm::vec3 sample(rand_dist(gen) * 2.0f - 1.0f, rand_dist(gen) * 2.0f - 1.0f, rand_dist(gen));
            sample = glm::normalize(sample);
            sample *= rand_dist(gen);
            float scale = float(i) / float(SSAO_KERNELS.size());
            float a = 0.1f;
            float b = 1.0f;
            float f = scale * scale;
            scale = a + f * (b - a);
            sample *= scale;
            SSAO_KERNELS[i] = sample;
        }
        */
    }
    void internal_generate_noise(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
        auto ssaoNoise = Engine::create_and_reserve<std::vector<glm::vec3>>(SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE);
        for (uint32_t i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i) {
            ssaoNoise.emplace_back(rand_dist(gen) * 2.0 - 1.0, rand_dist(gen) * 2.0 - 1.0, 0.0f);
        }
        const TextureType textureType = TextureType::Texture2D;
        Engine::Renderer::genAndBindTexture(textureType, GL_NOISE_TEXTURE);
        glTexImage2D(textureType.toGLType(), 0, GL_RGB16F, SSAO_NORMALMAP_SIZE, SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
        Texture::setFilter(textureType, TextureFilter::Nearest);
        TextureBaseClass::setWrapping(textureType, TextureWrap::Repeat);
    }
    void internal_init_blur_fragment_code() {
        GLSL_FRAG_CODE_BLUR = R"(
uniform SAMPLER_TYPE_2D uSceneToBeSSAOBlurred;
uniform vec4 Data;
uniform vec2  ScreenSize;

const int NUM_SAMPLES = 9;
const float WEIGHT[NUM_SAMPLES] = float[](0.227, 0.21, 0.1946, 0.162, 0.12, 0.08, 0.054, 0.03, 0.016);

varying vec2 texcoords;

void main() {
    vec2 uvs = texcoords * (ScreenInfo.zw / ScreenInfo.xy);
    float Sum = 0.0;
    vec2 inverseResolution = vec2(1.0) / ScreenInfo.xy;
    for(int i = 0; i < NUM_SAMPLES; ++i) {
        vec2 offset = (inverseResolution * float(i)) * Data.x;
        Sum += texture2D(USE_SAMPLER_2D(uSceneToBeSSAOBlurred), uvs + vec2(offset.x * Data.z, offset.y * Data.w)).a * WEIGHT[i] * Data.y;
        Sum += texture2D(USE_SAMPLER_2D(uSceneToBeSSAOBlurred), uvs - vec2(offset.x * Data.z, offset.y * Data.w)).a * WEIGHT[i] * Data.y;
    }
    gl_FragColor.a = Sum;
}
)";
    }
    void internal_init_fragment_code() {
        GLSL_FRAG_CODE = R"(
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gRandomMap;
uniform SAMPLER_TYPE_2D gDepthMap;

uniform vec2  ScreenSize;
uniform vec4  SSAOInfo;  //   x = radius     y = intensity    z = bias        w = scale
uniform ivec4 SSAOInfoA;//    x = UNUSED     y = UNUSED       z = Samples     w = NoiseTextureSize

varying vec2 texcoords;

void main() {
    vec2 uvs = texcoords * (ScreenInfo.zw / ScreenInfo.xy);
    vec3 Pos = GetViewPosition(USE_SAMPLER_2D(gDepthMap), uvs, CameraNear, CameraFar);
    vec3 Normal = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uvs).rg);
    Normal = GetViewNormalsFromWorld(Normal, CameraView);
    vec2 RandVector = normalize(texture2D(USE_SAMPLER_2D(gRandomMap), ScreenSize * uvs / SSAOInfoA.w).xy);
    float Radius = SSAOInfo.x / max(Pos.z, 100.0);
    gl_FragColor.a = SSAOExecute(USE_SAMPLER_2D(gDepthMap), uvs, SSAOInfoA.z, SSAOInfoA.w, RandVector, Radius, Pos, Normal, SSAOInfo.y, SSAOInfo.z, SSAOInfo.w);
}
)";
    }
}

Engine::priv::SSAO::~SSAO() {
    glDeleteTextures(1, &GL_NOISE_TEXTURE);
}

bool Engine::priv::SSAO::init() {
    if (!GLSL_FRAG_CODE.empty()) {
        return false;
    }
    internal_init_fragment_code();
    internal_init_blur_fragment_code();

    auto load_shaders_ssao = []() {
        m_Vertex_Shader = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        m_Fragment_Shader = Engine::Resources::addResource<Shader>(GLSL_FRAG_CODE, ShaderType::Fragment);
    };
    auto load_shader_program_ssao = []() {
        m_Shader_Program = Engine::Resources::addResource<ShaderProgram>("SSAO", m_Vertex_Shader, m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(load_shaders_ssao, load_shader_program_ssao);


    auto load_shaders_ssao_blur = []() {
        m_Vertex_Shader_Blur = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        m_Fragment_Shader_Blur = Engine::Resources::addResource<Shader>(GLSL_FRAG_CODE_BLUR, ShaderType::Fragment);
    };
    auto load_shader_program_ssao_blur = []() {
        m_Shader_Program_Blur = Engine::Resources::addResource<ShaderProgram>("SSAO_Blur", m_Vertex_Shader_Blur, m_Fragment_Shader_Blur);
    };
    Engine::priv::threading::addJobWithPostCallback(load_shaders_ssao_blur, load_shader_program_ssao_blur);


    std::uniform_real_distribution<float> rand_dist(0.0f, 1.0f);
    std::default_random_engine gen;
    //internal_generate_kernel(rand_dist, gen);
    internal_generate_noise(rand_dist, gen);
    Engine::Renderer::ssao::setLevel(STATIC_SSAO.m_SSAOLevel);

    return true;
}
void Engine::priv::SSAO::passSSAO(GBuffer& gbuffer, const Viewport& viewport, const Camera& camera, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    float divisor       = gbuffer.getSmallFBO().divisor();
    float screen_width  = viewportDimensions.z * divisor;
    float screen_height = viewportDimensions.w * divisor;

    Engine::Renderer::sendUniform2Safe("ScreenSize", screen_width, screen_height);
    Engine::Renderer::sendUniform4("SSAOInfo", m_ssao_radius * (viewportDimensions.z / float(Engine::Resources::getWindowSize().x)), m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Engine::Renderer::sendUniform4("SSAOInfoA", 0, 0, m_ssao_samples, SSAO_NORMALMAP_SIZE); //change to 4f eventually?

    Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0, false };
    Engine::priv::OpenGLBindTextureRAII gRandomMap{ "gRandomMap", GL_NOISE_TEXTURE, GL_TEXTURE_2D, 1, false };
    Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SSAO::passBlur(GBuffer& gbuffer, const Viewport& viewport, std::string_view type, uint32_t texture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program_Blur.get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    float divisor       = gbuffer.getSmallFBO().divisor();
    float screen_width  = viewportDimensions.z * divisor;
    float screen_height = viewportDimensions.w * divisor;
    glm::vec2 hv = (type == "H") ? glm::vec2{ 1.0f, 0.0f } : glm::vec2{ 0.0f, 1.0f };

    Engine::Renderer::sendUniform4("Data", m_ssao_blur_radius, m_ssao_blur_strength, hv.x, hv.y);
    Engine::Renderer::sendUniform2Safe("ScreenSize", screen_width, screen_height);

    Engine::priv::OpenGLBindTextureRAII uSceneToBeSSAOBlurred{ "uSceneToBeSSAOBlurred", gbuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}
void Engine::Renderer::ssao::setLevel(const SSAOLevel level) {
    Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel = level;
    switch (level) {
        case SSAOLevel::Off: {
            break;
        } case SSAOLevel::Low: {
            setSamples(4);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);       
            break;
        } case SSAOLevel::Medium: {
            setSamples(8);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        } case SSAOLevel::High: {
            setSamples(12);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        } case SSAOLevel::Ultra: {
            setSamples(16);
            setIntensity(1.8f);
            setBias(0.048f);
            setRadius(0.175f);
            setScale(1.0f);

            setBlurRadius(0.66f);
            setBlurStrength(0.48f);
            break;
        }
    }
}
void Engine::Renderer::ssao::enableBlur(bool blurEnabled) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = blurEnabled; }
void Engine::Renderer::ssao::disableBlur() noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = false; }
float Engine::Renderer::ssao::getBlurRadius() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius; }
void Engine::Renderer::ssao::setBlurRadius(float blurRadius) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius = std::max(0.0f, blurRadius); }
float Engine::Renderer::ssao::getBlurStrength() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength; }
void Engine::Renderer::ssao::setBlurStrength(float blurStrength) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength = std::max(0.0f, blurStrength); }
int Engine::Renderer::ssao::getBlurNumPasses() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_num_passes; }
void Engine::Renderer::ssao::setBlurNumPasses(int numPasses) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_num_passes = std::max(0, numPasses); }
float Engine::Renderer::ssao::getIntensity() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity; }
void Engine::Renderer::ssao::setIntensity(float intensity) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity = std::max(0.0f, intensity); }
float Engine::Renderer::ssao::getRadius() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius; }
void Engine::Renderer::ssao::setRadius(float radius) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius = std::max(0.0f, radius); }
float Engine::Renderer::ssao::getScale() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale; }
void Engine::Renderer::ssao::setScale(float scale) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale = std::max(0.0f, scale); }
float Engine::Renderer::ssao::getBias() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias; }
void Engine::Renderer::ssao::setBias(float bias) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias = bias; }
int Engine::Renderer::ssao::getSamples() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples; }
void Engine::Renderer::ssao::setSamples(int numSamples) noexcept {
    Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples = glm::clamp(std::max(0, numSamples), 0, SSAO_MAX_KERNEL_SIZE);
}