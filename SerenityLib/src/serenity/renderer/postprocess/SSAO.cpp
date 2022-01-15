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
    std::string  GLSL_FRAG_CODE_BLUR_COPY_PIXELS;
    uint32_t     GL_NOISE_TEXTURE = 0;

    Handle       m_Vertex_Shader;
    Handle       m_Fragment_Shader;
    Handle       m_Shader_Program;

    Handle       m_Vertex_Shader_Blur;
    Handle       m_Fragment_Shader_Blur;
    Handle       m_Shader_Program_Blur;

    Handle       m_Vertex_Shader_Blur_Copy_Pixels;
    Handle       m_Fragment_Shader_Blur_Copy_Pixels;
    Handle       m_Shader_Program_Blur_Copy_Pixels;

    constexpr int SSAO_NORMALMAP_SIZE = 4;

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
uniform sampler2D image;
uniform float strength;

varying vec2 texcoords;

const int NOISE_SIZE = )" + std::to_string(SSAO_NORMALMAP_SIZE) + R"(;
const int NOISE_SIZE_HALF = NOISE_SIZE / 2;

void main() {
    vec2 uvs = ViewportUVCalculation(texcoords);
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    float result = 0.0;
    for (int x = -NOISE_SIZE_HALF; x < NOISE_SIZE_HALF; ++x) {
        for (int y = -NOISE_SIZE_HALF; y < NOISE_SIZE_HALF; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture2D(image, uvs + offset).a;
        }
    }
    gl_FragColor.a = (result / (float(NOISE_SIZE) * float(NOISE_SIZE))) * strength;
}
    )";

        GLSL_FRAG_CODE_BLUR_COPY_PIXELS = R"(
uniform sampler2D image;
varying vec2 texcoords;

void main() {
    vec2 uvs = ViewportUVCalculation(texcoords);
    gl_FragColor.a = texture2D(image, uvs).a;
}
    )";


    }
    void internal_init_fragment_code() {
        
        GLSL_FRAG_CODE = R"(
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gRandomMap;
uniform SAMPLER_TYPE_2D gDepthMap;
uniform vec2  ScreenSize;
uniform vec4  SSAOInfo;  // x = radius           y = intensity  z = bias     w = scale
uniform vec4  SSAOInfoA; // x = rangeCheckScale  y = UNUSED     z = Samples  w = UNUSED
varying vec2 texcoords;

const int NOISE_SIZE = )" + std::to_string(SSAO_NORMALMAP_SIZE) + R"(;

const vec2 SSAOPoisson[16] = vec2[](
    vec2(1.0, 0.0), vec2(-1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, -1.0),
    vec2(-0.707, 0.707), vec2(0.707, -0.707), vec2(-0.707, -0.707), vec2(0.707, 0.707),
    vec2(-0.375, 0.927), vec2(0.375, -0.927), vec2(-0.375, -0.927), vec2(0.375, 0.927),
    vec2(-0.927, 0.375), vec2(0.927, -0.375), vec2(-0.927, -0.375), vec2(0.927, 0.375)
);

float SSAOOcclude(sampler2D inDepthSampler, vec2 offsetUV, vec3 ViewPosOrigin, vec3 normal, float intensity, float bias, float scl) {
    vec3 ViewPosSample    = GetViewPosition(inDepthSampler, offsetUV, CameraNear, CameraFar);
    vec3 PositionOffset   = ViewPosSample - ViewPosOrigin;
    float Length          = length(PositionOffset);
    vec3 VectorNormalized = PositionOffset / Length;
    float Dist            = Length * scl;
    float attenuation     = 1.0 / (1.0 + Dist);
    float angleMath       = max(0.0, dot(normal, VectorNormalized) - bias);
    float rangeCheck      = smoothstep(0.0, 1.0, SSAOInfo.x * SSAOInfoA.x / abs(ViewPosSample.z - ViewPosOrigin.z));

    return angleMath * attenuation * intensity * rangeCheck;
}

float SSAOExecute(sampler2D depthTexture, vec2 uvs, int numSamples, int noiseTextureSize, vec2 randomVector, float radius, vec3 position, vec3 normal, float intensity, float bias, float scl) {
    float res = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        vec2 coord1 = reflect(SSAOPoisson[i].xy * vec2(noiseTextureSize), randomVector) * radius;
        vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
        res += SSAOOcclude(depthTexture, uvs + (coord1 * 0.25), position, normal, intensity, bias, scl);
        res += SSAOOcclude(depthTexture, uvs + (coord2 * 0.50), position, normal, intensity, bias, scl);
        res += SSAOOcclude(depthTexture, uvs + (coord1 * 0.75), position, normal, intensity, bias, scl);
        res += SSAOOcclude(depthTexture, uvs + (coord2 * 1.00), position, normal, intensity, bias, scl);
    }
    res /= (numSamples * 4.0);
    return res;
}

void main() {
    vec2 uvs        = ViewportUVCalculation(texcoords);
    vec3 Pos        = GetViewPosition(USE_SAMPLER_2D(gDepthMap), uvs, CameraNear, CameraFar);
    vec3 Normal     = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uvs).rg);
    Normal          = GetViewNormalsFromWorld(Normal, CameraView);
    vec2 RandVector = normalize(texture2D(USE_SAMPLER_2D(gRandomMap), ScreenSize * uvs / NOISE_SIZE).xy);
    float Radius    = SSAOInfo.x / max(Pos.z, 100.0);
    gl_FragColor.a  = SSAOExecute(USE_SAMPLER_2D(gDepthMap), uvs, int(SSAOInfoA.z), NOISE_SIZE, RandVector, Radius, Pos, Normal, SSAOInfo.y, SSAOInfo.z, SSAOInfo.w);
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



    auto load_shaders_ssao_blur_copy_pixels = []() {
        m_Vertex_Shader_Blur_Copy_Pixels = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        m_Fragment_Shader_Blur_Copy_Pixels = Engine::Resources::addResource<Shader>(GLSL_FRAG_CODE_BLUR_COPY_PIXELS, ShaderType::Fragment);
    };
    auto load_shader_program_ssao_blur_copy_pixels = []() {
        m_Shader_Program_Blur_Copy_Pixels = Engine::Resources::addResource<ShaderProgram>("SSAO_BlurCopyPixels", m_Vertex_Shader_Blur_Copy_Pixels, m_Fragment_Shader_Blur_Copy_Pixels);
    };
    Engine::priv::threading::addJobWithPostCallback(load_shaders_ssao_blur_copy_pixels, load_shader_program_ssao_blur_copy_pixels);


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
    const float divisor            = gbuffer.getSmallFBO().divisor();
    const float screen_width       = viewportDimensions.z * divisor;
    const float screen_height      = viewportDimensions.w * divisor;

    Engine::Renderer::sendUniform2Safe("ScreenSize", screen_width, screen_height);
    Engine::Renderer::sendUniform4("SSAOInfo", m_Radius, m_Intensity, m_Bias, m_Scale);
    Engine::Renderer::sendUniform4("SSAOInfoA", m_RangeCheckScale, 0.0f, float(m_NumSamples), 0.0f);

    Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0, false };
    Engine::priv::OpenGLBindTextureRAII gRandomMap{ "gRandomMap", GL_NOISE_TEXTURE, GL_TEXTURE_2D, 1, false };
    Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SSAO::passBlur(GBuffer& gbuffer, const Viewport& viewport, uint32_t texture, const Engine::priv::RenderModule& renderer) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program_Blur.get<ShaderProgram>());

    Engine::Renderer::sendUniform1("strength", m_BlurStrength);
    Engine::priv::OpenGLBindTextureRAII image{ "image", gbuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SSAO::passBlurCopyPixels(GBuffer& gbuffer, const Viewport& viewport, uint32_t texture, const Engine::priv::RenderModule& renderer) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program_Blur.get<ShaderProgram>());

    Engine::priv::OpenGLBindTextureRAII image{ "image", gbuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}


void Engine::Renderer::ssao::setLevel(const SSAOLevel level) {
    Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel = level;
    auto setDefaultValues = []() {
        /*
        setIntensity(1.8f);
        setBias(0.048f);
        setRadius(0.175f);
        setScale(1.0f);
        setBlurRadius(0.66f);
        setBlurStrength(0.48f);
        */
    };
    switch (level) {
        case SSAOLevel::Off: {
            break;
        } case SSAOLevel::Low: {
            setSamples(4);
            setDefaultValues();
            break;
        } case SSAOLevel::Medium: {
            setSamples(8);
            setDefaultValues();
            break;
        } case SSAOLevel::High: {
            setSamples(12);
            setDefaultValues();
            break;
        } case SSAOLevel::Ultra: {
            setSamples(16);
            setDefaultValues();
            break;
        }
    }
}


void Engine::Renderer::ssao::enableBlur(bool blurEnabled) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_DoBlur = blurEnabled; }
void Engine::Renderer::ssao::disableBlur() noexcept { Engine::priv::SSAO::STATIC_SSAO.m_DoBlur = false; }
float Engine::Renderer::ssao::getBlurStrength() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_BlurStrength; }
void Engine::Renderer::ssao::setBlurStrength(float blurStrength) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_BlurStrength = std::max(0.0f, blurStrength); }
float Engine::Renderer::ssao::getIntensity() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_Intensity; }
void Engine::Renderer::ssao::setIntensity(float intensity) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_Intensity = std::max(0.0f, intensity); }
float Engine::Renderer::ssao::getRadius() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_Radius; }
void Engine::Renderer::ssao::setRadius(float radius) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_Radius = std::max(0.0f, radius); }
float Engine::Renderer::ssao::getScale() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_Scale; }
void Engine::Renderer::ssao::setScale(float scale) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_Scale = std::max(0.0f, scale); }
float Engine::Renderer::ssao::getBias() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_Bias; }
void Engine::Renderer::ssao::setBias(float bias) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_Bias = bias; }
int Engine::Renderer::ssao::getSamples() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_NumSamples; }
void Engine::Renderer::ssao::setSamples(int numSamples) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_NumSamples = glm::clamp(std::max(0, numSamples), 0, SSAO_MAX_KERNEL_SIZE); }
[[nodiscard]] float Engine::Renderer::ssao::getRangeScale() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_RangeCheckScale; }
void Engine::Renderer::ssao::setRangeScale(float rangeScale) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_RangeCheckScale = glm::max(0.0f, rangeScale); }