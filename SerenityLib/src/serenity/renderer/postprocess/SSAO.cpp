
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

    std::array<glm::vec3, SSAO_MAX_KERNEL_SIZE>       SSAO_KERNELS;

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

    constexpr int SSAO_NORMALMAP_SIZE  = 4;

    void internal_generate_kernel(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
        for (uint32_t i = 0; i < SSAO_KERNELS.size(); ++i) {
            glm::vec3 sample{
                rand_dist(gen) * 2.0f - 1.0f, 
                rand_dist(gen) * 2.0f - 1.0f, 
                rand_dist(gen) 
            };
            sample          = glm::normalize(sample);
            sample         *= rand_dist(gen);
            float scale     = float(i) / float(SSAO_KERNELS.size());
            float a         = 0.1f;
            float b         = 1.0f;
            float f         = scale * scale;
            scale           = a + f * (b - a);
            sample         *= scale;
            SSAO_KERNELS[i] = sample;
        }
    }
    void internal_generate_noise(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept {
        auto ssaoNoise = Engine::create_and_reserve<std::vector<glm::vec3>>(SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE);
        for (uint32_t i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i) {
            ssaoNoise.emplace_back(
                rand_dist(gen) * 2.0 - 1.0, 
                rand_dist(gen) * 2.0 - 1.0, 
                0.0f
            );
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

varying vec2 texcoords;
const int SSAO_NORMALMAP_SIZE = )" + std::to_string(SSAO_NORMALMAP_SIZE) + R"(;
const int SSAO_NORMALMAP_SIZE_HALF = SSAO_NORMALMAP_SIZE / 2;

void main() {
    vec2 uvs       = texcoords * (ScreenInfo.zw / ScreenInfo.xy);
    vec2 texelSize = 1.0 / vec2(textureSize(uSceneToBeSSAOBlurred, 0));
    float result = 0.0;
    for (int x = -SSAO_NORMALMAP_SIZE_HALF; x < SSAO_NORMALMAP_SIZE_HALF; ++x) {
        for (int y = -SSAO_NORMALMAP_SIZE_HALF; y < SSAO_NORMALMAP_SIZE_HALF; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture2D(USE_SAMPLER_2D(uSceneToBeSSAOBlurred), uvs + offset).a;
        }
    }
    gl_FragColor.a = result / (SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE);
}
)";


        GLSL_FRAG_CODE_BLUR_COPY_PIXELS = R"(
uniform SAMPLER_TYPE_2D uSceneToBeSSAOBlurred;

varying vec2 texcoords;

void main() {
    vec2 uvs       = texcoords * (ScreenInfo.zw / ScreenInfo.xy);
    gl_FragColor.a = texture2D(USE_SAMPLER_2D(uSceneToBeSSAOBlurred), uvs).a;
}
)";

    }
    void internal_init_fragment_code() {
        GLSL_FRAG_CODE = R"(
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gRandomMap;
uniform SAMPLER_TYPE_2D gDepthMap;

uniform vec3 uSamples[)" + std::to_string(SSAO_MAX_KERNEL_SIZE) + R"(];

uniform vec2  uNoiseScale;
uniform vec4  SSAOInfo;  //   x = radius     y = intensity    z = bias        w = scale
uniform ivec4 SSAOInfoA;//    x = UNUSED     y = UNUSED       z = Samples     w = NoiseTextureSize

varying vec2 texcoords;

void main() {
    vec2 uvs        = texcoords * (ScreenInfo.zw / ScreenInfo.xy);
    vec3 fragPos    = GetViewPosition(USE_SAMPLER_2D(gDepthMap), uvs, CameraNear, CameraFar);
    vec3 normal     = normalize(texture2D(USE_SAMPLER_2D(gNormalMap), uvs).rgb);
    normal          = GetViewNormalsFromWorld(normal, CameraView);
    vec3 randomVec  = normalize(texture2D(USE_SAMPLER_2D(gRandomMap), uvs * uNoiseScale).xyz);
    vec3 tangent    = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent  = cross(normal, tangent);
    mat3 TBN        = mat3(tangent, bitangent, normal);
    float occlusion = 0.0;
    for(int i = 0; i < SSAOInfoA.z; ++i) {
        vec3 samplePos    = TBN * (uSamples[i] * SSAOInfo.w);
        samplePos         = fragPos + samplePos * SSAOInfo.x; 
        
        vec4 offset       = vec4(samplePos, 1.0);
        offset            = CameraProj * offset;
        offset.xyz       /= offset.w;
        offset.xyz        = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = GetViewPosition(USE_SAMPLER_2D(gDepthMap), offset.xy, CameraNear, CameraFar).z;

        float rangeCheck  = smoothstep(0.0, 1.0, SSAOInfo.x / abs(fragPos.z - sampleDepth));
        occlusion        += (sampleDepth >= samplePos.z + SSAOInfo.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion /= SSAOInfoA.z;
    gl_FragColor.a = pow(occlusion, 1.0 / SSAOInfo.y);
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
    internal_generate_kernel(rand_dist, gen);
    internal_generate_noise(rand_dist, gen);
    Engine::Renderer::ssao::setLevel(STATIC_SSAO.m_SSAOLevel);

    return true;
}
void Engine::priv::SSAO::passSSAO(GBuffer& gbuffer, const Viewport& viewport, const Camera& camera, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    const float divisor            = gbuffer.getSmallFBO().divisor();
    const float screen_width       = gbuffer.width() * divisor;
    const float screen_height      = gbuffer.height() * divisor;

    Engine::Renderer::sendUniform3vSafe("uSamples[0]", SSAO_KERNELS.data(), uint32_t(SSAO_KERNELS.size()));
    Engine::Renderer::sendUniform2Safe("uNoiseScale", screen_width / float(SSAO_NORMALMAP_SIZE), screen_height / float(SSAO_NORMALMAP_SIZE));
    Engine::Renderer::sendUniform4Safe("SSAOInfo", m_Radius, m_Intensity, m_Bias, m_Scale);
    Engine::Renderer::sendUniform4Safe("SSAOInfoA", 0, 0, m_NumSamples, SSAO_NORMALMAP_SIZE); //change to 4f eventually?

    Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0, false };
    Engine::priv::OpenGLBindTextureRAII gRandomMap{ "gRandomMap", GL_NOISE_TEXTURE, GL_TEXTURE_2D, 1, false };
    Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SSAO::passBlur(GBuffer& gbuffer, const Viewport& viewport, uint32_t texture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program_Blur.get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();

    Engine::priv::OpenGLBindTextureRAII uSceneToBeSSAOBlurred{ "uSceneToBeSSAOBlurred", gbuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SSAO::passBlurCopyPixels(GBuffer& gbuffer, const Viewport& viewport, uint32_t texture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program_Blur_Copy_Pixels.get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();

    Engine::priv::OpenGLBindTextureRAII uSceneToBeSSAOBlurred{ "uSceneToBeSSAOBlurred", gbuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}

void Engine::Renderer::ssao::setLevel(const SSAOLevel level) {
    Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel = level;
    auto setRegularValues = []() {
        setIntensity(SSAO_DEFAULT_INTENSITY);
        setBias(SSAO_DEFAULT_BIAS);
        setRadius(SSAO_DEFAULT_RADIUS);
        setScale(SSAO_DEFAULT_SCALE);
    };
    switch (level) {
        case SSAOLevel::Off: {
            break;
        } case SSAOLevel::Low: {
            setSamples(8);
            setRegularValues();
            break;
        } case SSAOLevel::Medium: {
            setSamples(16);
            setRegularValues();
            break;
        } case SSAOLevel::High: {
            setSamples(32);
            setRegularValues();
            break;
        } case SSAOLevel::Ultra: {
            setSamples(64);
            setRegularValues();
            break;
        }
    }
}
void Engine::Renderer::ssao::enableBlur(bool blurEnabled) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_DoBlur = blurEnabled; }
void Engine::Renderer::ssao::disableBlur() noexcept { Engine::priv::SSAO::STATIC_SSAO.m_DoBlur = false; }
int Engine::Renderer::ssao::getBlurNumPasses() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_BlurNumPasses; }
void Engine::Renderer::ssao::setBlurNumPasses(int numPasses) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_BlurNumPasses = std::max(0, numPasses); }
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