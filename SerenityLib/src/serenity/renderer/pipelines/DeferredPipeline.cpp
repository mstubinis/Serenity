
#include <serenity/renderer/pipelines/DeferredPipeline.h>
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/system/Engine.h>
#include <serenity/lights/Lights.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/resources/material/MaterialLayer.h>
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>
#include <serenity/renderer/opengl/BindTextureRAII.h>

#include <serenity/ecs/components/ComponentTransform.h>

#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/DepthOfField.h>
#include <serenity/renderer/postprocess/Bloom.h>
#include <serenity/renderer/postprocess/FXAA.h>
#include <serenity/renderer/postprocess/SMAA.h>
#include <serenity/renderer/postprocess/GodRays.h>
#include <serenity/renderer/postprocess/Fog.h>

#include <serenity/resources/Engine_BuiltInShaders.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <SFML/Graphics/Image.hpp>
#include <execution>

#include <serenity/utils/BlockProfiler.h>

using namespace Engine;
using namespace Engine::priv;
using namespace Engine::Renderer;

namespace {
    constexpr auto DEFAULT_MATERIAL_BIND_FUNCTOR = [](const Material& m) {
        const auto& components = m.getComponents();
        size_t numComponents   = components.size();
        int    textureUnit     = 0;
        for (size_t i = 0; i < numComponents; ++i) {
            components[i].bind(i, textureUnit);
        }
        Engine::Renderer::sendUniform1Safe("numComponents", int(numComponents));
        Engine::Renderer::sendUniform1Safe("Shadeless", int(m.getShadeless()));
        Engine::Renderer::sendUniform4Safe("Material_F0AndID", m.getF0().r(), m.getF0().g(), m.getF0().b(), float(m.getID()));
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesOne", m.getGlowFloat(), m.getAOFloat(), m.getMetalnessFloat(), m.getSmoothnessFloat());
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesTwo", m.getAlphaFloat(), m.getDiffuseModelFloat(), m.getSpecularModelFloat(), 0.0f);
    };
    constexpr const std::array<const glm::mat4, 6> CAPTURE_VIEWS = {
        glm::mat4{  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
        glm::mat4{  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
        glm::mat4{  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
        glm::mat4{  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
        glm::mat4{  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
        glm::mat4{ -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f },
    };
#ifndef ENGINE_PRODUCTION
    constexpr const char* getDebugMsgSourceStr(GLenum src) {
        switch (src) {
            case GL_DEBUG_SOURCE_API: {
                return "API";
            } case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
                return "WINDOW_SYSTEM";
            } case GL_DEBUG_SOURCE_SHADER_COMPILER: {
                return "SHADER_COMPILER";
            } case GL_DEBUG_SOURCE_THIRD_PARTY: {
                return "THIRD_PARTY";
            } case GL_DEBUG_SOURCE_APPLICATION: {
                return "APPLICATION";
            } case GL_DEBUG_SOURCE_OTHER: {
                return "OTHER";
            }
        }
        return "";
    }
    constexpr const char* getDebugMsgTypeStr(GLenum type) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR: {
                return "ERROR";
            } case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
                return "DEPRECATED_BEHAVIOR";
            } case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
                return "UNDEFINED_BEHAVIOR";
            } case GL_DEBUG_TYPE_PORTABILITY: {
                return "PORTABILITY";
            } case GL_DEBUG_TYPE_PERFORMANCE: {
                return "PERFORMANCE";
            } case GL_DEBUG_TYPE_MARKER: {
                return "MARKER";
            } case GL_DEBUG_TYPE_PUSH_GROUP: {
                return "PUSH_GROUP";
            } case GL_DEBUG_TYPE_POP_GROUP: {
                return "POP_GROUP";
            } case GL_DEBUG_TYPE_OTHER: {
                return "OTHER";
            }
        }
        return "";
    }
    constexpr const char* getDebugMsgSeverityStr(GLenum sev) {
        switch (sev) {
            case GL_DEBUG_SEVERITY_HIGH: {
                return "HIGH";
            } case GL_DEBUG_SEVERITY_MEDIUM: {
                return "MED";
            } case GL_DEBUG_SEVERITY_LOW: {
                return "LOW";
            } case GL_DEBUG_SEVERITY_NOTIFICATION: {
                return "NOTIFICATION";
            }
        }
        return "";
    }
    void opengl_debug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
#ifdef _DEBUG
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            return;
        }
#endif
        std::cout << "opengl error - source: " << getDebugMsgSourceStr(source) << ", type: " << getDebugMsgTypeStr(type) << ", id: " << id << ", severity: " << getDebugMsgSeverityStr(severity) << ", message: " << message << '\n';
    }
#endif 
    template<class PROJECTION_MATRIX>
    std::vector<glm::mat4> internal_gen_capture_views(PROJECTION_MATRIX&& projection) {
        auto captureViews = Engine::create_and_reserve<std::vector<glm::mat4>>(CAPTURE_VIEWS.size());
        for (const auto& view : CAPTURE_VIEWS) {
            captureViews.emplace_back(std::forward<PROJECTION_MATRIX>(projection) * view);
        }
        return captureViews;
    }

    void resize_shadow_containers(const auto& light, auto& hashMap, auto& mainContainer) {
        const auto sceneID = light.sceneID();
        if (hashMap.size() <= sceneID) {
            hashMap.resize(sceneID + 1);
        }
        if (mainContainer.size() <= sceneID) {
            mainContainer.resize(sceneID + 1);
        }
    }
}

struct ShaderEnum final { enum Shader : uint32_t {
    DecalVertex,
    DecalFrag,
    FullscreenVertex,
    BulletPhysicsVertex,
    BulletPhysicsFrag,
    VertexBasic,
    Vertex2DAPI,
    VertexSkybox,
    LightingVertex,
    ForwardFrag,
    DeferredFrag,
    ZPrepassFrag,
    DeferredFrag2DAPI,
    DeferredFragSkybox,
    CopyDepthFrag,
    BlurFrag,
    FinalFrag,
    DepthAndTransparencyFrag,
    LightingFrag,
    LightingGIFrag,
    CubemapConvoludeFrag,
    CubemapPrefilterEnvFrag,
    BRDFPrecomputeFrag,
    //GrayscaleFrag,
    StencilPassFrag,
    ParticleVertex,
    ParticleFrag,
    NormalessDiffuseFrag,
    LightingFragBasic,
    LightingGIFragBasic,
    ShadowDepthVertex,
    ShadowDepthFrag,
    _TOTAL,
};};
struct ShaderProgramEnum final { enum Program : uint32_t {
    BulletPhysics,
    ZPrepass,
    Deferred2DAPI,
    DeferredBlur,
    DeferredFinal,
    DepthAndTransparency,
    DeferredSkybox,
    CopyDepth,
    DeferredLighting,
    DeferredLightingGI,
    CubemapConvolude,
    CubemapPrefilterEnv,
    BRDFPrecomputeCookTorrance,
    //Grayscale,
    StencilPass,
    Particle,
    NormalessDiffuse,
    DeferredLightingBasic,
    DeferredLightingGIBasic,
    ShadowDepth,
    _TOTAL,
};};

DeferredPipeline::DeferredPipeline(Engine::priv::RenderModule& renderer) 
    : m_Renderer{ renderer }
{
}
DeferredPipeline::~DeferredPipeline() {
    //TODO: add cleanup() from ssao / smaa here?
}
void DeferredPipeline::internal_gl_scissor_reset() noexcept {
    const auto winSize    = Engine::Resources::getWindowSize();
    m_CurrentScissorState = glm::vec4{ -1.0f };
    m_CurrentScissorDepth = std::numeric_limits<float>().min();
    glScissor(0, 0, winSize.x, winSize.y);
}
void DeferredPipeline::internal_gl_scissor(const glm::vec4& scissor, float depth) noexcept {
    if (scissor == glm::vec4{ -1.0f }) {
        internal_gl_scissor_reset();
    } else {
        m_CurrentScissorState = scissor;
        m_CurrentScissorDepth = depth;
        glScissor(GLint(scissor.x), GLint(scissor.y), GLsizei(scissor.z), GLsizei(scissor.w));
    }
}
void DeferredPipeline::init() {
    const auto window_size = Engine::Resources::getWindowSize();
    //const auto majorVersion = Engine::priv::OpenGLState::constants.MAJOR_VERSION;
    //const auto minorVersion = Engine::priv::OpenGLState::constants.MINOR_VERSION;
    m_2DProjectionMatrix   = glm::ortho(0.0f, float(window_size.x), 0.0f, float(window_size.y), 0.003f, 6000.0f);

    const float init_border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_InternalShaders.resize(ShaderEnum::_TOTAL);
    m_InternalShaderPrograms.resize(ShaderProgramEnum::_TOTAL);

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, init_border_color);

    m_OpenGLExtensionsManager.INIT();

    m_OpenGLStateMachine.GL_glEnable(GL_POLYGON_OFFSET_FILL);
    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
    m_OpenGLStateMachine.GL_glDisable(GL_STENCIL_TEST);
    m_OpenGLStateMachine.GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for non Power of Two textures
    if (Engine::priv::OpenGLState::constants.supportsCubemapSeamless()) {
        m_OpenGLStateMachine.GL_glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //can run real slowly on some gpu's / drivers. its core 32 but just to be safe, set it to 40
    }


    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_CLAMP);
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    Handle uboCameraHandle = Engine::Resources::addResource<UniformBufferObject>("Camera", static_cast<uint32_t>(sizeof(UBOCameraDataStruct)));
    m_UBOCamera = uboCameraHandle.get<UniformBufferObject>();

    priv::EShaders::init();

    m_FullscreenQuad.init();
    m_FullscreenTriangle.init();

    FXAA::init();
    SSAO::init();
    HDR::init();
    DepthOfField::init();
    Bloom::STATIC_BLOOM.init();
    GodRays::init();
    SMAA::init();

    auto emplaceShader = [](uint32_t index, const std::string& str, std::vector<Handle>& collection, ShaderType type) {
        collection[index] = Engine::Resources::addResource<Shader>(str, type);
    };
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(0, EShaders::decal_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(1, EShaders::decal_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(2, EShaders::fullscreen_quad_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(3, EShaders::bullet_physics_vert, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(4, EShaders::bullet_physcis_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(5, EShaders::vertex_basic, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(6, EShaders::vertex_2DAPI, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(7, EShaders::vertex_skybox, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(8, EShaders::lighting_vert, m_InternalShaders, ShaderType::Vertex); });

    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(9, EShaders::forward_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(10, EShaders::deferred_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(11, EShaders::zprepass_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(12, EShaders::deferred_frag_hud, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(13, EShaders::deferred_frag_skybox, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(14, EShaders::copy_depth_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(15, EShaders::blur_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(16, EShaders::final_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(17, EShaders::depth_and_transparency_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(18, EShaders::lighting_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(19, EShaders::lighting_frag_gi, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(20, EShaders::cubemap_convolude_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(21, EShaders::cubemap_prefilter_envmap_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(22, EShaders::brdf_precompute, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(23, EShaders::stencil_passover, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(24, EShaders::particle_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(25, EShaders::particle_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(26, EShaders::normaless_diffuse_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(27, EShaders::lighting_frag_basic, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(28, EShaders::lighting_frag_gi_basic, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(29, EShaders::shadow_depth_vert, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([this, &emplaceShader]() {emplaceShader(30, EShaders::shadow_depth_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::waitForAll();

    ShaderProgram::Deferred = Engine::Resources::addResource<ShaderProgram>("Deferred", m_InternalShaders[ShaderEnum::VertexBasic], m_InternalShaders[ShaderEnum::DeferredFrag]);
    ShaderProgram::Forward  = Engine::Resources::addResource<ShaderProgram>("Forward", m_InternalShaders[ShaderEnum::VertexBasic], m_InternalShaders[ShaderEnum::ForwardFrag]);
    ShaderProgram::Decal    = Engine::Resources::addResource<ShaderProgram>("Decal", m_InternalShaders[ShaderEnum::DecalVertex], m_InternalShaders[ShaderEnum::DecalFrag]);

    m_InternalShaderPrograms[ShaderProgramEnum::BulletPhysics] = Engine::Resources::addResource<ShaderProgram>("Bullet_Physics", m_InternalShaders[ShaderEnum::BulletPhysicsVertex], m_InternalShaders[ShaderEnum::BulletPhysicsFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::ZPrepass] = Engine::Resources::addResource<ShaderProgram>("ZPrepass", m_InternalShaders[ShaderEnum::VertexBasic], m_InternalShaders[ShaderEnum::ZPrepassFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::Deferred2DAPI] = Engine::Resources::addResource<ShaderProgram>("Deferred_2DAPI", m_InternalShaders[ShaderEnum::Vertex2DAPI], m_InternalShaders[ShaderEnum::DeferredFrag2DAPI]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredBlur] = Engine::Resources::addResource<ShaderProgram>("Deferred_Blur", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::BlurFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredFinal] = Engine::Resources::addResource<ShaderProgram>("Deferred_Final", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::FinalFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DepthAndTransparency] = Engine::Resources::addResource<ShaderProgram>("DepthAndTransparency", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::DepthAndTransparencyFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox] = Engine::Resources::addResource<ShaderProgram>("Deferred_Skybox", m_InternalShaders[ShaderEnum::VertexSkybox], m_InternalShaders[ShaderEnum::DeferredFragSkybox]);
    m_InternalShaderPrograms[ShaderProgramEnum::CopyDepth] = Engine::Resources::addResource<ShaderProgram>("Copy_Depth", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::CopyDepthFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLighting] = Engine::Resources::addResource<ShaderProgram>("Deferred_Light", m_InternalShaders[ShaderEnum::LightingVertex], m_InternalShaders[ShaderEnum::LightingFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGI] = Engine::Resources::addResource<ShaderProgram>("Deferred_Light_GI", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::LightingGIFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::CubemapConvolude] = Engine::Resources::addResource<ShaderProgram>("Cubemap_Convolude", m_InternalShaders[ShaderEnum::VertexSkybox], m_InternalShaders[ShaderEnum::CubemapConvoludeFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::CubemapPrefilterEnv] = Engine::Resources::addResource<ShaderProgram>("Cubemap_Prefilter_Env", m_InternalShaders[ShaderEnum::VertexSkybox], m_InternalShaders[ShaderEnum::CubemapPrefilterEnvFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance] = Engine::Resources::addResource<ShaderProgram>("BRDF_Precompute_CookTorrance", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::BRDFPrecomputeFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::StencilPass] = Engine::Resources::addResource<ShaderProgram>("Stencil_Pass", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::StencilPassFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::Particle] = Engine::Resources::addResource<ShaderProgram>("Particle", m_InternalShaders[ShaderEnum::ParticleVertex], m_InternalShaders[ShaderEnum::ParticleFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::NormalessDiffuse] = Engine::Resources::addResource<ShaderProgram>("NormalessDiffuse", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::NormalessDiffuseFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingBasic] = Engine::Resources::addResource<ShaderProgram>("Deferred_Light_Basic", m_InternalShaders[ShaderEnum::LightingVertex], m_InternalShaders[ShaderEnum::LightingFragBasic]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGIBasic] = Engine::Resources::addResource<ShaderProgram>("Deferred_Light_GI_Basic", m_InternalShaders[ShaderEnum::FullscreenVertex], m_InternalShaders[ShaderEnum::LightingGIFragBasic]);
    m_InternalShaderPrograms[ShaderProgramEnum::ShadowDepth] = Engine::Resources::addResource<ShaderProgram>("Shadow_Depth", m_InternalShaders[ShaderEnum::ShadowDepthVertex], m_InternalShaders[ShaderEnum::ShadowDepthFrag]);


    std::vector<uint8_t> ImageWhite(2 * 2 * 4, 255);
    std::vector<uint8_t> ImageBlack(2 * 2 * 4, 0);
    std::vector<uint8_t> ImageCheckers(8 * 8 * 4, 255);

    auto getIdx = [](uint32_t width, uint32_t x, uint32_t y) {
        return ((x * width) + y) * 4;
    };

    uint32_t count = 0;
    const uint32_t checkersWidth  = 8;
    const uint32_t checkersHeight = 8;
    for (uint32_t i = 0; i < checkersHeight; ++i) {
        for (uint32_t j = 0; j < checkersWidth; ++j) {
            if ((count % 2 == 0 && i % 2 == 0) || (count % 2 != 0 && i % 2 == 1)) {
                //red color
                ImageCheckers[getIdx(checkersWidth, i, j) + 0] = 255;
                ImageCheckers[getIdx(checkersWidth, i, j) + 1] = 0;
                ImageCheckers[getIdx(checkersWidth, i, j) + 2] = 0;
                ImageCheckers[getIdx(checkersWidth, i, j) + 3] = 255;
            }
            ++count;
        }
    }
    //"D" = default
    Texture::White     = Engine::Resources::addResource<Texture>(ImageWhite.data(), 2, 2, "DWhiteTexture", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);
    Texture::Black     = Engine::Resources::addResource<Texture>(ImageBlack.data(), 2, 2, "DBlackTexture", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);

    Texture::Checkers  = Engine::Resources::addResource<Texture>(ImageCheckers.data(), 8, 8, "DCheckersTexture", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);
    Texture::Checkers.get<Texture>()->setFilter(TextureFilter::Nearest);
    Material::Checkers = Engine::Resources::addResource<Material>("DCheckers", Texture::Checkers);
    Material::Checkers.get<Material>()->setSpecularModel(SpecularModel::None);
    Material::Checkers.get<Material>()->setSmoothness(0_uc);
    Material::Checkers.get<Material>()->setMetalness(0_uc);

    Material::WhiteShadeless = Engine::Resources::addResource<Material>("DWhiteShadeless", Texture::White);
    Material::WhiteShadeless.get<Material>()->setSpecularModel(SpecularModel::None);
    Material::WhiteShadeless.get<Material>()->setSmoothness(0_uc);
    Material::WhiteShadeless.get<Material>()->setMetalness(0_uc);
    Material::WhiteShadeless.get<Material>()->setShadeless(true);

    Texture::BRDF = Engine::Resources::addResource<Texture>(512, 512, ImagePixelType::FLOAT, ImagePixelFormat::RG, ImageInternalFormat::RG16F, 1.0f);
    Texture::BRDF.get<Texture>()->setWrapping(TextureWrap::ClampToEdge);

    internal_generate_brdf_lut(m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance], 512, 256);

    //particle instancing
    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().bind();
    glGenBuffers(1, &m_Particle_Instance_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_Particle_Instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);

    auto sizeofOne = sizeof(Engine::priv::ParticleFloatType) * 4;
    auto sizeofTwo = sizeof(Engine::priv::ParticleFloatType) * 2;


#if defined(ENGINE_PARTICLES_HALF_SIZE)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_HALF_FLOAT,      GL_FALSE, sizeof(Engine::priv::ParticleDOD),  (void*)0  );
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_HALF_FLOAT,      GL_FALSE, sizeof(Engine::priv::ParticleDOD),  (void*)sizeofOne);
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 2, GL_UNSIGNED_SHORT, sizeof(Engine::priv::ParticleDOD), (void*)(sizeofOne + sizeofTwo));
#else
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)sizeofOne);
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 2, GL_UNSIGNED_INT, sizeof(Engine::priv::ParticleDOD), (void*)(sizeofOne + sizeofTwo));
#endif
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().unbind();
}

void DeferredPipeline::internal_generate_brdf_lut(Handle program, uint32_t brdfSize, int numSamples) {
    FramebufferObject fbo{ brdfSize, brdfSize }; //try without a depth format
    fbo.bind();

    auto brdfTexture        = Texture::BRDF.get<Texture>();
    TextureType textureType = TextureType::Texture2D;
    Engine::Renderer::bindTextureForModification(textureType, brdfTexture->address());
    glTexImage2D(textureType.toGLType(), 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
    Texture::setFilter(textureType, TextureFilter::Linear);
    TextureBaseClass::setWrapping(textureType, TextureWrap::ClampToEdge);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureType.toGLType(), brdfTexture->address(), 0);
    fbo.checkStatus();

    m_Renderer.bind(program.get<ShaderProgram>());

    Engine::Renderer::sendUniform1("NUM_SAMPLES", numSamples);
    Engine::Renderer::Settings::clear(true, true, false);
    Engine::Renderer::colorMask(true, true, false, false);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::onPipelineChanged() {

}
uint32_t DeferredPipeline::getUniformLocation(const char* location) {
    const auto& uniforms = m_RendererState.current_bound_shader_program->uniforms();
    return (!uniforms.contains(location)) ? -1 : uniforms.at(location);
}
uint32_t DeferredPipeline::getUniformLocationUnsafe(const char* location) {
    return m_RendererState.current_bound_shader_program->uniforms().at(location);
}
uint32_t DeferredPipeline::getMaxNumTextureUnits() {
    return Engine::priv::OpenGLState::constants.MAX_TEXTURE_IMAGE_UNITS;
}
void DeferredPipeline::restoreDefaultState() {
    auto winWidth = Resources::getWindowSize();
    m_OpenGLStateMachine.GL_RESTORE_DEFAULT_STATE_MACHINE(winWidth.x, winWidth.y);
}
void DeferredPipeline::restoreCurrentState() {
    m_OpenGLStateMachine.GL_RESTORE_CURRENT_STATE_MACHINE();
}
void DeferredPipeline::clear2DAPI() {
    m_Background2DAPICommands.clear();
    m_2DAPICommands.clear();
}
void DeferredPipeline::sort2DAPI() {
    auto lambda_sorter = [&](const API2DCommand& lhs, const API2DCommand& rhs) {
        return lhs.depth > rhs.depth;
    };
    Engine::sort(std::execution::par_unseq, m_Background2DAPICommands, lambda_sorter);
    Engine::sort(std::execution::par_unseq, m_2DAPICommands,           lambda_sorter);
}
ShaderProgram* DeferredPipeline::getCurrentBoundShaderProgram() {
    return m_RendererState.current_bound_shader_program;
}
Material* DeferredPipeline::getCurrentBoundMaterial() {
    return m_RendererState.current_bound_material;
}
Mesh* DeferredPipeline::getCurrentBoundMesh() {
    return m_RendererState.current_bound_mesh;
}
uint32_t DeferredPipeline::getCurrentBoundTextureOfType(uint32_t textureType) {
    return m_OpenGLStateMachine.getCurrentlyBoundTextureOfType(textureType);
}
bool DeferredPipeline::stencilOperation(uint32_t stencilFail, uint32_t depthFail, uint32_t depthPass) {
    return m_OpenGLStateMachine.GL_glStencilOp(stencilFail, depthFail, depthPass);
}
bool DeferredPipeline::stencilMask(uint32_t mask) {
    return m_OpenGLStateMachine.GL_glStencilMask(mask);
}
bool DeferredPipeline::stencilFunction(uint32_t stencilFunction, uint32_t reference, uint32_t mask) {
    return m_OpenGLStateMachine.GL_glStencilFunc(stencilFunction, reference, mask);
}
bool DeferredPipeline::setDepthFunction(uint32_t depthFunction) {
    return m_OpenGLStateMachine.GL_glDepthFunc(depthFunction);
}
bool DeferredPipeline::setViewport(float x, float y, float width, float height) {
    return m_OpenGLStateMachine.GL_glViewport(GLint(x), GLint(y), GLsizei(width), GLsizei(height));
}
void DeferredPipeline::clear(bool color, bool depth, bool stencil) {
    if (!color && !depth && !stencil) {
        return;
    }
    GLuint clearBit = (color * GL_COLOR_BUFFER_BIT) | (depth * GL_DEPTH_BUFFER_BIT) | (stencil * GL_STENCIL_BUFFER_BIT);
    glClear(clearBit);
}
bool DeferredPipeline::colorMask(bool r, bool g, bool b, bool alpha) {
    return m_OpenGLStateMachine.GL_glColorMask(r, g, b, alpha);
}
bool DeferredPipeline::clearColor(bool r, bool g, bool b, bool alpha) {
    return m_OpenGLStateMachine.GL_glClearColor(r, g, b, alpha);
}
bool DeferredPipeline::bindTextureForModification(TextureType textureType, uint32_t textureObject) {
    return m_OpenGLStateMachine.GL_glBindTextureForModification(textureType.toGLType(), textureObject);
}
bool DeferredPipeline::bindVAO(uint32_t vaoObject) {
    return m_OpenGLStateMachine.GL_glBindVertexArray(vaoObject);
}
bool DeferredPipeline::deleteVAO(uint32_t& vaoObject) {
    if (vaoObject) {
        glDeleteVertexArrays(1, &vaoObject);
        vaoObject = 0;
        return true;
    }
    return false;
}
void DeferredPipeline::generateAndBindTexture(TextureType textureType, uint32_t& textureObject) {
    glGenTextures(1, &textureObject);
    m_OpenGLStateMachine.GL_glBindTextureForModification(textureType.toGLType(), textureObject);
}
void DeferredPipeline::generateAndBindVAO(uint32_t& vaoObject) {
    glGenVertexArrays(1, &vaoObject);
    DeferredPipeline::bindVAO(vaoObject);
}
bool DeferredPipeline::enableAPI(uint32_t apiEnum) {
    return m_OpenGLStateMachine.GL_glEnable(apiEnum);
}
bool DeferredPipeline::disableAPI(uint32_t apiEnum) {
    return m_OpenGLStateMachine.GL_glDisable(apiEnum);
}
bool DeferredPipeline::enableAPI_i(uint32_t apiEnum, uint32_t index) {
    return m_OpenGLStateMachine.GL_glEnablei(apiEnum, index);
}
bool DeferredPipeline::disableAPI_i(uint32_t apiEnum, uint32_t index) {
    return m_OpenGLStateMachine.GL_glDisablei(apiEnum, index);
}
void DeferredPipeline::clearTexture(int unit, uint32_t textureTarget) {
    m_OpenGLStateMachine.GL_glUnbindTexture(unit, textureTarget);
}
void DeferredPipeline::sendTexture(const char* location, Texture& texture, int unit) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(texture.getTextureType().toGLType(), texture.address());
    Engine::Renderer::sendUniform1(location, unit);
}
void DeferredPipeline::sendTexture(const char* location, TextureCubemap& cubemap, int unit) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(GL_TEXTURE_CUBE_MAP, cubemap.address());
    Engine::Renderer::sendUniform1(location, unit);
}
void DeferredPipeline::sendTexture(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    Engine::Renderer::sendUniform1(location, unit);
}
void DeferredPipeline::sendTextureSafe(const char* location, Texture& texture, int unit) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(texture.getTextureType().toGLType(), texture.address());
    Engine::Renderer::sendUniform1Safe(location, unit);
}
void DeferredPipeline::sendTextureSafe(const char* location, TextureCubemap& cubemap, int unit) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(GL_TEXTURE_CUBE_MAP, cubemap.address());
    Engine::Renderer::sendUniform1Safe(location, unit);
}
void DeferredPipeline::sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    Engine::Renderer::sendUniform1Safe(location, unit);
}
void DeferredPipeline::sendTextures(const char* location, const Texture** textures, int startingSlot, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(textures[i]->getTextureType().toGLType(), textures[i]->address());
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1v(location, m_TextureSlotsBuffer.data(), arrSize);
}
void DeferredPipeline::sendTextures(const char* location, const TextureCubemap** cubemaps, int startingSlot, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(GL_TEXTURE_CUBE_MAP, cubemaps[i]->address());
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1v(location, m_TextureSlotsBuffer.data(), arrSize);
}
void DeferredPipeline::sendTextures(const char* location, const GLuint* addresses, int startingSlot, GLuint glTextureType, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(glTextureType, addresses[i]);
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1v(location, m_TextureSlotsBuffer.data(), arrSize);
}
void DeferredPipeline::sendTexturesSafe(const char* location, const Texture** textures, int startingSlot, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(textures[i]->getTextureType().toGLType(), textures[i]->address());
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1vSafe(location, m_TextureSlotsBuffer.data(), arrSize);
}
void DeferredPipeline::sendTexturesSafe(const char* location, const TextureCubemap** cubemaps, int startingSlot, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(GL_TEXTURE_CUBE_MAP, cubemaps[i]->address());
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1vSafe(location, m_TextureSlotsBuffer.data(), arrSize);
}
void DeferredPipeline::sendTexturesSafe(const char* location, const GLuint* data, int startingSlot, GLuint glTextureType, const int arrSize) {
    m_TextureSlotsBuffer.resize(arrSize);
    for (int i = 0; i < arrSize; ++i) {
        m_OpenGLStateMachine.GL_glActiveTexture(startingSlot + i);
        m_OpenGLStateMachine.GL_glBindTextureForRendering(glTextureType, data[i]);
        m_TextureSlotsBuffer[i] = startingSlot + i;
    }
    Engine::Renderer::sendUniform1vSafe(location, m_TextureSlotsBuffer.data(), arrSize);
}
bool DeferredPipeline::cullFace(uint32_t face) {
    return m_OpenGLStateMachine.GL_glCullFace(face);
}
bool DeferredPipeline::bindReadFBO(uint32_t fbo) {
    return m_OpenGLStateMachine.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
}
bool DeferredPipeline::bindDrawFBO(uint32_t fbo) {
    return m_OpenGLStateMachine.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}
bool DeferredPipeline::bindRBO(uint32_t rbo) {
    return m_OpenGLStateMachine.GL_glBindRenderbuffer(rbo);
}
bool DeferredPipeline::bind(ModelInstance* modelInstance) {
    return true;
}
bool DeferredPipeline::unbind(ModelInstance* modelInstance) {
    return true;
}
bool DeferredPipeline::bind(ShaderProgram* program) {
    if (m_RendererState.current_bound_shader_program != program) {
        m_OpenGLStateMachine.GL_glUseProgram(program->program());
        m_RendererState.current_bound_shader_program = program;
        return true;
    }
    return false;
}
bool DeferredPipeline::unbind(ShaderProgram* program) {
    m_RendererState.current_bound_shader_program = nullptr;
    m_OpenGLStateMachine.GL_glUseProgram(0);
    return true;
}
bool DeferredPipeline::bind(Material* material) {
    bool res = false;
    if (m_RendererState.current_bound_material != material) {
        m_RendererState.current_bound_material = material;
        res = true;
    }
    DEFAULT_MATERIAL_BIND_FUNCTOR(*m_RendererState.current_bound_material);
    return res;
}
bool DeferredPipeline::unbind(Material* material) {
    if (!m_RendererState.current_bound_material) {
        return false;
    }
    m_RendererState.current_bound_material = nullptr;
    return true;
}
bool DeferredPipeline::bind(Mesh* mesh) {
    if (m_RendererState.current_bound_mesh != mesh) {
        m_RendererState.current_bound_mesh = mesh;
        return true;
    }
    return false;
}
bool DeferredPipeline::unbind(Mesh* mesh) {
    if (!m_RendererState.current_bound_mesh) {
        return false;
    }
    m_RendererState.current_bound_mesh = nullptr;
    return true;
}
void DeferredPipeline::generatePBRData(TextureCubemap& cubemap, Handle convolutionTextureHandle , Handle preEnvTextureHandle, uint32_t convoludeSize, uint32_t prefilterSize) { 
    uint32_t size = convoludeSize;
    //Engine::Renderer::unbindFBO();
    Engine::priv::FramebufferObject fbo{ size, size };
    fbo.bind();

    const std::vector<glm::mat4> captureViews = internal_gen_capture_views(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f));
    {
        auto& convolutionTexture = *convolutionTextureHandle.get<TextureCubemap>();
        Engine::Renderer::bindTextureForModification(TextureType::CubeMap, convolutionTexture.address());
        m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::CubemapConvolude].get<ShaderProgram>());

        Engine::priv::OpenGLBindTextureRAII cube{ "cubemap", cubemap, 0, false };

        Engine::Renderer::setViewport(0.0f, 0.0f, size, size);
        for (uint32_t i = 0; i < captureViews.size(); ++i) {
            Engine::Renderer::sendUniformMatrix4("VP", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, convolutionTexture.address(), 0);
            fbo.checkStatus();
            Engine::Renderer::Settings::clear(true, true, false);
            Skybox::bindMesh();
        }
    }
    //now gen EnvPrefilterMap for specular IBL
    {
        size = prefilterSize;
        auto& preEnvTexture = *preEnvTextureHandle.get<TextureCubemap>();
        Engine::Renderer::bindTextureForModification(TextureType::CubeMap, preEnvTexture.address());
        m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::CubemapPrefilterEnv].get<ShaderProgram>());

        Engine::priv::OpenGLBindTextureRAII cube{ "cubemap", cubemap, 0, false };

        Engine::Renderer::setViewport(0.0f, 0.0f, size, size);
        Engine::Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix", 12.56637f / float((cubemap.width() * cubemap.width()) * 6));
        Engine::Renderer::sendUniform1("NUM_SAMPLES", 32);
        const uint32_t maxMipLevels = 5;
        for (uint32_t mipmapLevel = 0; mipmapLevel < maxMipLevels; ++mipmapLevel) {
            const uint32_t mipSize = size * uint32_t(glm::pow(0.5, mipmapLevel)); // reisze framebuffer according to mip-level size.
            const float roughness = float(mipmapLevel) / float(maxMipLevels - 1);
            const float a = roughness * roughness;
            fbo.resize(mipSize, mipSize);
            Engine::Renderer::sendUniform2("Data", roughness, a * a);
            for (uint32_t i = 0; i < captureViews.size(); ++i) {
                Engine::Renderer::sendUniformMatrix4("VP", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preEnvTexture.address(), mipmapLevel);
                fbo.checkStatus();
                Engine::Renderer::Settings::clear(true, true, false);
                Skybox::bindMesh();
            }
        }
    }
}
void DeferredPipeline::onFullscreen() {
    //TODO: move these lines to a more generic area, all rendering pipelines will pretty much do this
    restoreCurrentOpenGLState();

    Engine::Renderer::GLEnable(GL_CULL_FACE);
    Engine::Renderer::GLEnable(GL_DEPTH_CLAMP);

    const auto winSize = Engine::Resources::getWindowSize();
    m_GBuffer.init(winSize.x, winSize.y);
}
void DeferredPipeline::onResize(uint32_t newWidth, uint32_t newHeight) {
    float floatWidth     = float(newWidth);
    float floatHeight    = float(newHeight);
    m_2DProjectionMatrix = glm::ortho(0.0f, floatWidth, 0.0f, floatHeight, 0.003f, 6000.0f);

    m_FullscreenQuad.changeDimensions(floatWidth, floatHeight);
    m_FullscreenTriangle.changeDimensions(floatWidth, floatHeight);

    m_GBuffer.resize(newWidth, newHeight);
}
void DeferredPipeline::onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight) {
    //TODO: move to a more generic area
    m_OpenGLStateMachine.GL_INIT_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);

    if (m_OpenGLStateMachine.constants.MAJOR_VERSION >= 4 && m_OpenGLStateMachine.constants.MINOR_VERSION >= 3) {
        #ifndef ENGINE_PRODUCTION
        //debug logging
        glDebugMessageCallback(opengl_debug, nullptr);
        #endif
    }

    Engine::Renderer::GLEnable(GL_CULL_FACE);
    m_GBuffer.init(windowWidth, windowHeight);
}
void DeferredPipeline::renderSkybox(Skybox* skybox, Handle shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) {
    glm::mat4 view_no_position = camera.getView();
    Math::removeMatrixPosition(view_no_position);
    m_Renderer.bind(shaderProgram.get<ShaderProgram>());

    Engine::priv::OpenGLBindTextureRAII Texture{ "Texture", skybox ? skybox->cubemap().get<TextureCubemap>()->address() : 0, GL_TEXTURE_CUBE_MAP, 0, true };
    Engine::Renderer::sendUniform1("IsFake", skybox ? 0 : 1);
    if (!skybox) {
        const auto& bgColor = scene.getBackgroundColor();
        Engine::Renderer::sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    }
    Engine::Renderer::sendUniformMatrix4("VP", camera.getProjection() * view_no_position);
    Skybox::bindMesh();
}
void DeferredPipeline::toggleShadowCaster(SunLight& sunLight, bool isCaster) {
    const auto sceneID = sunLight.sceneID();
    resize_shadow_containers(sunLight, m_ShadowCasters.m_ShadowCastersSunHashed, m_ShadowCasters.m_ShadowCastersSun);
    if (m_ShadowCasters.m_ShadowCastersSunHashed[sceneID].contains(&sunLight)) {
        m_ShadowCasters.m_ShadowCastersSunHashed[sceneID].at(&sunLight)->m_Enabled = isCaster;
    }
}
void DeferredPipeline::toggleShadowCaster(PointLight& pointLight, bool isCaster) {

}
void DeferredPipeline::toggleShadowCaster(DirectionalLight& directionalLight, bool isCaster) {
    const auto sceneID = directionalLight.sceneID();
    resize_shadow_containers(directionalLight, m_ShadowCasters.m_ShadowCastersSunHashed, m_ShadowCasters.m_ShadowCastersSun);
    if (m_ShadowCasters.m_ShadowCastersDirectionalHashed[sceneID].contains(&directionalLight)) {
        m_ShadowCasters.m_ShadowCastersDirectionalHashed[sceneID].at(&directionalLight)->m_Enabled = isCaster;
    }
}
void DeferredPipeline::toggleShadowCaster(SpotLight& spotLight, bool isCaster) {

}
void DeferredPipeline::toggleShadowCaster(RodLight& rodLight, bool isCaster) {

}
void DeferredPipeline::toggleShadowCaster(ProjectionLight& projectionLight, bool isCaster) {

}
bool DeferredPipeline::buildShadowCaster(SunLight& sunLight, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    const auto sceneID = sunLight.sceneID();
    resize_shadow_containers(sunLight, m_ShadowCasters.m_ShadowCastersSunHashed, m_ShadowCasters.m_ShadowCastersSun);

    if (!m_ShadowCasters.m_ShadowCastersSunHashed[sceneID].contains(&sunLight)) {
        auto& data = m_ShadowCasters.m_ShadowCastersSun[sceneID].emplace_back(
            &sunLight,
            GLDeferredSunLightShadowInfo{ 
                shadowMapWidth, 
                shadowMapHeight,
                frustumType, 
                nearFactor, 
                farFactor 
            }
        );
        m_ShadowCasters.m_ShadowCastersSunHashed[sceneID].emplace(&sunLight, &std::get<1>(data));
        return true;
    } else {
        auto& data = m_ShadowCasters.m_ShadowCastersSunHashed[sceneID].at(&sunLight);
        data->setShadowInfo(shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
    }
    return false;
}
bool DeferredPipeline::buildShadowCaster(PointLight& pointLight) {
    return false;
}
bool DeferredPipeline::buildShadowCaster(DirectionalLight& directionalLight, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    const auto sceneID = directionalLight.sceneID();
    resize_shadow_containers(directionalLight, m_ShadowCasters.m_ShadowCastersDirectionalHashed, m_ShadowCasters.m_ShadowCastersDirectional);

    if (!m_ShadowCasters.m_ShadowCastersDirectionalHashed[sceneID].contains(&directionalLight)) {
        auto& data = m_ShadowCasters.m_ShadowCastersDirectional[sceneID].emplace_back(
            &directionalLight, 
            GLDeferredDirectionalLightShadowInfo{
                shadowMapWidth,
                shadowMapHeight,
                frustumType, 
                nearFactor, 
                farFactor 
            }
        );
        m_ShadowCasters.m_ShadowCastersDirectionalHashed[sceneID].emplace(&directionalLight, &std::get<1>(data));
        return true;
    } else {
        auto& data = m_ShadowCasters.m_ShadowCastersDirectionalHashed[sceneID].at(&directionalLight);
        data->setShadowInfo(shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
    }
    return false;
}
bool DeferredPipeline::buildShadowCaster(SpotLight& spotLight) {
    return false;
}
bool DeferredPipeline::buildShadowCaster(RodLight& rodLight) {
    return false;
}
bool DeferredPipeline::buildShadowCaster(ProjectionLight& projectionLight) {
    return false;
}
void DeferredPipeline::deleteAllShadowCasters(Scene& scene) {
    m_ShadowCasters.clearSceneData(scene);
}
void DeferredPipeline::sendGPUDataAllLights(const Scene& scene, const Camera& camera) {
    int maxLights = glm::min(int(scene.getNumLights()), MAX_LIGHTS_PER_PASS);
    Engine::Renderer::sendUniform1Safe("numLights", maxLights);
    int i = 0;
    auto lambda = [&i, this, &camera, maxLights](const auto& container) {
        if (i >= maxLights) {
            return;
        }
        for (auto& light : container) {
            if (i >= maxLights) {
                break;
            }
            auto start = "light[" + std::to_string(i) + "].";
            sendGPUDataLight(camera, *light, start);
            ++i;
        }
    };
    lambda(Engine::priv::PublicScene::GetLights<SunLight>(scene));
    lambda(Engine::priv::PublicScene::GetLights<DirectionalLight>(scene));
    lambda(Engine::priv::PublicScene::GetLights<PointLight>(scene));
    lambda(Engine::priv::PublicScene::GetLights<SpotLight>(scene));
    lambda(Engine::priv::PublicScene::GetLights<RodLight>(scene));
    lambda(Engine::priv::PublicScene::GetLights<ProjectionLight>(scene));
}
void DeferredPipeline::sendGPUDataGI(Skybox* skybox) {
    const auto maxTextures      = getMaxNumTextureUnits() - 1U;
    bool hasGI                  = skybox && skybox->cubemap().get<TextureCubemap>()->hasGlobalIlluminationData();
    auto& blackTexture          = *Texture::Black.get<Texture>();
    TextureCubemap* convolution = hasGI ? skybox->cubemap().get<TextureCubemap>()->getConvolutionTexture().get<TextureCubemap>() : nullptr;
    TextureCubemap* preEnv      = hasGI ? skybox->cubemap().get<TextureCubemap>()->getPreEnvTexture().get<TextureCubemap>() : nullptr;

    Engine::Renderer::sendTextureSafe( "irradianceMap", hasGI ? convolution->address() : blackTexture.address(), maxTextures - 2,
        hasGI ? convolution->getTextureType().toGLType() : blackTexture.getTextureType().toGLType() );
    Engine::Renderer::sendTextureSafe( "prefilterMap", hasGI ? preEnv->address() : blackTexture.address(), maxTextures - 1,
        hasGI ? preEnv->getTextureType().toGLType() : blackTexture.getTextureType().toGLType() );
    Engine::Renderer::sendTextureSafe( "brdfLUT", *Texture::BRDF.get<Texture>(), maxTextures );
}
void DeferredPipeline::sendGPUDataLight(const Camera& camera, const SunLight& sunLight, const std::string& start) {
    auto transform   = sunLight.getComponent<ComponentTransform>();
    auto pos         = glm::vec3{ transform->getPosition() };
    const auto& col  = sunLight.getColor();
    sendUniform4Safe((start + "DataA").c_str(), 0.0f, sunLight.getDiffuseIntensity(), sunLight.getSpecularIntensity(), 0.0f);
    sendUniform4Safe((start + "DataC").c_str(), 0.0f, pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, float(sunLight.getType()));
    sendUniform1Safe("Type", 0.0f);
    sendUniform1Safe("uShadowEnabled", 0);
    if (m_Renderer.m_Lighting) {
        if (m_ShadowCasters.m_ShadowCastersSunHashed[sunLight.sceneID()].contains(&sunLight)) {
            auto& data = *m_ShadowCasters.m_ShadowCastersSunHashed[sunLight.sceneID()].at(&sunLight);
            if (data.m_Enabled) {
                //startingSpot - start from the maxSlot minus the 3 textures used in the GI portion. TODO: better do this without hardcoding
                const auto startingSpot = getMaxNumTextureUnits() - 1U - 3 - DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS;
                data.bindUniformsReading(startingSpot); //TODO: this texture slot might not work for forward lighting
            }
        }
    }
}
int DeferredPipeline::sendGPUDataLight(const Camera& camera, const PointLight& pointLight, const std::string& start) {
    auto transform  = pointLight.getComponent<ComponentTransform>();
    auto pos        = glm::vec3{ transform->getPosition() };
    auto cull       = pointLight.getCullingRadius();
    auto factor     = 1100.0f * cull;
    auto distSq     = (float)camera.getDistanceSquared(pos);

    if ((!Engine::priv::Culling::sphereIntersectTest(pos, cull, camera)) || (distSq > factor * factor)) {
        return 0;
    }
    const auto& col = pointLight.getColor();
    sendUniform4Safe((start + "DataA").c_str(), 0.0f, pointLight.getDiffuseIntensity(), pointLight.getSpecularIntensity(), 0.0f);
    sendUniform4Safe((start + "DataB").c_str(), 0.0f, 0.0f, pointLight.getConstant(), pointLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), pointLight.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, float(pointLight.getType()));
    sendUniform4Safe((start + "DataE").c_str(), 0.0f, 0.0f, float(pointLight.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);
    sendUniform1Safe("uShadowEnabled", 0);
    if (distSq <= (cull * cull)) { //inside the light volume
        return 1;
    }
    return 2;
}
void DeferredPipeline::sendGPUDataLight(const Camera& camera, const DirectionalLight& directionalLight, const std::string& start) {
    auto transform  = directionalLight.getComponent<ComponentTransform>();
    auto forward    = transform->getForward();
    const auto& col = directionalLight.getColor();
    sendUniform4Safe((start + "DataA").c_str(), 0.0f, directionalLight.getDiffuseIntensity(), directionalLight.getSpecularIntensity(), forward.x);
    sendUniform4Safe((start + "DataB").c_str(), forward.y, forward.z, 0.0f, 0.0f);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, float(directionalLight.getType()));
    sendUniform1Safe("Type", 0.0f);
    sendUniform1Safe("uShadowEnabled", 0);
    if (m_Renderer.m_Lighting) {
        if (m_ShadowCasters.m_ShadowCastersDirectionalHashed[directionalLight.sceneID()].contains(&directionalLight)) {
            auto& data = *m_ShadowCasters.m_ShadowCastersDirectionalHashed[directionalLight.sceneID()].at(&directionalLight);
            if (data.m_Enabled) {
                //startingSpot - start from the maxSlot minus the 3 textures used in the GI portion. TODO: better do this without hardcoding
                const auto startingSpot = getMaxNumTextureUnits() - 1U - 3 - DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS;
                data.bindUniformsReading(startingSpot); //TODO: this texture slot might not work for forward lighting
            }
        }
    }
}
int DeferredPipeline::sendGPUDataLight(const Camera& camera, const SpotLight& spotLight, const std::string& start) {
    auto transform = spotLight.getComponent<ComponentTransform>();
    auto pos       = glm::vec3{ transform->getPosition() };
    auto forward   = transform->getForward();
    auto cull      = spotLight.getCullingRadius();
    auto factor    = 1100.0f * cull;
    auto distSq    = float(camera.getDistanceSquared(pos));
    if (!Engine::priv::Culling::sphereIntersectTest(pos, cull, camera)) {
        return 0;
    }
    if (distSq > factor * factor) {
        return 0;
    }
    const auto& col = spotLight.getColor();

    auto cosRad = [](float in) { return glm::cos(glm::radians(in)); };

    sendUniform4Safe((start + "DataA").c_str(), 0.0f, spotLight.getDiffuseIntensity(), spotLight.getSpecularIntensity(), forward.x);
    sendUniform4Safe((start + "DataB").c_str(), forward.y, forward.z, spotLight.getConstant(), spotLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), spotLight.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, float(spotLight.getType()));
    sendUniform4Safe((start + "DataE").c_str(), cosRad(spotLight.getCutoff()), cosRad(spotLight.getCutoffOuter()), float(spotLight.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 2.0f);
    sendUniform1Safe("uShadowEnabled", 0);
    if (distSq <= (cull * cull)) { //inside the light volume
        return 1;
    }
    return 2;
}
int DeferredPipeline::sendGPUDataLight(const Camera& camera, const RodLight& rodLight, const std::string& start) {
    auto transform       = rodLight.getComponent<ComponentTransform>();
    auto pos             = glm::vec3{ transform->getPosition() };
    auto cullingDistance = rodLight.getRodLength() + (rodLight.getCullingRadius() * 2.0f);
    auto factor          = 1100.0f * cullingDistance;
    auto distSq          = float(camera.getDistanceSquared(pos));
    if (!Engine::priv::Culling::sphereIntersectTest(pos, cullingDistance, camera) || (distSq > factor * factor)) {
        return 0;
    }
    const auto& col      = rodLight.getColor();
    float half           = rodLight.getRodLength() / 2.0f;
    auto firstEndPt      = (pos + (transform->getForward() * half)) - glm::vec3{ m_CameraUBODataPtr->CameraInfo3 };
    auto secndEndPt      = (pos - (transform->getForward() * half)) - glm::vec3{ m_CameraUBODataPtr->CameraInfo3 };
    sendUniform4Safe((start + "DataA").c_str(), 0.0f, rodLight.getDiffuseIntensity(), rodLight.getSpecularIntensity(), firstEndPt.x);
    sendUniform4Safe((start + "DataB").c_str(), firstEndPt.y, firstEndPt.z, rodLight.getConstant(), rodLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), rodLight.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, float(rodLight.getType()));
    sendUniform4Safe((start + "DataE").c_str(), rodLight.getRodLength(), 0.0f, float(rodLight.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);
    sendUniform1Safe("uShadowEnabled", 0);
    if (distSq <= (cullingDistance * cullingDistance)) {
        return 1;
    }
    return 2;
}
int DeferredPipeline::sendGPUDataLight(const Camera& camera, const ProjectionLight& rodLight, const std::string& start) {
    return 2;
}
void DeferredPipeline::renderDirectionalLight(Camera& camera, DirectionalLight& directionalLight, Viewport& viewport) {
    if (!directionalLight.isActive()) {
        return;
    }
    std::string start = "light.";
    sendGPUDataLight(camera, directionalLight, start);
    const auto viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void DeferredPipeline::renderSunLight(Camera& camera, SunLight& sunLight, Viewport& viewport) {
    if (!sunLight.isActive()) {
        return;
    }
    std::string start = "light.";
    sendGPUDataLight(camera, sunLight, start);
    const auto viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void DeferredPipeline::renderPointLight(Camera& camera, PointLight& pointLight) {
    if (!pointLight.isActive()) {
        return;
    }
    std::string start = "light.";
    int result   = sendGPUDataLight(camera, pointLight, start);
    if (result == 0) {
        return;
    }
    auto transform       = pointLight.getComponent<ComponentTransform>();
    auto renderingMatrix = transform->getWorldMatrixRendering();
    sendUniformMatrix4("Model", renderingMatrix);
    sendUniformMatrix4("VP", m_CameraUBODataPtr->CameraViewProj);

    if (result == 1) {
        cullFace(GL_FRONT);
    } else if (result == 2) {
        cullFace(GL_BACK);
    }
    auto& pointLightMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPointLightBounds().get<Mesh>();
    m_Renderer.bind(&pointLightMesh);
    renderMesh(pointLightMesh);
    m_Renderer.unbind(&pointLightMesh);

    cullFace(GL_BACK);
}
void DeferredPipeline::renderSpotLight(Camera& camera, SpotLight& spotLight) {
    if (!spotLight.isActive()) {
        return;
    }
    std::string start = "light.";
    int result   = sendGPUDataLight(camera, spotLight, start);

    if (result == 0) {
        return;
    }
    auto transform       = spotLight.getComponent<ComponentTransform>();
    auto renderingMatrix = transform->getWorldMatrixRendering();
    sendUniformMatrix4("Model", renderingMatrix);
    sendUniformMatrix4("VP", m_CameraUBODataPtr->CameraViewProj);

    if (result == 1) {
        cullFace(GL_FRONT);
    } else if (result == 2) {
        cullFace(GL_BACK);
    }
    auto& spotLightMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getSpotLightBounds().get<Mesh>();
    m_Renderer.bind(&spotLightMesh);
    renderMesh(spotLightMesh);
    m_Renderer.unbind(&spotLightMesh);

    cullFace(GL_BACK);
}
void DeferredPipeline::renderRodLight(Camera& camera, RodLight& rodLight) {
    if (!rodLight.isActive()) {
        return;
    }
    std::string start = "light.";
    int result   = sendGPUDataLight(camera, rodLight, start);

    if (result == 0) {
        return;
    }
    auto transform       = rodLight.getComponent<ComponentTransform>();
    auto renderingMatrix = transform->getWorldMatrixRendering();
    sendUniformMatrix4("Model", renderingMatrix);
    sendUniformMatrix4("VP", m_CameraUBODataPtr->CameraViewProj);

    if (result == 1) {
        cullFace(GL_FRONT);
    }else if (result == 2) {
        cullFace(GL_BACK);
    }

    auto& rodLightMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getRodLightBounds().get<Mesh>();
    m_Renderer.bind(&rodLightMesh);
    renderMesh(rodLightMesh);
    m_Renderer.unbind(&rodLightMesh);

    cullFace(GL_BACK);
}
void DeferredPipeline::renderProjectionLight(Camera& camera, ProjectionLight& projectionLight) {
    if (!projectionLight.isActive()) {
        return;
    }
    std::string start = "light.";
    int result = sendGPUDataLight(camera, projectionLight, start);

    if (result == 0) {
        return;
    }
    Engine::Renderer::sendTextureSafe("gTextureMap", *projectionLight.getTexture().get<Texture>(), 5);

    if (result == 1) {
        cullFace(GL_FRONT);
    }else if (result == 2) {
        cullFace(GL_BACK);
    }

    auto& projLightMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getProjectionLightBounds().get<Mesh>();

    projLightMesh.modifyVertices(0, projectionLight.getPoints().data(), projectionLight.getPoints().size());
    projLightMesh.modifyIndices(projectionLight.getIndices().data(), projectionLight.getIndices().size());

    m_Renderer.bind(&projLightMesh);
    renderMesh(projLightMesh);
    m_Renderer.unbind(&projLightMesh);

    cullFace(GL_BACK);
}
void DeferredPipeline::renderDecal(ModelInstance& decalModelInstance) {
    Entity parent          = decalModelInstance.getOwner();
    auto transform         = parent.getComponent<ComponentTransform>();
    glm::mat4 parentModel  = transform->getWorldMatrixRendering();
    auto maxTextures       = getMaxNumTextureUnits() - 1U;

    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform1Safe("Object_Color", decalModelInstance.getColor().toPackedInt());
    Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", decalModelInstance.getGodRaysColor().toPackedInt());

    glm::mat4 modelMatrix  = parentModel * decalModelInstance.getModelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3{ modelMatrix }));

    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}
void DeferredPipeline::renderParticles(ParticleSystem& system, Camera& camera, Handle program) {
    const size_t particle_count = system.ParticlesDOD.size();
    if (particle_count > 0) {
        m_Renderer.bind(program.get<ShaderProgram>());
        for (auto it = system.Bimap.rbegin(); it != system.Bimap.rend(); ++it) {
            system.MaterialIDToIndex.try_emplace(it->first, (uint32_t)system.MaterialIDToIndex.size());
        }
        for (auto& particlePOD : system.ParticlesDOD) {
            particlePOD.MatID = system.MaterialIDToIndex.at(particlePOD.MatID);
        }
        for (const auto& [id, index] : system.MaterialIDToIndex) {
            Material* mat              = system.Bimap.at(id);
            Texture& texture           = *mat->getComponent(MaterialComponentType::Diffuse).getTexture(0).get<Texture>();
            const std::string location = "Tex" + std::to_string(index);
            Engine::Renderer::sendTextureSafe(location.c_str(), texture, index);
        }
        const auto maxTextures = getMaxNumTextureUnits() - 1U;
        Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);

        glBindBuffer(GL_ARRAY_BUFFER, m_Particle_Instance_VBO);
        glBufferData(GL_ARRAY_BUFFER, particle_count * sizeof(Engine::priv::ParticleDOD), system.ParticlesDOD.data(), GL_STREAM_DRAW);

        auto& particleMesh = *Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>();
        m_Renderer.bind(&particleMesh);
        glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)particleMesh.getVertexData().m_Indices.size(), GL_UNSIGNED_INT, 0, (GLsizei)particle_count);
        m_Renderer.unbind(&particleMesh);
    }
}
void DeferredPipeline::renderMesh(Mesh& mesh, uint32_t mode) {
    const auto indicesSize = mesh.getVertexData().m_Indices.size();
    if (indicesSize > 0) {
        glDrawElements(mode, (GLsizei)indicesSize, GL_UNSIGNED_INT, nullptr);
    }
}
void DeferredPipeline::renderLightProbe(LightProbe& lightProbe) {
    //goal: render all 6 sides into a fbo and into a cubemap, and have that cubemap stored in the light probe to be used for Global Illumination
}
void DeferredPipeline::render2DText(const std::string& text, Handle fontHandle, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, TextAlignment textAlignment, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    Font& font = *fontHandle.get<Font>();
    Handle textureHandle = font.getGlyphTexture();
    float y = 0.0f;
    float x = 0.0f;
    float z = -0.001f - depth;

    glm::mat4 modelMatrix = glm::mat4{ 1.0f };
    modelMatrix           = glm::translate(modelMatrix, glm::vec3{ position.x, position.y, 0 });
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3{ 0, 0, 1 });
    modelMatrix           = glm::scale(modelMatrix, glm::vec3{ scale.x, scale.y, 1 });

    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    Engine::Renderer::sendUniform4("Object_Color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Engine::priv::OpenGLBindTextureRAII DiffuseTexture{ "DiffuseTexture", *textureHandle.get<Texture>(), 0, false };

    Mesh& fontPlane = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh().get<Mesh>();
    m_TextRenderer.renderText(fontPlane, m_Renderer, text, font, color, textAlignment, x, y, z);

    renderMesh(fontPlane);
    m_Renderer.unbind(&fontPlane);
}
void DeferredPipeline::render2DTexture(Handle textureHandle, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment align, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    float translationX = position.x;
    float translationY = position.y;
    float totalSizeX   = scale.x;
    float totalSizeY   = scale.y;

    Texture* texture   = textureHandle.get<Texture>();

    Engine::priv::OpenGLBindTextureRAII DiffuseTexture{ 
        "DiffuseTexture",
        texture ? texture->address() : 0, 
        texture ? texture->getTextureType().toGLType() : GL_TEXTURE_2D, 
        0,
        false 
    };
    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", texture ? 1 : 0);
    if (texture) {
        totalSizeX *= texture->width();
        totalSizeY *= texture->height();
    }
    Engine::Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

    glm::mat4 modelMatrix = glm::mat4{ 1.0f };
    modelMatrix           = glm::translate(modelMatrix, glm::vec3{ translationX, translationY, -0.001f - depth });
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3{ 0, 0, 1 });
    modelMatrix           = glm::scale(modelMatrix, glm::vec3{ totalSizeX, totalSizeY, 1.0f });

    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Mesh& plane2D = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlane2DMesh().get<Mesh>();
    m_Renderer.bind(&plane2D);
    renderMesh(plane2D);
    m_Renderer.unbind(&plane2D);
}
void DeferredPipeline::render2DTexture(uint32_t textureAddress, int textureWidth, int textureHeight, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment align, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    float translationX = position.x;
    float translationY = position.y;
    float totalSizeX   = scale.x;
    float totalSizeY   = scale.y;

    totalSizeX *= textureWidth;
    totalSizeY *= textureHeight;

    Engine::Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

    glm::mat4 modelMatrix = glm::mat4{ 1.0f };
    modelMatrix           = glm::translate(modelMatrix, glm::vec3{ translationX, translationY, -0.001f - depth });
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3{ 0, 0, 1 });
    modelMatrix           = glm::scale(modelMatrix, glm::vec3{ totalSizeX, totalSizeY, 1.0f });

    Engine::priv::OpenGLBindTextureRAII DiffuseTexture{ "DiffuseTexture", textureAddress, GL_TEXTURE_2D, 0, false };
    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Mesh& plane2D = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlane2DMesh().get<Mesh>();
    m_Renderer.bind(&plane2D);
    renderMesh(plane2D);
    m_Renderer.unbind(&plane2D);
}
void DeferredPipeline::render2DTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment alignment, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    float translationX = position.x;
    float translationY = position.y;

    Engine::Renderer::alignmentOffset(alignment, translationX, translationY, width, height);

    glm::mat4 modelMatrix = glm::mat4{ 1.0f };
    modelMatrix           = glm::translate(modelMatrix, glm::vec3{ translationX, translationY, -0.001f - depth });
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3{ 0, 0, 1 });
    modelMatrix           = glm::scale(modelMatrix, glm::vec3{ width, height, 1 });

    Engine::priv::OpenGLBindTextureRAII DiffuseTexture{ "DiffuseTexture", 0, GL_TEXTURE_2D, 0, false };
    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Mesh& triangle = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh().get<Mesh>();
    m_Renderer.bind(&triangle);
    renderMesh(triangle);
    m_Renderer.unbind(&triangle);
}
void DeferredPipeline::renderInitFrame(Engine::priv::RenderModule& renderer) {
    m_GBuffer.bindBackbuffer();
    Engine::Renderer::Settings::clear(true, true, true); // clear all
}
void DeferredPipeline::internal_render_per_frame_preparation(Viewport& viewport, Camera& camera) {
    const auto winSize             = Engine::Resources::getWindowSize();
    const auto& viewportDimensions = viewport.getViewportDimensions();

    /*
    for (uint32_t i = 0; i < Engine::priv::OpenGLState::constants.MAX_COMBINED_TEXTURE_IMAGE_UNITS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    */

    //if resizing gbuffer
        //m_GBuffer.resize(uint32_t(viewportDimensions.z), uint32_t(viewportDimensions.w));
    //else
        //glScissor(viewportDimensions.x, viewportDimensions.y, viewportDimensions.z, viewportDimensions.w);

    m_2DProjectionMatrix = glm::ortho(0.0f, viewportDimensions.z, 0.0f, viewportDimensions.w, 0.003f, 3000.0f); //TODO: might have to recheck this

    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure
}
void DeferredPipeline::internal_init_frame_gbuffer(Viewport& viewport, Camera& camera) {
    m_GBuffer.bindFramebuffers({ GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, GBufferType::Lighting }, "RGBA", true);
    Engine::Renderer::Settings::clear(true, true, true); // clear all

    m_GBuffer.bindFramebuffers({ GBufferType::Bloom, GBufferType::GodRays }, "RGBA", false);
    Engine::Renderer::Settings::clear(true, true, true); // clear all
}
void DeferredPipeline::internal_pass_shadows_depth(Viewport& viewport, Scene& scene, Camera& camera) {
    if (!m_Renderer.m_Lighting) {
        return;
    }
    const auto sceneID = scene.id();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::ShadowDepth].get<ShaderProgram>());
    Engine::Renderer::colorMask(false, false, false, false);

    glDisable(GL_CULL_FACE);
    //glPolygonOffset(1.0, 1.0);

    //directional lights
    if (m_ShadowCasters.m_ShadowCastersDirectional.size() > sceneID) {
        for (auto& [directionalLight, data] : m_ShadowCasters.m_ShadowCastersDirectional[sceneID]) {
            if (data.m_Enabled) {
                data.calculateOrthographicProjections(camera, directionalLight->getDirection());
                Engine::Renderer::setViewport(0.0f, 0.0f, data.m_ShadowWidth, data.m_ShadowHeight);
                for (int i = 0; i < int(data.m_LightSpaceMatrices.size()); ++i) {
                    data.bindUniformsWriting(i);
                    const auto& viewProj = data.m_LightSpaceMatrices[i];

                    PublicScene::RenderGeometryOpaqueShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderGeometryTransparentShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderGeometryTransparentTrianglesSortedShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardOpaqueShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardTransparentShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardTransparentTrianglesSortedShadowMap(m_Renderer, scene, nullptr, viewProj);
                }
            }
        }
    }
    //sun lights
    if (m_ShadowCasters.m_ShadowCastersSun.size() > sceneID) {
        for (auto& [sunLight, data] : m_ShadowCasters.m_ShadowCastersSun[scene.id()]) {
            if (data.m_Enabled) {
                data.calculateOrthographicProjections(camera, glm::normalize(sunLight->getPosition() - camera.getPosition()));
                Engine::Renderer::setViewport(0.0f, 0.0f, data.m_ShadowWidth, data.m_ShadowHeight);
                for (int i = 0; i < int(data.m_LightSpaceMatrices.size()); ++i) {
                    data.bindUniformsWriting(i);
                    const auto& viewProj = data.m_LightSpaceMatrices[i];

                    PublicScene::RenderGeometryOpaqueShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderGeometryTransparentShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderGeometryTransparentTrianglesSortedShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardOpaqueShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardTransparentShadowMap(m_Renderer, scene, nullptr, viewProj);
                    PublicScene::RenderForwardTransparentTrianglesSortedShadowMap(m_Renderer, scene, nullptr, viewProj);
                }
            }
        }
    }

    glEnable(GL_CULL_FACE);
    //glPolygonOffset(0, 0);
    const auto& viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::setViewport(viewportDimensions);
    Engine::Renderer::colorMask(true, true, true, true);
}
bool DeferredPipeline::internal_pass_depth_prepass(Viewport& viewport, Camera& camera) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::ZPrepass].get<ShaderProgram>());

    Scene& scene = viewport.getScene();

    PublicScene::RenderGeometryOpaque(m_Renderer, scene, &viewport, &camera, false);
    if (viewport.getRenderFlags().has(ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    //PublicScene::RenderGeometryTransparent(m_Renderer, scene, &viewport, &camera, false);
    //PublicScene::RenderGeometryTransparentTrianglesSorted(m_Renderer, scene, &viewport, &camera, true, false);

    PublicScene::RenderForwardOpaque(m_Renderer, scene, &viewport, &camera, false);
    //PublicScene::RenderForwardTransparent(m_Renderer, scene, &viewport, &camera, false);
    //PublicScene::RenderForwardTransparentTrianglesSorted(m_Renderer, scene, &viewport, &camera, false);

    glDepthMask(GL_FALSE);
    return true;
}
void DeferredPipeline::internal_pass_geometry(Viewport& viewport, Camera& camera) {
    Scene& scene                   = viewport.getScene();
    const auto& viewportDimensions = viewport.getViewportDimensions();
    const auto gbufferSize         = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };

    m_GBuffer.bindFramebuffers({ GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc }, "RGBA", true, 
        (gbufferSize.x / 2.0f) - (viewportDimensions.z / 2.0f),
        (gbufferSize.y / 2.0f) - (viewportDimensions.w / 2.0f),
        viewportDimensions.z, 
        viewportDimensions.w
    );
    Engine::Renderer::Settings::clear(true, false, false); // clear color only
    
    PublicScene::RenderGeometryOpaque(m_Renderer, scene, &viewport, &camera);
    
    if (m_Renderer.m_DrawSkybox && viewport.getRenderFlags().has(ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnablei(GL_BLEND, 0);
    PublicScene::RenderGeometryTransparent(m_Renderer, scene, &viewport, &camera);
    PublicScene::RenderGeometryTransparentTrianglesSorted(m_Renderer, scene, &viewport, &camera, true);    
    glDisablei(GL_BLEND, 0);
}
void DeferredPipeline::internal_pass_forward(Viewport& viewport, Camera& camera, bool depthPrepass) {
    Scene& scene                   = viewport.getScene();
    const auto& viewportDimensions = viewport.getViewportDimensions();
    const auto gbufferSize         = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };

    m_GBuffer.bindFramebuffers({ GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, GBufferType::Lighting }, "RGBA", true, 
        (gbufferSize.x / 2.0f) - (viewportDimensions.z / 2.0f),
        (gbufferSize.y / 2.0f) - (viewportDimensions.w / 2.0f),
        viewportDimensions.z, 
        viewportDimensions.w
    );

    PublicScene::RenderForwardOpaque(m_Renderer, scene, &viewport, &camera);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (uint32_t i = 0; i < 4; ++i) {
        glEnablei(GL_BLEND, i);
    }
    if (!depthPrepass) {
        glDepthMask(GL_TRUE);
    }
    PublicScene::RenderForwardTransparent(m_Renderer, scene, &viewport, &camera);
    PublicScene::RenderForwardTransparentTrianglesSorted(m_Renderer, scene, &viewport, &camera);
    if (!depthPrepass) {
        glDepthMask(GL_FALSE);
    }
    PublicScene::RenderDecals(m_Renderer, scene, &viewport, &camera);
    PublicScene::RenderForwardParticles(m_Renderer, scene, &viewport, &camera);
    PublicScene::RenderParticles(m_Renderer, scene, viewport, camera, m_InternalShaderPrograms[ShaderProgramEnum::Particle]);

    for (uint32_t i = 0; i < 4; ++i) {
        glDisablei(GL_BLEND, i); //this is needed for smaa at least
    }
}
void DeferredPipeline::internal_pass_ssao(Viewport& viewport, Camera& camera) {

    auto framebuffer1 = GBufferType::Bloom;
    auto framebuffer2 = GBufferType::GodRays;

    //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes
    m_GBuffer.bindFramebuffers({ framebuffer1, framebuffer2 }, "A", false);
    Engine::Renderer::Settings::clear(true, false, false); //bloom and god rays alpha channels cleared to black 
    if (SSAO::STATIC_SSAO.m_SSAOLevel > SSAOLevel::Off && viewport.getRenderFlags().has(ViewportRenderingFlag::SSAO)) {
        Engine::Renderer::GLEnablei(GL_BLEND, 0);//i dont think this is needed anymore
        m_GBuffer.bindFramebuffers({ framebuffer1 }, "A", false);
        SSAO::STATIC_SSAO.passSSAO(m_GBuffer, viewport, camera, m_Renderer);

        if (SSAO::STATIC_SSAO.m_DoBlur) {
            Engine::Renderer::GLDisablei(GL_BLEND, 0); //yes this is absolutely needed
            for (int i = 0; i < SSAO::STATIC_SSAO.m_BlurNumPasses; ++i) {
                m_GBuffer.bindFramebuffers({ framebuffer2 }, "A", false);
                SSAO::STATIC_SSAO.passBlur(m_GBuffer, viewport, framebuffer1, m_Renderer);
                m_GBuffer.bindFramebuffers({ framebuffer1 }, "A", false);
                SSAO::STATIC_SSAO.passBlurCopyPixels(m_GBuffer, viewport, framebuffer2, m_Renderer);
            }
        }  
    }
}
void DeferredPipeline::internal_pass_stencil(Viewport& viewport) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::colorMask(false, false, false, false);

    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::StencilPass].get<ShaderProgram>());

    m_GBuffer.getMainFBO().bind();

    Engine::Renderer::GLEnable(GL_STENCIL_TEST);
    Engine::Renderer::Settings::clear(false, false, true); //stencil is completely filled with 0's
    Engine::Renderer::stencilMask(0b11111111);

    //if a fragment is written, it passed. discard in the glsl shader is needed to fail this test.
    Engine::Renderer::stencilFunc(GL_ALWAYS, 0b00000000, 0b00000000);

    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR, GL_INCR);

    //this sneaky shader discards pixels that are "shadeless" based on their normal being (1,1,1). the discarding allows the stencil to not be written and thus disabling these pixels
    //from the lighting calculations
    Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);

    Engine::Renderer::stencilMask(0b11111111);

    //any fragments happening later on pass only if the value in the buffer is NOT EQUAL to zero.
    Engine::Renderer::stencilFunc(GL_NOTEQUAL, 0b00000000, 0b11111111);

    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil
    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::internal_pass_lighting(Viewport& viewport, Camera& camera, bool mainRenderFunction) {
    const Scene& scene             = viewport.getScene();
    const auto& viewportDimensions = viewport.getViewportDimensions();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLighting].get<ShaderProgram>());
    auto winSize = glm::vec2{ Engine::Resources::getWindowSize() };
    Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities);

    //TODO: is this needed?
    {
        Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0, false };
        Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1, false };
        Engine::priv::OpenGLBindTextureRAII gMiscMap{ "gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 2, false };
        Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 3, false };
        Engine::priv::OpenGLBindTextureRAII gSSAOMap{ "gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 4, false };

        Engine::Renderer::setDepthFunc(GL_GEQUAL);
        Engine::Renderer::GLEnable(GL_DEPTH_TEST);

        for (const auto& light : PublicScene::GetLights<PointLight>(scene)) {
            renderPointLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<SpotLight>(scene)) {
            renderSpotLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<RodLight>(scene)) {
            renderRodLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<ProjectionLight>(scene)) {
            renderProjectionLight(camera, *light);
        }
        Engine::Renderer::setDepthFunc(GL_LEQUAL);
        Engine::Renderer::GLDisable(GL_DEPTH_TEST);
        for (const auto& light : PublicScene::GetLights<SunLight>(scene)) {
            renderSunLight(camera, *light, viewport);
        }
        for (const auto& light : PublicScene::GetLights<DirectionalLight>(scene)) {
            renderDirectionalLight(camera, *light, viewport);
        }
        Engine::Renderer::clearTexture(5, GL_TEXTURE_2D);
    }
    if (mainRenderFunction) {
        //do GI here. (only doing GI during the main render pass, not during light probes
        glBlendEquationSeparate(GL_MAX, GL_FUNC_ADD);
        m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGI].get<ShaderProgram>());
        Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities);

        Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0, false };
        Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1, false };
        Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 2, false };
        Engine::priv::OpenGLBindTextureRAII gSSAOMap{ "gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 3, false };
        Engine::priv::OpenGLBindTextureRAII gMiscMap{ "gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 4, false };

        Skybox* skybox = scene.skybox();
        sendGPUDataGI(skybox);

        Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);

        const auto maxTextures = getMaxNumTextureUnits() - 1U;
        bool hasGI = skybox && skybox->cubemap().get<TextureCubemap>()->hasGlobalIlluminationData();
        Engine::Renderer::clearTexture(maxTextures - 2, hasGI ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(maxTextures - 1, hasGI ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(maxTextures, GL_TEXTURE_2D);

        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    }
}
void DeferredPipeline::internal_pass_lighting_basic(Viewport& viewport, Camera& camera, bool mainRenderFunction) {
    const Scene& scene             = viewport.getScene();
    const auto& viewportDimensions = viewport.getViewportDimensions();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingBasic].get<ShaderProgram>());
    auto winSize = glm::vec2{ Engine::Resources::getWindowSize() };
    
    Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities);
    {

        Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0, false };
        Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1, false };
        Engine::priv::OpenGLBindTextureRAII gMiscMap{ "gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 2, false };
        Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 3, false };
        Engine::priv::OpenGLBindTextureRAII gSSAOMap{ "gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 4, false };

        Engine::Renderer::setDepthFunc(GL_GEQUAL);
        Engine::Renderer::GLEnable(GL_DEPTH_TEST);

        for (const auto& light : PublicScene::GetLights<PointLight>(scene)) {
            renderPointLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<SpotLight>(scene)) {
            renderSpotLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<RodLight>(scene)) {
            renderRodLight(camera, *light);
        }
        for (const auto& light : PublicScene::GetLights<ProjectionLight>(scene)) {
            renderProjectionLight(camera, *light);
        }
        Engine::Renderer::setDepthFunc(GL_LEQUAL);
        Engine::Renderer::GLDisable(GL_DEPTH_TEST);
        for (const auto& light : PublicScene::GetLights<SunLight>(scene)) {
            renderSunLight(camera, *light, viewport);
        }
        for (const auto& light : PublicScene::GetLights<DirectionalLight>(scene)) {
            renderDirectionalLight(camera, *light, viewport);
        }
    }
    if (mainRenderFunction) {
        //do GI here. (only doing GI during the main render pass, not during light probes)
        m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGIBasic].get<ShaderProgram>());

        Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0, false };
        Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1, false };

        Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
    }
}
void DeferredPipeline::internal_pass_god_rays(Viewport& viewport, Camera& camera) {
    m_GBuffer.bindFramebuffers({ GBufferType::GodRays }, "RGB", false);
    Engine::Renderer::Settings::clear(true, false, false); //godrays rgb channels cleared to black
    auto& godRaysPlatform = GodRays::STATIC_GOD_RAYS;
    auto sun = Engine::Renderer::godRays::getSun();
    if (sun && viewport.getRenderFlags().has(ViewportRenderingFlag::GodRays) && godRaysPlatform.godRays_active) {
        auto transform    = sun.getComponent<ComponentTransform>();
        if (!transform) {
            return;
        }
        glm::vec3 oPos    = transform->getPosition();
        glm::vec3 camPos  = camera.getPosition();
        glm::vec3 camVec  = camera.getViewVector();
        bool infront      = Engine::Math::isPointWithinCone(camPos, -camVec, oPos, Engine::Math::toRadians(godRaysPlatform.fovDegrees));
        if (infront) {
            const auto sp = Engine::Math::getScreenCoordinates(oPos, camera, false);
            const auto b  = glm::normalize(camPos - oPos);
            float alpha   = Engine::Math::getAngleBetweenTwoVectors(camVec, b, true) / godRaysPlatform.fovDegrees;
            alpha         = glm::pow(alpha, godRaysPlatform.alphaFalloff);
            alpha         = glm::clamp(alpha, 0.01f, 0.99f);
            if (boost::math::isnan(alpha) || boost::math::isinf(alpha)) { //yes this is needed...
                alpha     = 0.01f;
            }
            alpha = 1.0f - alpha;
            godRaysPlatform.pass(m_GBuffer, viewport, glm::vec2{ sp.x, sp.y }, alpha, m_Renderer);
        }
    }
}
void DeferredPipeline::internal_pass_hdr(Viewport& viewport, Camera& camera, GBufferType::Type outTexture, GBufferType::Type outTexture2) {
    HDR::STATIC_HDR.pass(m_GBuffer, viewport, outTexture, outTexture2, GodRays::STATIC_GOD_RAYS.godRays_active, GodRays::STATIC_GOD_RAYS.factor, m_Renderer);
}
void DeferredPipeline::internal_pass_bloom(Viewport& viewport, GBufferType::Type sceneTexture) {
    if (Bloom::STATIC_BLOOM.m_Bloom_Active && viewport.getRenderFlags().has(ViewportRenderingFlag::Bloom)) {
        m_GBuffer.bindFramebuffers({ GBufferType::Bloom }, "RGB", false);
        Bloom::STATIC_BLOOM.pass(m_GBuffer, viewport, sceneTexture, m_Renderer);
        for (int i = 0; i < Bloom::STATIC_BLOOM.m_Num_Passes; ++i) {
            m_GBuffer.bindFramebuffers({ GBufferType::GodRays }, "RGB", false);
            internal_pass_blur(viewport, GBufferType::Bloom, "H");
            m_GBuffer.bindFramebuffers({ GBufferType::Bloom }, "RGB", false);
            internal_pass_blur(viewport, GBufferType::GodRays, "V");
        }
    }
}
void DeferredPipeline::internal_pass_depth_of_field(Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (DepthOfField::STATIC_DOF.dof && viewport.getRenderFlags().has(ViewportRenderingFlag::DepthOfField)) {
        m_GBuffer.bindFramebuffers({ outTexture }, "RGBA");
        DepthOfField::STATIC_DOF.pass(m_GBuffer, viewport, sceneTexture, m_Renderer);
        std::swap(sceneTexture, outTexture);
    }
}
void DeferredPipeline::internal_pass_aa(bool mainRenderFunction, Viewport& viewport, Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (!mainRenderFunction || m_Renderer.m_AA_algorithm == AntiAliasingAlgorithm::None || !viewport.getRenderFlags().has(ViewportRenderingFlag::AntiAliasing)) {
        m_GBuffer.bindFramebuffers({ outTexture }, "RGBA");
        internal_pass_final(viewport, sceneTexture);
        m_GBuffer.bindBackbuffer(viewport);   
        internal_pass_depth_and_transparency(viewport, outTexture);
    } else {
        switch (m_Renderer.m_AA_algorithm) {
            case AntiAliasingAlgorithm::None: {
                break;
            }
            case AntiAliasingAlgorithm::FXAA: {
                if (mainRenderFunction) {
                    m_GBuffer.bindFramebuffers({ outTexture }, "RGBA");
                    internal_pass_final(viewport, sceneTexture);
                    m_GBuffer.bindFramebuffers({ sceneTexture }, "RGBA");
                    FXAA::STATIC_FXAA.pass(m_GBuffer, viewport, outTexture, m_Renderer);
                    m_GBuffer.bindBackbuffer(viewport);
                    internal_pass_depth_and_transparency(viewport, sceneTexture);
                }
                break;
            }
            case AntiAliasingAlgorithm::SMAA_LOW: {}
            case AntiAliasingAlgorithm::SMAA_MED: {}
            case AntiAliasingAlgorithm::SMAA_HIGH: {}
            case AntiAliasingAlgorithm::SMAA_ULTRA: {
                if (mainRenderFunction) {
                    m_GBuffer.bindFramebuffers({ outTexture }, "RGBA");
                    internal_pass_final(viewport, sceneTexture);
                    std::swap(sceneTexture, outTexture);
                    const auto gbufferSize          = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };
                    const auto& viewportDimensions  = viewport.getViewportDimensions();


                    const glm::vec4 SMAA_PIXEL_SIZE = glm::vec4(1.0f / gbufferSize.x, 1.0f / gbufferSize.y, gbufferSize.x, gbufferSize.y);
                    SMAA::STATIC_SMAA.passEdge(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture, outTexture, m_Renderer);
                    SMAA::STATIC_SMAA.passBlend(m_GBuffer, SMAA_PIXEL_SIZE, viewport, outTexture, m_Renderer);
                    m_GBuffer.bindFramebuffers({ outTexture }, "RGBA");
                    SMAA::STATIC_SMAA.passNeighbor(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture, m_Renderer);
                    //m_GBuffer.bindFramebuffers({ sceneTexture }, "RGBA");
                    //SMAA::STATIC_SMAA.passFinal(m_GBuffer, viewport, m_Renderer);
                    m_GBuffer.bindBackbuffer(viewport);
                    internal_pass_depth_and_transparency(viewport, outTexture);
                }
                break;
            }
        }
    }
}
void DeferredPipeline::internal_pass_final(Viewport& viewport, GBufferType::Type sceneTexture) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredFinal].get<ShaderProgram>());

    Engine::Renderer::sendUniform1Safe("HasBloom", int(Bloom::STATIC_BLOOM.m_Bloom_Active));
    Engine::Renderer::sendUniform1Safe("HasFog", int(Fog::STATIC_FOG.fog_active));

    Engine::priv::OpenGLBindTextureRAII SceneTexture{ "SceneTexture", m_GBuffer.getTexture(sceneTexture), 0, true };
    Engine::priv::OpenGLBindTextureRAII gBloomMap{ "gBloomMap", m_GBuffer.getTexture(GBufferType::Bloom), 1, true };
    Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 2, true };
    Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 3, true };
    if (Engine::Renderer::fog::enabled()) {
        Engine::Renderer::sendUniform1Safe("FogDistNull", Fog::STATIC_FOG.distNull);
        Engine::Renderer::sendUniform1Safe("FogDistBlend", Fog::STATIC_FOG.distBlend);
        Engine::Renderer::sendUniform4Safe("FogColor", Fog::STATIC_FOG.color);
    }
    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void DeferredPipeline::internal_pass_depth_and_transparency(Viewport& viewport, GBufferType::Type sceneTexture) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DepthAndTransparency].get<ShaderProgram>());

    Engine::Renderer::GLEnablei(GL_BLEND, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    //Engine::Renderer::GLEnable(GL_DEPTH_TEST);

    //glDepthFunc(GL_ALWAYS);
    glDepthFunc(GL_LEQUAL);
    // 
    //sendUniform4Safe("TransparencyMaskColor", viewport.getTransparencyMaskColor());
    //sendUniform1Safe("TransparencyMaskActive", int(viewport.isTransparencyMaskActive()));
    Engine::Renderer::sendUniform1Safe("DepthMaskValue", viewport.getDepthMaskValue());
    Engine::Renderer::sendUniform1Safe("DepthMaskActive", int(viewport.isDepthMaskActive()));

    Engine::priv::OpenGLBindTextureRAII SceneTexture{ "SceneTexture", m_GBuffer.getTexture(sceneTexture), 0, true };
    Engine::priv::OpenGLBindTextureRAII gDepthMap{ "gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 1, true };

    Engine::Renderer::renderFullscreenQuad(0.0f, 0.0f, viewport.getDepth(), 0.003f, 6000.0f);

    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);
    //Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    Engine::Renderer::GLDisablei(GL_BLEND, 0);
}
void DeferredPipeline::internal_pass_blur(Viewport& viewport, GLuint texture, std::string_view type) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredBlur].get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    const glm::vec2 hv = (type == "H") ? glm::vec2{ 1.0f, 0.0f } : glm::vec2{ 0.0f, 1.0f };
    const auto& bloom = Bloom::STATIC_BLOOM;
    Engine::Renderer::sendUniform4("strengthModifier",
        bloom.m_Blur_Strength,
        bloom.m_Blur_Strength,
        bloom.m_Blur_Strength,
        SSAO::STATIC_SSAO.m_BlurStrength
    );
    Engine::Renderer::sendUniform4("DataA", bloom.m_Blur_Radius, 0.0f, hv.x, hv.y);

    Engine::priv::OpenGLBindTextureRAII uSceneToBeBlurred{ "uSceneToBeBlurred", m_GBuffer.getTexture(texture), 0, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void DeferredPipeline::internal_pass_normaless_diffuse(Viewport& viewport) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::NormalessDiffuse].get<ShaderProgram>());

    m_GBuffer.bindFramebuffers({ GBufferType::Lighting }, "RGBA");

    Engine::Renderer::sendUniform1Safe("HasLighting", int(m_Renderer.m_Lighting));

    Engine::priv::OpenGLBindTextureRAII gNormalMap{ "gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 0, false };
    Engine::priv::OpenGLBindTextureRAII gDiffuseMap{ "gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 1, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void DeferredPipeline::renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) {
    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    if (mainRenderFunc && viewport.getRenderFlags().has(ViewportRenderingFlag::PhysicsDebug)) {
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
           if (m_Renderer.m_DrawPhysicsDebug && &camera == scene.getActiveCamera()) {
        #endif
                Engine::Renderer::GLDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::BulletPhysics].get<ShaderProgram>());
                Core::m_Engine->m_PhysicsModule.render(scene, camera);
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
           }
        #endif
    }
}
void DeferredPipeline::render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport& viewport, bool clearDepth) {
    glEnablei(GL_BLEND, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    if (mainRenderFunc) {
        if (viewport.getRenderFlags().has(ViewportRenderingFlag::API2D)) {

            //yes this is stupid on opengl's part.. but to be able to write to the depth buffer you need depth testing enabled DESPITE there
            //being a depthMask function that should be the only requirement... well whatever, to always pass depth test - use glDepthFunc(GL_ALWAYS)
            Engine::Renderer::GLEnable(GL_DEPTH_TEST);

            Engine::Renderer::setDepthFunc(GL_LEQUAL);
            //Engine::Renderer::setDepthFunc(GL_ALWAYS);

            Engine::Renderer::Settings::clear(false, clearDepth, false); //clear depth only
            m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::Deferred2DAPI].get<ShaderProgram>());
            Engine::Renderer::sendUniformMatrix4("VP", m_2DProjectionMatrix);
            if (commands.size() > 0) {
                Engine::Renderer::GLEnable(GL_SCISSOR_TEST);
                for (const auto& command : commands) {
                    command.func();
                }
                Engine::Renderer::GLDisable(GL_SCISSOR_TEST);
            }
            Engine::Renderer::setDepthFunc(GL_LEQUAL); //restore back to normal
        }
    } 
    glDisablei(GL_BLEND, 0);
}
void DeferredPipeline::internal_render2DAPI(uint32_t diffuseBuffer, const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport& viewport, bool clearDepth) {
    m_GBuffer.bindFramebuffers({ diffuseBuffer }, "RGBA");
    render2DAPI(commands, mainRenderFunc, viewport, clearDepth);
}
void DeferredPipeline::render(Engine::priv::RenderModule& renderer, Viewport& viewport, bool mainRenderFunction) {
    auto& camera                   = viewport.getCamera();
    auto& scene                    = viewport.getScene();
    const auto& sceneAmbient       = scene.getAmbientColor();
    const auto& viewportDimensions = viewport.getViewportDimensions();

    internal_render_per_frame_preparation(viewport, camera);
    internal_init_frame_gbuffer(viewport, camera);

    const auto gbufferSize = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };

    if (mainRenderFunction) {
#pragma region Camera UBO
        if (m_UBOCamera && Engine::priv::OpenGLState::constants.supportsUBO()) {
            UniformBufferObjectMapper mapper{ *m_UBOCamera };
            m_CameraUBODataPtr = static_cast<UBOCameraDataStruct*>(mapper.getPtr());

            const float logDepthBufferFCoeff  = (2.0f / glm::log2(camera.getFar() + 1.0f)) * 0.5f;

            m_CameraUBODataPtr->CameraProj    = camera.getProjection();
            m_CameraUBODataPtr->CameraInvProj = camera.getProjectionInverse();
            m_CameraUBODataPtr->ScreenInfo    = glm::vec4{ gbufferSize.x, gbufferSize.y, viewportDimensions.z, viewportDimensions.w };
            m_CameraUBODataPtr->ScreenInfo1   = glm::vec4{ viewportDimensions.x, viewportDimensions.y, glm::tan((camera.getAngle())) * 0.5f, camera.getAspectRatio() };
            m_CameraUBODataPtr->RendererInfo1 = glm::vec4{ renderer.m_GI_Pack, renderer.m_Gamma, 0.0f, 0.0f };
            m_CameraUBODataPtr->RendererInfo2 = glm::vec4{ sceneAmbient.r, sceneAmbient.g, sceneAmbient.b, 0.0f };
            //TODO: change the manual camera uniform sending (for when glsl version < 140) to give a choice between the two render spaces
            //same simulation and render space
            /*
            m_CameraUBODataPtr->CameraView        = camera.getView();
            m_CameraUBODataPtr->CameraViewProj    = camera.getViewProjection();
            m_CameraUBODataPtr->CameraInvView     = camera.getViewInverse();
            m_CameraUBODataPtr->CameraInvViewProj = camera.getViewProjectionInverse();
            m_CameraUBODataPtr->CameraInfo1       = glm::vec4{ camera.getPosition(), camera.getNear() };
            m_CameraUBODataPtr->CameraInfo2       = glm::vec4{ camera.getViewVector(), camera.getFar() };
            m_CameraUBODataPtr->CameraInfo3       = glm::vec4{ 0.0f, 0.0f, 0.0f, logDepthBufferFCoeff };
            */

            //this render space places the camera at the origin and offsets submitted model matrices to the vertex shaders by the camera's real simulation position
            //this helps to deal with shading inaccuracies for when the camera is very far away from the origin 
            glm::mat4 viewNoTranslation = camera.getView();
            Engine::Math::setMatrixPosition(viewNoTranslation, 0.0001f, 0.0001f, 0.0001f);
            m_CameraUBODataPtr->CameraView        = viewNoTranslation;
            m_CameraUBODataPtr->CameraViewProj    = m_CameraUBODataPtr->CameraProj * viewNoTranslation;
            m_CameraUBODataPtr->CameraInvView     = glm::inverse(m_CameraUBODataPtr->CameraView);
            m_CameraUBODataPtr->CameraInvViewProj = glm::inverse(m_CameraUBODataPtr->CameraViewProj);
            m_CameraUBODataPtr->CameraInfo1       = glm::vec4{ viewNoTranslation[3][0], viewNoTranslation[3][1], viewNoTranslation[3][2], camera.getNear() };
            m_CameraUBODataPtr->CameraInfo2       = glm::vec4{ glm::vec3{viewNoTranslation[0][2], viewNoTranslation[1][2], viewNoTranslation[2][2]}, camera.getFar() };
            m_CameraUBODataPtr->CameraInfo3       = glm::vec4{ camera.getPosition(), logDepthBufferFCoeff };
        }
#pragma endregion
    }
    internal_pass_shadows_depth(viewport, scene, camera);

    bool depthPrepass = false;
    //depthPrepass      = internal_pass_depth_prepass(viewport, camera);

    internal_pass_geometry(viewport, camera);

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_ssao(viewport, camera);

    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_stencil(viewport);

    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    //this needs to be cleaned up
    m_GBuffer.bindFramebuffers({ GBufferType::Lighting }, "RGB");
    Engine::Renderer::Settings::clear(true, false, false);
    if (m_Renderer.m_Lighting) {
        if (renderer.m_LightingAlgorithm == LightingAlgorithm::PBR) {
            internal_pass_lighting(viewport, camera, mainRenderFunction);
        } else {
            internal_pass_lighting_basic(viewport, camera, mainRenderFunction);
        }
    }
    Engine::Renderer::GLDisablei(GL_BLEND, 0);
    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
        
    glDepthRange(0.0f, 0.98f);

    internal_render2DAPI(GBufferType::Diffuse, m_Background2DAPICommands, mainRenderFunction, viewport, false);
    glDepthRange(0.0f, 1.0f);
    internal_pass_forward(viewport, camera, depthPrepass);

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    internal_pass_god_rays(viewport, camera);
    internal_pass_normaless_diffuse(viewport);
    internal_pass_hdr(viewport, camera, GBufferType::Normal, GBufferType::Misc);//out textures
    internal_pass_bloom(viewport, GBufferType::Normal);//in texture
    GBufferType::Type sceneTexture = GBufferType::Misc;
    GBufferType::Type outTexture = GBufferType::Lighting;

    internal_pass_depth_of_field(viewport, sceneTexture, outTexture);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    internal_pass_aa(mainRenderFunction, viewport, camera, sceneTexture, outTexture);

    renderPhysicsAPI(mainRenderFunction, viewport, camera, scene);

    m_GBuffer.bindBackbuffer(viewport);

    internal_render2DAPI(GBufferType::BackBuffer, m_2DAPICommands, mainRenderFunction, viewport, true);
}
void DeferredPipeline::render2DAPI(Engine::priv::RenderModule& renderer, Viewport& viewport, bool mainRenderFunction) {
    auto& camera             = viewport.getCamera();
    auto& scene              = viewport.getScene();
    const auto& sceneAmbient = scene.getAmbientColor();
    auto& viewportDimensions = viewport.getViewportDimensions();
    auto winSize             = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };

    m_GBuffer.resize(uint32_t(viewportDimensions.z), uint32_t(viewportDimensions.w));

    internal_render_per_frame_preparation(viewport, camera);


    m_GBuffer.bindBackbuffer(viewport);
    internal_render2DAPI(GBufferType::BackBuffer, m_2DAPICommands, mainRenderFunction, viewport, true);
}
void DeferredPipeline::internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>& commands, Handle textureHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    commands.emplace_back([textureHandle, p, c, a, s, d, align, scissor, this]() { DeferredPipeline::render2DTexture(textureHandle, p, c, a, s, d, align, scissor); }, d);
}
void DeferredPipeline::internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>& commands, uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    commands.emplace_back([textureAddress, textureWidth, textureHeight, p, c, a, s, d, align, scissor, this]() { DeferredPipeline::render2DTexture(textureAddress, textureWidth, textureHeight, p, c, a, s, d, align, scissor); }, d);
}
void DeferredPipeline::internal_renderText(std::vector<IRenderingPipeline::API2DCommand>& commands, const std::string& t, Handle fontHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    if (!t.empty()) {
        commands.emplace_back([t, fontHandle, p, c, a, s, d, align, scissor, this]() { DeferredPipeline::render2DText(t, fontHandle, p, c, a, s, d, align, scissor); }, d);
    }
}
void DeferredPipeline::internal_renderBorder(std::vector<IRenderingPipeline::API2DCommand>& commands, float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    const float doubleBorder = borderSize * 2.0f;
    const float halfWidth    = w / 2.0f;
    const float halfHeight   = h / 2.0f;
    float translationX       = pos.x;
    float translationY       = pos.y;
    Engine::Renderer::alignmentOffset(align, translationX, translationY, w, h);
    const glm::vec2 newPos{ translationX, translationY };

    internal_renderRectangle(commands, newPos - glm::vec2{ halfWidth, 0.0f }, col, borderSize, h + doubleBorder, angle, depth, Alignment::Right, scissor);
    internal_renderRectangle(commands, newPos + glm::vec2{ halfWidth, 0.0f }, col, borderSize, h + doubleBorder, angle, depth, Alignment::Left, scissor);
    internal_renderRectangle(commands, newPos - glm::vec2{ 0.0f, halfHeight }, col, w, borderSize, angle, depth, Alignment::TopCenter, scissor);
    internal_renderRectangle(commands, newPos + glm::vec2{ 0.0f, halfHeight + borderSize }, col, w, borderSize, angle, depth, Alignment::BottomCenter, scissor);
}
void DeferredPipeline::internal_renderRectangle(std::vector<IRenderingPipeline::API2DCommand>& commands, const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    commands.emplace_back([=]() { DeferredPipeline::render2DTexture(Handle{}, pos, col, angle, glm::vec2(width, height), depth, align, scissor); }, depth);
}
void DeferredPipeline::internal_renderTriangle(std::vector<IRenderingPipeline::API2DCommand>& commands, const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    commands.emplace_back([=]() { DeferredPipeline::render2DTriangle(position, color, angle, width, height, depth, align, scissor); }, depth);
}
void DeferredPipeline::renderTexture(Handle textureHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    internal_renderTexture(m_2DAPICommands, textureHandle, p, c, a, s, d, align, scissor);
}
void DeferredPipeline::renderTexture(uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    internal_renderTexture(m_2DAPICommands, textureAddress, textureWidth, textureHeight, p, c, a, s, d, align, scissor);
}
void DeferredPipeline::renderText(const std::string& t, Handle fontHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    internal_renderText(m_2DAPICommands, t, fontHandle, p, c, a, s, d, align, scissor);
}
void DeferredPipeline::renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderBorder(m_2DAPICommands, borderSize, pos, col, w, h, angle, depth, align, scissor);
}
void DeferredPipeline::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderRectangle(m_2DAPICommands, pos, col, width, height, angle, depth, align, scissor);
}
void DeferredPipeline::renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderTriangle(m_2DAPICommands, position, color, angle, width, height, depth, align, scissor);
}

void DeferredPipeline::renderBackgroundTexture(Handle textureHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    internal_renderTexture(m_Background2DAPICommands, textureHandle, p, c, a, s, d, align, scissor);
}
void DeferredPipeline::renderBackgroundText(const std::string& t, Handle fontHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    internal_renderText(m_Background2DAPICommands, t, fontHandle, p, c, a, s, d, align, scissor);
}
void DeferredPipeline::renderBackgroundBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderBorder(m_Background2DAPICommands, borderSize, pos, col, w, h, angle, depth, align, scissor);
}
void DeferredPipeline::renderBackgroundRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderRectangle(m_Background2DAPICommands, pos, col, width, height, angle, depth, align, scissor);
}
void DeferredPipeline::renderBackgroundTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    internal_renderTriangle(m_Background2DAPICommands, position, color, angle, width, height, depth, align, scissor);
}

void DeferredPipeline::renderFullscreenTriangle(float depth, float inNear, float inFar) {
    const auto winSize      = glm::vec2{ Engine::Resources::getWindowSize() };
    const auto gbufferSize  = glm::vec2{m_GBuffer.width(), m_GBuffer.height() };

    auto modelMatrix        = glm::mat4{ 1.0f };
    modelMatrix             = glm::translate(modelMatrix, glm::vec3{ 0.0f, 0.0f, 0.0f - depth });
    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix4Safe("VP", glm::ortho(0.0f, winSize.x, 0.0f, winSize.y, inNear, inFar));
    m_FullscreenTriangle.render();
}
void DeferredPipeline::renderFullscreenQuad(float x, float y, float width, float height, float depth, float inNear, float inFar) {
    const auto winSize = glm::vec2{ Engine::Resources::getWindowSize() };
    if (width <= 0.0f)
        width = winSize.x;
    if (height <= 0.0f)
        height = winSize.y;
    auto modelMatrix       = glm::mat4{ 1.0f };
    modelMatrix            = glm::translate(modelMatrix, glm::vec3{ x, y, 0.0f - depth });
    modelMatrix            = glm::scale(modelMatrix, glm::vec3{ width / winSize.x, height / winSize.y, 1.0f });
    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix4Safe("VP", glm::ortho(0.0f, winSize.x, 0.0f, winSize.y, inNear, inFar));  //todo: change to gbufferSize?
    m_FullscreenQuad.render();
}
void DeferredPipeline::renderFullscreenQuadCentered(float width, float height, float depth, float inNear, float inFar) {
    const auto winSize = glm::vec2{ Engine::Resources::getWindowSize() };
    const auto gbufferSize = glm::vec2{ m_GBuffer.width(), m_GBuffer.height() };
    if (width <= 0.0f)
        width = gbufferSize.x;
    if (height <= 0.0f)
        height = gbufferSize.y;
    auto modelMatrix = glm::mat4{ 1.0f };
    modelMatrix = glm::translate(modelMatrix, glm::vec3{ (gbufferSize.x / 2.0f) - (width / 2.0f), (gbufferSize.y / 2.0f) - (height / 2.0f), 0.0f - depth });
    modelMatrix = glm::scale(modelMatrix, glm::vec3{ width / gbufferSize.x, height / gbufferSize.y, 1.0f });
    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix4Safe("VP", glm::ortho(0.0f, winSize.x, 0.0f, winSize.y, inNear, inFar)); //todo: change to gbufferSize?
    m_FullscreenQuad.render();
}