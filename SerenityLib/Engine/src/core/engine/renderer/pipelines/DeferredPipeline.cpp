#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/pipelines/DeferredPipeline.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/system/Engine.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/lights/LightProbe.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>

#include <ecs/ComponentBody.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

#include <core/engine/resources/Engine_BuiltInShaders.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <SFML/Graphics/Image.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace Engine::Renderer;

//Engine::view_ptr<Engine::priv::DeferredPipeline> PIPELINE = nullptr;

constexpr std::array<glm::mat4, 6> CAPTURE_VIEWS = {
    glm::mat4(0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
    glm::mat4(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
    glm::mat4(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
};

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
    _TOTAL,
};};


DeferredPipeline::DeferredPipeline(Engine::priv::RenderModule& renderer) 
    : m_Renderer{ renderer }
{
    //PIPELINE = this;
}
DeferredPipeline::~DeferredPipeline() {
    SAFE_DELETE(UniformBufferObject::UBO_CAMERA);

    //TODO: add cleanup() from ssao / smaa here?
}

void DeferredPipeline::internal_gl_scissor_reset() noexcept {
    auto winSize          = Resources::getWindowSize();
    m_CurrentScissorState = glm::vec4(-1.0f);
    m_CurrentScissorDepth = std::numeric_limits<float>().min();
    GLCall(glScissor(0, 0, winSize.x, winSize.y));
}
void DeferredPipeline::internal_gl_scissor(const glm::vec4& scissor, float depth) noexcept {
    if (scissor == glm::vec4(-1.0f)) {
        internal_gl_scissor_reset();
    }else{
        m_CurrentScissorState = scissor;
        m_CurrentScissorDepth = depth;
        GLCall(glScissor((GLint)scissor.x, (GLint)scissor.y, (GLsizei)scissor.z, (GLsizei)scissor.w));
    }
}

void DeferredPipeline::init() {
    const auto window_size = Engine::Resources::getWindowSize();
    m_2DProjectionMatrix   = glm::ortho(0.0f, (float)window_size.x, 0.0f, (float)window_size.y, 0.005f, 3000.0f);

    float init_border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_InternalShaders.resize(ShaderEnum::_TOTAL, Handle{});
    m_InternalShaderPrograms.resize(ShaderProgramEnum::_TOTAL, Handle{});

    GLCall(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS));
    GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, init_border_color));

    m_OpenGLExtensionsManager.INIT();


    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
    m_OpenGLStateMachine.GL_glDisable(GL_STENCIL_TEST);
    m_OpenGLStateMachine.GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for non Power of Two textures
    if (Engine::priv::RenderModule::OPENGL_VERSION >= 40) {
        m_OpenGLStateMachine.GL_glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //can run real slowly on some gpu's / drivers. its core 32 but just to be safe, set it to 40
    }
    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_CLAMP);
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    GLCall(glDepthRange(0.0f, 1.0f));


    priv::EShaders::init(Engine::priv::RenderModule::OPENGL_VERSION, Engine::priv::RenderModule::GLSL_VERSION);


    UniformBufferObject::UBO_CAMERA = NEW UniformBufferObject("Camera", sizeof(UBOCameraDataStruct));
    UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraDataStruct);


    m_FullscreenQuad.init();
    m_FullscreenTriangle.init();


    FXAA::STATIC_FXAA.init_shaders();
    SSAO::STATIC_SSAO.init_shaders();
    HDR::STATIC_HDR.init_shaders();
    DepthOfField::STATIC_DOF.init_shaders();
    Bloom::bloom.init_shaders();
    GodRays::STATIC_GOD_RAYS.init_shaders();
    SMAA::STATIC_SMAA.init_shaders();

    auto emplaceShader = [](uint32_t index, const std::string& str, std::vector<Handle>& collection, ShaderType type) {
        collection[index] = Engine::Resources::addResource<Shader>(str, type, false);
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


    sf::Image sfImageWhite;
    sfImageWhite.create(2, 2, sf::Color::White);
    sf::Image sfImageBlack;
    sfImageBlack.create(2, 2, sf::Color::Black);

    sf::Image sfImageCheckers;
    sfImageCheckers.create(8, 8, sf::Color::White);

    uint32_t count = 0;
    for (uint32_t i = 0; i < sfImageCheckers.getSize().x; ++i) {
        for (uint32_t j = 0; j < sfImageCheckers.getSize().y; ++j) {
            if ((count % 2 == 0 && i % 2 == 0) || (count % 2 != 0 && i % 2 == 1)) {
                sfImageCheckers.setPixel(i, j, sf::Color::Red);
            }
            ++count;
        }
    }

    Texture::White = Engine::Resources::addResource<Texture>(sfImageWhite, "WhiteTexturePlaceholder", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);

    Texture::Black = Engine::Resources::addResource<Texture>(sfImageBlack, "BlackTexturePlaceholder", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);
    Texture::Checkers = Engine::Resources::addResource<Texture>(sfImageCheckers, "CheckersTexturePlaceholder", false, ImageInternalFormat::RGBA8, TextureType::Texture2D);
    Texture::Checkers.get<Texture>()->setFilter(TextureFilter::Nearest);
    Material::Checkers = Engine::Resources::addResource<Material>("MaterialDefaultCheckers", Texture::Checkers);
    Material::Checkers.get<Material>()->setSpecularModel(SpecularModel::None);
    Material::Checkers.get<Material>()->setSmoothness(0_uc);

    Material::WhiteShadeless = Engine::Resources::addResource<Material>("MaterialDefaultWhiteShadeless", Texture::White);
    Material::WhiteShadeless.get<Material>()->setSpecularModel(SpecularModel::None);
    Material::WhiteShadeless.get<Material>()->setSmoothness(0_uc);
    Material::WhiteShadeless.get<Material>()->setShadeless(true);

    Texture::BRDF = Engine::Resources::addResource<Texture>(512, 512, ImagePixelType::FLOAT, ImagePixelFormat::RG, ImageInternalFormat::RG16F, 1.0f);
    Texture::BRDF.get<Texture>()->setWrapping(TextureWrap::ClampToEdge);

    SSAO::STATIC_SSAO.init();
    SMAA::STATIC_SMAA.init();

    internal_generate_brdf_lut(m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance], 512, 256);



    //particle instancing
    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().bind();
    GLCall(glGenBuffers(1, &m_Particle_Instance_VBO));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Particle_Instance_VBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW));

    auto sizeofOne = sizeof(ParticleSystem::ParticleFloatType) * 4;
    auto sizeofTwo = sizeof(ParticleSystem::ParticleFloatType) * 2;

#if defined(ENGINE_PARTICLES_HALF_SIZE)
    GLCall(glEnableVertexAttribArray(2));
    GLCall(glVertexAttribPointer(2, 4, GL_HALF_FLOAT,        GL_FALSE, sizeof(ParticleSystem::ParticleDOD),  (void*)0  ));
    GLCall(glEnableVertexAttribArray(3));
    GLCall(glVertexAttribPointer(3, 2, GL_HALF_FLOAT,        GL_FALSE, sizeof(ParticleSystem::ParticleDOD),  (void*)sizeofOne));
    GLCall(glEnableVertexAttribArray(4));
    GLCall(glVertexAttribIPointer(4, 2, GL_UNSIGNED_SHORT, sizeof(ParticleSystem::ParticleDOD), (void*)(sizeofOne + sizeofTwo)));
#else
    GLCall(glEnableVertexAttribArray(2));
    GLCall(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleSystem::ParticleDOD), (void*)0));
    GLCall(glEnableVertexAttribArray(3));
    GLCall(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleSystem::ParticleDOD), (void*)sizeofOne));
    GLCall(glEnableVertexAttribArray(4));
    GLCall(glVertexAttribIPointer(4, 2, GL_UNSIGNED_INT, sizeof(ParticleSystem::ParticleDOD), (void*)(sizeofOne + sizeofTwo)));
#endif

    GLCall(glVertexAttribDivisor(2, 1));
    GLCall(glVertexAttribDivisor(3, 1));
    GLCall(glVertexAttribDivisor(4, 1));

    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().unbind();
}
void DeferredPipeline::internal_generate_pbr_data_for_texture(Handle covoludeShaderProgram, Handle prefilterShaderProgram, Texture& texture, Handle convolutionTextureHandle, Handle preEnvTextureHandle, uint32_t convoludeTextureSize, uint32_t preEnvFilterSize) {
    auto texType = texture.getTextureType();
    if (texType != TextureType::CubeMap) {
        //cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call...\n";
        return;
    }
    uint32_t size = convoludeTextureSize;
    auto& convolutionTexture = *convolutionTextureHandle.get<Texture>();
    Engine::Renderer::bindTextureForModification(texType, convolutionTexture.address());
    //Engine::Renderer::unbindFBO();
    priv::FramebufferObject fbo(size, size); //try without a depth format
    fbo.bind();

    //make these 2 variables global / constexpr in the renderer class?
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f);

    m_Renderer.bind(covoludeShaderProgram.get<ShaderProgram>());

    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType.toGLType());
    Engine::Renderer::setViewport(0.0f, 0.0f, (float)size, (float)size);
    for (uint32_t i = 0; i < 6; ++i) {
        glm::mat4 vp = captureProjection * CAPTURE_VIEWS[i];
        Engine::Renderer::sendUniformMatrix4("VP", vp);
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, convolutionTexture.address(), 0));
        Engine::Renderer::Settings::clear(true, true, false);
        Skybox::bindMesh();
    }


    //now gen EnvPrefilterMap for specular IBL
    size = preEnvFilterSize;
    auto& preEnvTexture = *preEnvTextureHandle.get<Texture>();
    Engine::Renderer::bindTextureForModification(texType, preEnvTexture.address());


    m_Renderer.bind(prefilterShaderProgram.get<ShaderProgram>());

    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType.toGLType());
    Engine::Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix", 12.56637f / float((texture.width() * texture.width()) * 6));
    Engine::Renderer::sendUniform1("NUM_SAMPLES", 32);
    uint32_t maxMipLevels = 5;
    for (uint32_t m = 0; m < maxMipLevels; ++m) {
        const uint32_t mipSize(size * (uint32_t)glm::pow(0.5, m)); // reisze framebuffer according to mip-level size.
        fbo.resize(mipSize, mipSize);
        float roughness = (float)m / (float)(maxMipLevels - 1);
        Engine::Renderer::sendUniform1("roughness", roughness);
        float a = roughness * roughness;
        Engine::Renderer::sendUniform1("a2", a * a);
        for (uint32_t i = 0; i < 6; ++i) {
            glm::mat4 vp = captureProjection * CAPTURE_VIEWS[i];
            Engine::Renderer::sendUniformMatrix4("VP", vp);
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preEnvTexture.address(), m));
            Engine::Renderer::Settings::clear(true, true, false);
            Skybox::bindMesh();
        }
    }
    fbo.unbind();
}
void DeferredPipeline::internal_generate_brdf_lut(Handle program, uint32_t brdfSize, int numSamples) {
    FramebufferObject fbo(brdfSize, brdfSize); //try without a depth format
    fbo.bind();

    Engine::Renderer::bindTextureForModification(TextureType::Texture2D, Texture::BRDF.get<Texture>()->address());
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0));
    Texture::setFilter(TextureType::Texture2D, TextureFilter::Linear);
    Texture::setWrapping(TextureType::Texture2D, TextureWrap::ClampToEdge);
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture::BRDF.get<Texture>()->address(), 0));

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
    return Engine::priv::OpenGLState::MAX_TEXTURE_UNITS;
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
    std::sort(std::execution::par_unseq, m_Background2DAPICommands.begin(),  m_Background2DAPICommands.end(),  lambda_sorter);
    std::sort(std::execution::par_unseq, m_2DAPICommands.begin(),            m_2DAPICommands.end(),            lambda_sorter);
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
    return m_OpenGLStateMachine.GL_glViewport((GLint)x, (GLint)y, (GLsizei)width, (GLsizei)height);
}
void DeferredPipeline::clear(bool color, bool depth, bool stencil) {
    if (!color && !depth && !stencil) {
        return;
    }
    GLuint clearBit = 0;
    if (color)   clearBit |= GL_COLOR_BUFFER_BIT;
    if (depth)   clearBit |= GL_DEPTH_BUFFER_BIT;
    if (stencil) clearBit |= GL_STENCIL_BUFFER_BIT;
    GLCall(glClear(clearBit));
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
        GLCall(glDeleteVertexArrays(1, &vaoObject));
        vaoObject = 0;
        return true;
    }
    return false;
}
void DeferredPipeline::generateAndBindTexture(TextureType textureType, uint32_t& textureObject) {
    GLCall(glGenTextures(1, &textureObject));
    m_OpenGLStateMachine.GL_glBindTextureForModification(textureType.toGLType(), textureObject);
}
void DeferredPipeline::generateAndBindVAO(uint32_t& vaoObject) {
    GLCall(glGenVertexArrays(1, &vaoObject));
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
void DeferredPipeline::sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) {
    m_OpenGLStateMachine.GL_glActiveTexture(unit);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    Engine::Renderer::sendUniform1Safe(location, unit);
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
    if (m_RendererState.current_bound_material != material) {
        m_RendererState.current_bound_material = material;
        return true;
    }
    return false;
}
bool DeferredPipeline::unbind(Material* material) {
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
    m_RendererState.current_bound_mesh = nullptr;
    return true;
}
void DeferredPipeline::generatePBRData(Texture& texture, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeSize, uint32_t prefilterSize) {
    internal_generate_pbr_data_for_texture(
        m_InternalShaderPrograms[ShaderProgramEnum::CubemapConvolude], 
        m_InternalShaderPrograms[ShaderProgramEnum::CubemapPrefilterEnv], 
        texture, 
        convolutionTexture,
        preEnvTexture,
        convoludeSize, 
        prefilterSize
    );
}
void DeferredPipeline::onFullscreen() {
    //TODO: move these lines to a more generic area, all rendering pipelines will pretty much do this
    restoreCurrentOpenGLState();

    Engine::Renderer::GLEnable(GL_CULL_FACE);
    Engine::Renderer::GLEnable(GL_DEPTH_CLAMP);

    auto winSize = Resources::getWindowSize();
    m_GBuffer.init(winSize.x, winSize.y);
}
void DeferredPipeline::onResize(uint32_t newWidth, uint32_t newHeight) {
    float floatWidth     = (float)newWidth;
    float floatHeight    = (float)newHeight;
    m_2DProjectionMatrix = glm::ortho(0.0f, floatWidth, 0.0f, floatHeight, 0.003f, 6000.0f);

    m_FullscreenQuad.changeDimensions(floatWidth, floatHeight);
    m_FullscreenTriangle.changeDimensions(floatWidth, floatHeight);

    m_GBuffer.resize(newWidth, newHeight);
}
void DeferredPipeline::onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight, uint32_t glslVersion, uint32_t openglVersion) {
    //TODO: move to a more generic area
    Engine::priv::RenderModule::GLSL_VERSION   = glslVersion;
    Engine::priv::RenderModule::OPENGL_VERSION = openglVersion;
    m_OpenGLStateMachine.GL_INIT_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
    Engine::Renderer::GLEnable(GL_CULL_FACE);
    m_GBuffer.init(windowWidth, windowHeight);
}
void DeferredPipeline::renderSkybox(Skybox* skybox, Handle shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) {
    glm::mat4 view_no_position = camera.getView();
    Math::removeMatrixPosition(view_no_position);

    m_Renderer.bind(shaderProgram.get<ShaderProgram>());

    if (skybox) {
        Engine::Renderer::sendUniform1("IsFake", 0);
        Engine::Renderer::sendTextureSafe("Texture", skybox->texture().get<Texture>()->address(), 0, GL_TEXTURE_CUBE_MAP);
    }else{
        Engine::Renderer::sendUniform1("IsFake", 1);
        const auto& bgColor = scene.getBackgroundColor();
        Engine::Renderer::sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    }
    Engine::Renderer::sendUniform1Safe("ScreenGamma", m_Renderer.m_Gamma);
    Engine::Renderer::sendUniformMatrix4("VP", camera.getProjection() * view_no_position);
    Skybox::bindMesh();

    Engine::Renderer::sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
    //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...
}
void DeferredPipeline::sendGPUDataLight(Camera& camera, SunLight& sunLight, const std::string& start) {
    auto body        = sunLight.getComponent<ComponentBody>();
    auto pos         = glm::vec3(body->getPosition());
    const auto& col  = sunLight.color();
    sendUniform4Safe((start + "DataA").c_str(), sunLight.getAmbientIntensity(), sunLight.getDiffuseIntensity(), sunLight.getSpecularIntensity(), 0.0f);
    sendUniform4Safe((start + "DataC").c_str(), 0.0f, pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, (float)sunLight.type());
    sendUniform1Safe("Type", 0.0f);
}
int DeferredPipeline::sendGPUDataLight(Camera& camera, PointLight& pointLight, const std::string& start) {
    auto body       = pointLight.getComponent<ComponentBody>();
    auto pos        = glm::vec3(body->getPosition());
    auto cull       = pointLight.getCullingRadius();
    auto factor     = 1100.0f * cull;
    auto distSq     = (float)camera.getDistanceSquared(pos);
    if ((!camera.sphereIntersectTest(pos, cull)) || (distSq > factor * factor)) {
        return 0;
    }
    const auto& col = pointLight.color();
    sendUniform4Safe((start + "DataA").c_str(), pointLight.getAmbientIntensity(), pointLight.getDiffuseIntensity(), pointLight.getSpecularIntensity(), 0.0f);
    sendUniform4Safe((start + "DataB").c_str(), 0.0f, 0.0f, pointLight.getConstant(), pointLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), pointLight.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, (float)pointLight.type());
    sendUniform4Safe((start + "DataE").c_str(), 0.0f, 0.0f, (float)pointLight.getAttenuationModel(), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    if (distSq <= (cull * cull)) { //inside the light volume
        return 1;
    }
    return 2;
}
void DeferredPipeline::sendGPUDataLight(Camera& camera, DirectionalLight& directionalLight, const std::string& start) {
    auto body       = directionalLight.getComponent<ComponentBody>();
    auto forward    = glm::vec3(body->forward());
    const auto& col = directionalLight.color();
    sendUniform4Safe((start + "DataA").c_str(), directionalLight.getAmbientIntensity(), directionalLight.getDiffuseIntensity(), directionalLight.getSpecularIntensity(), forward.x);
    sendUniform4Safe((start + "DataB").c_str(), forward.y, forward.z, 0.0f, 0.0f);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, (float)directionalLight.type());
    sendUniform1Safe("Type", 0.0f);
}
int DeferredPipeline::sendGPUDataLight(Camera& camera, SpotLight& spotLight, const std::string& start) {
    auto body    = spotLight.getComponent<ComponentBody>();
    auto pos     = glm::vec3(body->getPosition());
    auto forward = glm::vec3(body->forward());
    auto cull    = spotLight.getCullingRadius();
    auto factor  = 1100.0f * cull;
    auto distSq  = (float)camera.getDistanceSquared(pos);
    if (!camera.sphereIntersectTest(pos, cull)) {
        return 0;
    }
    if (distSq > factor * factor) {
        return 0;
    }
    const auto& col = spotLight.color();
    sendUniform4Safe((start + "DataA").c_str(), spotLight.getAmbientIntensity(), spotLight.getDiffuseIntensity(), spotLight.getSpecularIntensity(), forward.x);
    sendUniform4Safe((start + "DataB").c_str(), forward.y, forward.z, spotLight.getConstant(), spotLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), spotLight.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, (float)spotLight.type());
    sendUniform4Safe((start + "DataE").c_str(), spotLight.getCutoff(), spotLight.getCutoffOuter(), (float)spotLight.getAttenuationModel(), 0.0f);
    sendUniform2Safe("VertexShaderData", spotLight.getCutoffOuter(), cull);
    sendUniform1Safe("Type", 2.0f);

    if (distSq <= (cull * cull)) { //inside the light volume
        return 1;
    }
    return 2;
}
int DeferredPipeline::sendGPUDataLight(Camera& camera, RodLight& rodLight, const std::string& start) {
    auto body            = rodLight.getComponent<ComponentBody>();
    auto pos             = glm::vec3(body->getPosition());
    auto cullingDistance = rodLight.rodLength() + (rodLight.getCullingRadius() * 2.0f);
    auto factor          = 1100.0f * cullingDistance;
    auto distSq          = (float)camera.getDistanceSquared(pos);
    if (!camera.sphereIntersectTest(pos, cullingDistance) || (distSq > factor * factor)) {
        return 0;
    }
    const auto& col      = rodLight.color();
    float half           = rodLight.rodLength() / 2.0f;
    auto firstEndPt      = pos + (glm::vec3(body->forward()) * half);
    auto secndEndPt      = pos - (glm::vec3(body->forward()) * half);
    sendUniform4Safe((start + "DataA").c_str(), rodLight.getAmbientIntensity(), rodLight.getDiffuseIntensity(), rodLight.getSpecularIntensity(), firstEndPt.x);
    sendUniform4Safe((start + "DataB").c_str(), firstEndPt.y, firstEndPt.z, rodLight.getConstant(), rodLight.getLinear());
    sendUniform4Safe((start + "DataC").c_str(), rodLight.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
    sendUniform4Safe((start + "DataD").c_str(), col.x, col.y, col.z, (float)rodLight.type());
    sendUniform4Safe((start + "DataE").c_str(), rodLight.rodLength(), 0.0f, (float)rodLight.getAttenuationModel(), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    if (distSq <= (cullingDistance * cullingDistance)) {
        return 1;
    }
    return 2;
}
int DeferredPipeline::sendGPUDataLight(Camera& camera, ProjectionLight& rodLight, const std::string& start) {
    return 2;
}
void DeferredPipeline::renderDirectionalLight(Camera& camera, DirectionalLight& directionalLight, Viewport& viewport) {
    if (!directionalLight.isActive()) {
        return;
    }
    std::string start = "light.";
    sendGPUDataLight(camera, directionalLight, start);
    renderFullscreenQuad();
}
void DeferredPipeline::renderSunLight(Camera& camera, SunLight& sunLight, Viewport& viewport) {
    if (!sunLight.isActive()) {
        return;
    }
    std::string start = "light.";
    sendGPUDataLight(camera, sunLight, start);
    renderFullscreenQuad();
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
    auto body        = pointLight.getComponent<ComponentBody>();
    auto modelMatrix = body->modelMatrixRendering();
    sendUniformMatrix4("Model", modelMatrix);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if(result == 1){
        cullFace(GL_FRONT);
    }else if (result == 2) {
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
    auto body        = spotLight.getComponent<ComponentBody>();
    auto modelMatrix = body->modelMatrixRendering();
    sendUniformMatrix4("Model", modelMatrix);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (result == 1) {
        cullFace(GL_FRONT);
    }else if (result == 2) {
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
    auto body        = rodLight.getComponent<ComponentBody>();
    auto modelMatrix = body->modelMatrixRendering();
    sendUniformMatrix4("Model", modelMatrix);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

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
    Entity parent          = decalModelInstance.parent();
    auto body              = parent.getComponent<ComponentBody>();
    glm::mat4 parentModel  = body->modelMatrixRendering();
    auto maxTextures       = getMaxNumTextureUnits() - 1U;

    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform1Safe("Object_Color", decalModelInstance.color().toPackedInt());
    Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", decalModelInstance.godRaysColor().toPackedInt());

    glm::mat4 modelMatrix  = parentModel * decalModelInstance.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}

void DeferredPipeline::renderParticles(ParticleSystem& system, Camera& camera, Handle program) {
    const size_t particle_count = system.ParticlesDOD.size();
    if (particle_count > 0) {
        m_Renderer.bind(program.get<ShaderProgram>());
        for (auto& [id, mat] : system.MaterialToIndexReverse) {
            system.MaterialIDToIndex.try_emplace(id, (uint32_t)system.MaterialIDToIndex.size());
        }
        for (auto& pod : system.ParticlesDOD) {
            pod.MatID = system.MaterialIDToIndex.at(pod.MatID);
        }
        for (auto& [id, index] : system.MaterialIDToIndex) {
            Material* mat         = system.MaterialToIndexReverse.at(id);
            Texture& texture      = *mat->getComponent((uint32_t)MaterialComponentType::Diffuse).texture(0).get<Texture>();
            std::string location  = "DiffuseTexture" + std::to_string(index) + "";
            Engine::Renderer::sendTextureSafe(location.c_str(), texture, index);
        }
        const auto maxTextures = getMaxNumTextureUnits() - 1U;
        Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);

        GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Particle_Instance_VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, particle_count * sizeof(ParticleSystem::ParticleDOD), system.ParticlesDOD.data(), GL_STREAM_DRAW));

        auto& particleMesh = *Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>();
        m_Renderer.bind(&particleMesh);
        GLCall(glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)particleMesh.getVertexData().m_Indices.size(), GL_UNSIGNED_INT, 0, (GLsizei)particle_count));
        m_Renderer.unbind(&particleMesh);
    }
}
void DeferredPipeline::renderMesh(Mesh& mesh, uint32_t mode) {
    const auto indicesSize = mesh.getVertexData().m_Indices.size();
    if (indicesSize == 0) {
        return;
    }
    GLCall(glDrawElements(mode, (GLsizei)indicesSize, GL_UNSIGNED_INT, nullptr));
}
void DeferredPipeline::renderLightProbe(LightProbe& lightProbe) {
    //goal: render all 6 sides into a fbo and into a cubemap, and have that cubemap stored in the light probe to be used for Global Illumination
}
void DeferredPipeline::internal_render_2d_text_left(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    uint32_t i = 0;
    for (const auto character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight;
            x  = 0.0f;
        }else if (character != '\0') {
            uint32_t accum     = i * 4;
            const CharGlyph& glyph = font.getGlyphData(character);
            float startingY        = y - (glyph.height + glyph.yoffset);
            ++i;

            m_Text_Indices.push(accum + 0);
            m_Text_Indices.push(accum + 1);
            m_Text_Indices.push(accum + 2);
            m_Text_Indices.push(accum + 3);
            m_Text_Indices.push(accum + 1);
            m_Text_Indices.push(accum + 0);

            float startingX = x + glyph.xoffset;
            x += glyph.xadvance;

            for (uint32_t i = 0; i < 4; ++i) {
                m_Text_Points.emplace_push(startingX + glyph.pts[i].x, startingY + glyph.pts[i].y, z);
            }
            for (uint32_t i = 0; i < 4; ++i) {
                m_Text_UVs.emplace_push(glyph.uvs[i].x, glyph.uvs[i].y);
            }
        }
    }
}
void DeferredPipeline::internal_render_2d_text_center(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    std::vector<std::string> lines;
    std::vector<uint16_t> lines_sizes;
    std::string line_accumulator = "";
    for (const auto character : text) {
        if (character == '\n') {
            lines.emplace_back(line_accumulator);
            lines_sizes.emplace_back((uint16_t)x);
            line_accumulator.clear();
            x = 0.0f;
            continue;
        }else if (character != '\0') {
            const CharGlyph& chr = font.getGlyphData(character);
            line_accumulator += character;
            x += chr.xadvance;
        }
    }
    if (!line_accumulator.empty()) {
        lines.emplace_back(line_accumulator);
        lines_sizes.emplace_back((uint16_t)x);
    }

    x = 0.0f;
    uint32_t i = 0;
    for (size_t l = 0; l < lines.size(); ++l) {
        const auto& line = lines[l];
        const auto& line_size = lines_sizes[l] / 2;
        for (auto& character : line) {
            if (character != '\0') {
                uint32_t accum     = i * 4;
                const CharGlyph& glyph = font.getGlyphData(character);
                float startingY        = y - (glyph.height + glyph.yoffset);
                ++i;

                m_Text_Indices.push(accum + 0);
                m_Text_Indices.push(accum + 1);
                m_Text_Indices.push(accum + 2);
                m_Text_Indices.push(accum + 3);
                m_Text_Indices.push(accum + 1);
                m_Text_Indices.push(accum + 0);

                float startingX = x + glyph.xoffset;
                x += glyph.xadvance;

                for (uint32_t i = 0; i < 4; ++i) {
                    m_Text_Points.emplace_push(startingX + glyph.pts[i].x - line_size, startingY + glyph.pts[i].y, z);
                }
                for (uint32_t i = 0; i < 4; ++i) {
                    m_Text_UVs.emplace_push(glyph.uvs[i].x, glyph.uvs[i].y);
                }
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}
void DeferredPipeline::internal_render_2d_text_right(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    std::vector<std::string> lines;
    std::string line_accumulator = "";
    for (const auto character : text) {
        if (character == '\n') {
            lines.emplace_back(line_accumulator);
            line_accumulator.clear();
            continue;
        }else if (character != '\0') {
            line_accumulator += character;
        }
    }
    if (lines.size() == 0) {
        lines.emplace_back(line_accumulator);
    }

    uint32_t i = 0;
    for (auto& line : lines) {
        int line_size = (int)line.size();
        int k = 0;
        for (int j = line_size; j >= 0; --j) {
            auto character = line[j];
            if (character != '\0') {
                uint32_t accum = i * 4;
                ++i;
                const CharGlyph& glyph = font.getGlyphData(character);
                float startingY  = y - (glyph.height + glyph.yoffset);

                m_Text_Indices.push(accum + 0);
                m_Text_Indices.push(accum + 1);
                m_Text_Indices.push(accum + 2);
                m_Text_Indices.push(accum + 3);
                m_Text_Indices.push(accum + 1);
                m_Text_Indices.push(accum + 0);

                if (k == 0) {
                    x -= glyph.width;
                }
                float startingX = x + glyph.xoffset;
                x -= glyph.xadvance;

                for (uint32_t i = 0; i < 4; ++i) {
                    m_Text_Points.emplace_push(startingX + glyph.pts[i].x, startingY + glyph.pts[i].y, z);
                }
                for (uint32_t i = 0; i < 4; ++i) {
                    m_Text_UVs.emplace_push(glyph.uvs[i].x, glyph.uvs[i].y);
                }
                ++k;
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}
void DeferredPipeline::render2DText(const std::string& text, Handle fontHandle, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, TextAlignment textAlignment, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    Mesh& fontPlane = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh().get<Mesh>();
    m_Renderer.bind(&fontPlane);

    Font& font = *fontHandle.get<Font>();
    auto  newLineGlyphHeight = font.getMaxHeight() + font.getLineHeight();
    Handle textureHandle = font.getGlyphTexture();
    float y = 0.0f;
    float x = 0.0f;
    float z = -0.001f - depth;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1));

    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    Engine::Renderer::sendTexture("DiffuseTexture", *textureHandle.get<Texture>(), 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    if (textAlignment == TextAlignment::Left) {
        internal_render_2d_text_left(text, font, -newLineGlyphHeight, x, y, z);
    }else if (textAlignment == TextAlignment::Right) {
        internal_render_2d_text_right(text, font, -newLineGlyphHeight, x, y, z);
    }else if (textAlignment == TextAlignment::Center) {
        internal_render_2d_text_center(text, font, -newLineGlyphHeight, x, y, z);
    }
    fontPlane.modifyVertices(0, m_Text_Points.data(), m_Text_Points.size(), MeshModifyFlags::None); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(1, m_Text_UVs.data(), m_Text_UVs.size(), MeshModifyFlags::UploadToGPU);
    fontPlane.modifyIndices(m_Text_Indices.data(), m_Text_Indices.size(), MeshModifyFlags::UploadToGPU);
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

    if (texture) {
        totalSizeX *= texture->width();
        totalSizeY *= texture->height();
        Engine::Renderer::sendTexture("DiffuseTexture", *texture, 0);
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    }else{
        Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
    }
    Engine::Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(totalSizeX, totalSizeY, 1.0f));

    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Mesh& plane = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().get<Mesh>();
    m_Renderer.bind(&plane);
    renderMesh(plane);
    m_Renderer.unbind(&plane);
}
void DeferredPipeline::render2DTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment alignment, const glm::vec4& scissor) {
    internal_gl_scissor(scissor, depth);

    float translationX = position.x;
    float translationY = position.y;

    Engine::Renderer::alignmentOffset(alignment, translationX, translationY, width, height);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(width, height, 1));

    Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    Mesh& triangle = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh().get<Mesh>();
    m_Renderer.bind(&triangle);
    renderMesh(triangle);
    m_Renderer.unbind(&triangle);
}
void DeferredPipeline::internal_render_per_frame_preparation(Viewport& viewport, Camera& camera) {
    const auto& winSize    = Resources::getWindowSize();
    const auto& dimensions = glm::vec4(viewport.getViewportDimensions());
    if (viewport.isAspectRatioSynced()) {
        camera.setAspect(dimensions.z / dimensions.w);
    }
    //Engine::Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w); //m_GBuffer.bindFramebuffers() already does this
    GLCall(glScissor(0, 0, winSize.x, winSize.y));

    m_2DProjectionMatrix = glm::ortho(0.0f, dimensions.z, 0.0f, dimensions.w, 0.005f, 3000.0f); //might have to recheck this
    //this is god awful and ugly, but it's needed. find a way to refactor this properly
    for (uint32_t i = 0; i < 12; ++i) {
        GLCall(glActiveTexture(GL_TEXTURE0 + i));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    }
    GLCall(glDepthMask(GL_TRUE));
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure

    //m_GBuffer.bindFramebuffers(GBufferType::Bloom, GBufferType::GodRays, "RGBA", false);
    //Engine::Renderer::Settings::clear(true, true, true); // clear all
    m_GBuffer.bindFramebuffers("", true);
    Engine::Renderer::Settings::clear(false, true, true); // clear depth & stencil only
}
bool DeferredPipeline::internal_pass_depth_prepass(Viewport& viewport, Camera& camera) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::ZPrepass].get<ShaderProgram>());

    Scene& scene = viewport.getScene();

    InternalScenePublicInterface::RenderGeometryOpaque(m_Renderer, scene, viewport, camera);
    if (viewport.getRenderFlags().has(ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    //InternalScenePublicInterface::RenderGeometryTransparent(m_Renderer, scene, viewport, camera);
    //InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(m_Renderer, scene, viewport, camera, true);

    InternalScenePublicInterface::RenderForwardOpaque(m_Renderer, scene, viewport, camera);
    //InternalScenePublicInterface::RenderForwardTransparent(m_Renderer, scene, viewport, camera);
    //InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(m_Renderer, scene, viewport, camera);

    GLCall(glDepthMask(GL_FALSE));
    return true;
}
void DeferredPipeline::internal_pass_geometry(Viewport& viewport, Camera& camera) {
    Scene& scene           = viewport.getScene();
    const glm::vec4& clear = viewport.getBackgroundColor();

    m_GBuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

    Engine::Renderer::Settings::clear(true, false, false); // clear color only
    
    //glClearBufferfv(GL_COLOR, 0, std::vector<float>{ clear.r, clear.g, clear.b/*, clear.a*/ }.data());
    //auto& godRays = GodRays::STATIC_GOD_RAYS;
    //if (godRays.godRays_active) {
        //r = outglow, g = outspecular, b = godRaysRG, a = godRaysB. disabing for now with clear color of zero
    //    glClearBufferfv(GL_COLOR, 2, std::vector<float>{ 0.0f, 0.0f, 0.0f, 0.0f }.data());
    //}

    InternalScenePublicInterface::RenderGeometryOpaque(m_Renderer, scene, viewport, camera);
    if (viewport.getRenderFlags().has(ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    InternalScenePublicInterface::RenderGeometryTransparent(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(m_Renderer, scene, viewport, camera, true);
}
void DeferredPipeline::internal_pass_forward(Viewport& viewport, Camera& camera, bool depthPrepass) {
    Scene& scene = viewport.getScene();
    m_GBuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, GBufferType::Lighting, "RGBA");

    InternalScenePublicInterface::RenderForwardOpaque(m_Renderer, scene, viewport, camera);

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    for (uint32_t i = 0; i < 4; ++i) {
        Engine::Renderer::GLEnablei(GL_BLEND, i);
    }
    if (!depthPrepass) {
        GLCall(glDepthMask(GL_TRUE));
    }
    InternalScenePublicInterface::RenderForwardTransparent(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(m_Renderer, scene, viewport, camera);
    if (!depthPrepass) {
        GLCall(glDepthMask(GL_FALSE));
    }
    InternalScenePublicInterface::RenderDecals(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderForwardParticles(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderParticles(m_Renderer, scene, viewport, camera, m_InternalShaderPrograms[ShaderProgramEnum::Particle]);

    for (uint32_t i = 0; i < 4; ++i) {
        Engine::Renderer::GLDisablei(GL_BLEND, i); //this is needed for smaa at least
    }
}
void DeferredPipeline::internal_pass_ssao(Viewport& viewport, Camera& camera) {

    auto framebuffer1 = GBufferType::Bloom;
    auto framebuffer2 = GBufferType::GodRays;

    //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes
    m_GBuffer.bindFramebuffers(framebuffer1, framebuffer2, "A", false);
    Engine::Renderer::Settings::clear(true, false, false); //bloom and god rays alpha channels cleared to black 
    if (SSAO::STATIC_SSAO.m_SSAOLevel > SSAOLevel::Off && viewport.getRenderFlags().has(ViewportRenderingFlag::SSAO)) {
        Engine::Renderer::GLEnablei(GL_BLEND, 0);//i dont think this is needed anymore
        m_GBuffer.bindFramebuffers(framebuffer1, "A", false);
        SSAO::STATIC_SSAO.passSSAO(m_GBuffer, viewport, camera, m_Renderer);
        if (SSAO::STATIC_SSAO.m_ssao_do_blur) {
            Engine::Renderer::GLDisablei(GL_BLEND, 0); //yes this is absolutely needed
            for (uint32_t i = 0; i < SSAO::STATIC_SSAO.m_ssao_blur_num_passes; ++i) {
                m_GBuffer.bindFramebuffers(framebuffer2, "A", false);
                SSAO::STATIC_SSAO.passBlur(m_GBuffer, viewport, "H", framebuffer1, m_Renderer);
                m_GBuffer.bindFramebuffers(framebuffer1, "A", false);
                SSAO::STATIC_SSAO.passBlur(m_GBuffer, viewport, "V", framebuffer2, m_Renderer);
            }
        }  
    }
}
void DeferredPipeline::internal_pass_stencil() {
    Engine::Renderer::colorMask(false, false, false, false);

    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::StencilPass].get<ShaderProgram>());

    m_GBuffer.getMainFBO().bind();

    Engine::Renderer::GLEnable(GL_STENCIL_TEST);
    Engine::Renderer::Settings::clear(false, false, true); //stencil is completely filled with 0's
    Engine::Renderer::stencilMask(0xFFFFFFFF);

    Engine::Renderer::stencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
    //exclude shadeless normals
    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);

    Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil
    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::internal_pass_lighting(Viewport& viewport, Camera& camera, bool mainRenderFunction) {
    const Scene& scene = viewport.getScene();
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLighting].get<ShaderProgram>());
    auto winSize = glm::vec2(Engine::Resources::getWindowSize());
    if (RenderModule::GLSL_VERSION < 140) {
        Engine::Renderer::sendUniformMatrix4Safe("CameraView", camera.getView());
        Engine::Renderer::sendUniformMatrix4Safe("CameraProj", camera.getProjection());
        //sendUniformMatrix4Safe("CameraViewProj",camera.getViewProjection()); //moved to shader binding function
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
        Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
        Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
    }
    Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, (uint32_t)Material::m_MaterialProperities.size());
    Engine::Renderer::sendUniform4Safe("ScreenData", 0.0f, m_Renderer.m_Gamma, winSize.x, winSize.y);

    Engine::Renderer::sendTexture("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0);
    Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1);
    Engine::Renderer::sendTexture("gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 2);
    Engine::Renderer::sendTexture("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 3);
    Engine::Renderer::sendTexture("gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 4);

    Engine::Renderer::setDepthFunc(GL_GEQUAL);
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    
    for (const auto& light : InternalScenePublicInterface::GetPointLights(scene)) {
        renderPointLight(camera, *light);
    }
    for (const auto& light : InternalScenePublicInterface::GetSpotLights(scene)) {
        renderSpotLight(camera, *light);
    }
    for (const auto& light : InternalScenePublicInterface::GetRodLights(scene)) {
        renderRodLight(camera, *light);
    }
    for (const auto& light : InternalScenePublicInterface::GetProjectionLights(scene)) {
        renderProjectionLight(camera, *light);
    }
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    for (const auto& light : InternalScenePublicInterface::GetSunLights(scene)) {
        renderSunLight(camera, *light, viewport);
    }
    for (const auto& light : InternalScenePublicInterface::GetDirectionalLights(scene)) {
        renderDirectionalLight(camera, *light, viewport);
    }
    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(2, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(3, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(4, GL_TEXTURE_2D);


    if (mainRenderFunction) {
        //do GI here. (only doing GI during the main render pass, not during light probes
        m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGI].get<ShaderProgram>());
        if (Engine::priv::RenderModule::GLSL_VERSION < 140) {
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
            Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
            Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
        }
        Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, (uint32_t)Material::m_MaterialProperities.size());
        Engine::Renderer::sendUniform4Safe("ScreenData", m_Renderer.m_GI_Pack, m_Renderer.m_Gamma, winSize.x, winSize.y);
        Engine::Renderer::sendTexture("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0);
        Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1);
        Engine::Renderer::sendTexture("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 2);
        Engine::Renderer::sendTexture("gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 3);
        Engine::Renderer::sendTexture("gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 4);
        Skybox* skybox = scene.skybox();
        if (skybox && skybox->texture().get<Texture>()->hasGlobalIlluminationData()) {
            Engine::Renderer::sendTextureSafe("irradianceMap", skybox->texture().get<Texture>()->getConvolutionTexture().get<Texture>()->address(), 5, GL_TEXTURE_CUBE_MAP);
            Engine::Renderer::sendTextureSafe("prefilterMap", skybox->texture().get<Texture>()->getPreEnvTexture().get<Texture>()->address(), 6, GL_TEXTURE_CUBE_MAP);
        }else{
            Engine::Renderer::sendTextureSafe("irradianceMap", Texture::Black.get<Texture>()->address(), 5, GL_TEXTURE_2D);
            Engine::Renderer::sendTextureSafe("prefilterMap", Texture::Black.get<Texture>()->address(), 6, GL_TEXTURE_2D);
        }
        Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF.get<Texture>(), 7);

        Engine::Renderer::renderFullscreenQuad();

        Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(2, GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(3, GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(4, GL_TEXTURE_2D);

        if (skybox && skybox->texture().get<Texture>()->hasGlobalIlluminationData()) {
            Engine::Renderer::clearTexture(5, GL_TEXTURE_CUBE_MAP);
            Engine::Renderer::clearTexture(6, GL_TEXTURE_CUBE_MAP);
        }else{
            Engine::Renderer::clearTexture(5, GL_TEXTURE_2D);
            Engine::Renderer::clearTexture(6, GL_TEXTURE_2D);
        }
        Engine::Renderer::clearTexture(7, GL_TEXTURE_2D);
    }
}
void DeferredPipeline::internal_pass_god_rays(Viewport& viewport, Camera& camera) {
    m_GBuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
    Engine::Renderer::Settings::clear(true, false, false); //godrays rgb channels cleared to black
    auto& godRaysPlatform = GodRays::STATIC_GOD_RAYS;
    auto sun = Engine::Renderer::godRays::getSun();
    if (!sun.null() && viewport.getRenderFlags().has(ViewportRenderingFlag::GodRays) && godRaysPlatform.godRays_active) {
        auto body        = sun.getComponent<ComponentBody>();
        if (!body) {
            return;
        }
        glm::vec3 oPos   = body->getPosition();
        glm::vec3 camPos = camera.getPosition();
        glm::vec3 camVec = camera.getViewVector();
        bool infront     = Engine::Math::isPointWithinCone(camPos, -camVec, oPos, Engine::Math::toRadians(godRaysPlatform.fovDegrees));
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
            godRaysPlatform.pass(m_GBuffer, viewport, glm::vec2(sp.x, sp.y), alpha, m_Renderer);
        }
    }
}
void DeferredPipeline::internal_pass_hdr(Viewport& viewport, Camera& camera, GBufferType::Type outTexture, GBufferType::Type outTexture2) {
    HDR::STATIC_HDR.pass(m_GBuffer, viewport, outTexture, outTexture2, GodRays::STATIC_GOD_RAYS.godRays_active, GodRays::STATIC_GOD_RAYS.factor, m_Renderer);
}
void DeferredPipeline::internal_pass_bloom(Viewport& viewport, GBufferType::Type sceneTexture) {
    if (Bloom::bloom.m_Bloom_Active && viewport.getRenderFlags().has(ViewportRenderingFlag::Bloom)) {
        m_GBuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
        Bloom::bloom.pass(m_GBuffer, viewport, sceneTexture, m_Renderer);
        for (uint32_t i = 0; i < Bloom::bloom.m_Num_Passes; ++i) {
            m_GBuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
            internal_pass_blur(viewport, GBufferType::Bloom, "H");
            m_GBuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
            internal_pass_blur(viewport, GBufferType::GodRays, "V");
        }
    }
}
void DeferredPipeline::internal_pass_depth_of_field(Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (DepthOfField::STATIC_DOF.dof && viewport.getRenderFlags().has(ViewportRenderingFlag::DepthOfField)) {
        m_GBuffer.bindFramebuffers(outTexture, "RGBA");
        DepthOfField::STATIC_DOF.pass(m_GBuffer, viewport, sceneTexture, m_Renderer);
        std::swap(sceneTexture, outTexture);
    }
}
void DeferredPipeline::internal_pass_aa(bool mainRenderFunction, Viewport& viewport, Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (!mainRenderFunction || m_Renderer.m_AA_algorithm == AntiAliasingAlgorithm::None || !viewport.getRenderFlags().has(ViewportRenderingFlag::AntiAliasing)) {
        m_GBuffer.bindFramebuffers(outTexture, "RGBA");
        internal_pass_final(sceneTexture);
        m_GBuffer.bindBackbuffer(viewport);   
        internal_pass_depth_and_transparency(viewport, outTexture);
    }else{
        switch (m_Renderer.m_AA_algorithm) {
            case AntiAliasingAlgorithm::None: {
                break;
            }
            case AntiAliasingAlgorithm::FXAA: {
                if (mainRenderFunction) {
                    m_GBuffer.bindFramebuffers(outTexture, "RGBA");
                    internal_pass_final(sceneTexture);
                    m_GBuffer.bindFramebuffers(sceneTexture, "RGBA");
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
                    m_GBuffer.bindFramebuffers(outTexture, "RGBA");
                    internal_pass_final(sceneTexture);
                    std::swap(sceneTexture, outTexture);
                    const auto winSize = glm::vec2(Resources::getWindowSize());
                    const glm::vec4& SMAA_PIXEL_SIZE = glm::vec4(1.0f / winSize.x, 1.0f / winSize.y, winSize.x, winSize.y);
                    SMAA::STATIC_SMAA.passEdge(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture, outTexture, m_Renderer);
                    SMAA::STATIC_SMAA.passBlend(m_GBuffer, SMAA_PIXEL_SIZE, viewport, outTexture, m_Renderer);
                    m_GBuffer.bindFramebuffers(outTexture, "RGBA");
                    SMAA::STATIC_SMAA.passNeighbor(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture, m_Renderer);
                    //m_GBuffer.bindFramebuffers(sceneTexture);
                    //SMAA::smaa.passFinal(m_GBuffer, viewport);//unused
                    m_GBuffer.bindBackbuffer(viewport);
                    internal_pass_depth_and_transparency(viewport, outTexture);
                }
                break;
            }
        }
    }
}
void DeferredPipeline::internal_pass_final(GBufferType::Type sceneTexture) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredFinal].get<ShaderProgram>());
    Engine::Renderer::sendUniform1Safe("HasBloom", (int)Bloom::bloom.m_Bloom_Active);
    Engine::Renderer::sendUniform1Safe("HasFog", (int)Fog::STATIC_FOG.fog_active);

    if (Engine::Renderer::fog::enabled()) {
        Engine::Renderer::sendUniform1Safe("FogDistNull", Fog::STATIC_FOG.distNull);
        Engine::Renderer::sendUniform1Safe("FogDistBlend", Fog::STATIC_FOG.distBlend);
        Engine::Renderer::sendUniform4Safe("FogColor", Fog::STATIC_FOG.color);
        Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 2);
    }
    Engine::Renderer::sendTextureSafe("SceneTexture", m_GBuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("gBloomMap", m_GBuffer.getTexture(GBufferType::Bloom), 1);
    Engine::Renderer::sendTextureSafe("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 2);
    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(2, GL_TEXTURE_2D);
}
void DeferredPipeline::internal_pass_depth_and_transparency(Viewport& viewport, GBufferType::Type sceneTexture) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DepthAndTransparency].get<ShaderProgram>());
    Engine::Renderer::sendTextureSafe("SceneTexture", m_GBuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 1);

    //Engine::Renderer::GLEnable(GL_BLEND);
    Engine::Renderer::GLEnablei(GL_BLEND, 0);

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    //sendUniform4Safe("TransparencyMaskColor", viewport.getTransparencyMaskColor());
    //sendUniform1Safe("TransparencyMaskActive", (int)viewport.isTransparencyMaskActive());
    Engine::Renderer::sendUniform1Safe("DepthMaskValue", viewport.getDepthMaskValue());
    Engine::Renderer::sendUniform1Safe("DepthMaskActive", (int)viewport.isDepthMaskActive());

    Engine::Renderer::renderFullscreenQuad();

    //Engine::Renderer::GLDisable(GL_BLEND);
    Engine::Renderer::GLDisablei(GL_BLEND, 0);
}
void DeferredPipeline::internal_pass_blur(Viewport& viewport, GLuint texture, std::string_view type) {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::DeferredBlur].get<ShaderProgram>());

    glm::vec2 hv(0.0f);
    if (type == "H") { 
        hv = glm::vec2(1.0f, 0.0f); 
    }else{ 
        hv = glm::vec2(0.0f, 1.0f); 
    }

    auto& bloom = Bloom::bloom;
    Engine::Renderer::sendUniform4("strengthModifier",
        bloom.m_Blur_Strength,
        bloom.m_Blur_Strength,
        bloom.m_Blur_Strength,
        SSAO::STATIC_SSAO.m_ssao_blur_strength
    );
    Engine::Renderer::sendUniform4("DataA", bloom.m_Blur_Radius, 0.0f, hv.x, hv.y);
    Engine::Renderer::sendTexture("image", m_GBuffer.getTexture(texture), 0);

    Engine::Renderer::renderFullscreenQuad();
}
void DeferredPipeline::internal_pass_normaless_diffuse() {
    m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::NormalessDiffuse].get<ShaderProgram>());

    m_GBuffer.bindFramebuffers(GBufferType::Lighting, "RGBA");

    Engine::Renderer::sendUniform1Safe("HasLighting", (int)m_Renderer.m_Lighting);
    Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::sendTexture("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 1);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
}
void DeferredPipeline::renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) {
    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    if (mainRenderFunc && viewport.getRenderFlags().has(ViewportRenderingFlag::PhysicsDebug)) {
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
            if (m_Renderer.m_DrawPhysicsDebug && &camera == scene.getActiveCamera()) {
        #endif
                Engine::Renderer::GLDisable(GL_DEPTH_TEST);
                GLCall(glDepthMask(GL_FALSE));
                m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::BulletPhysics].get<ShaderProgram>());
                Core::m_Engine->m_PhysicsModule.render(camera);
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
            }
        #endif
    }
}
void DeferredPipeline::render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport& viewport, bool clearDepth) {
    if (commands.size() > 0) {
        Engine::Renderer::GLEnablei(GL_BLEND, 0);

        //Engine::Renderer::GLEnable(GL_DEPTH_TEST);
        //Engine::Renderer::GLDisable(GL_DEPTH_TEST);
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GLCall(glDepthMask(GL_TRUE));
        if (mainRenderFunc) {
            if (viewport.getRenderFlags().has(ViewportRenderingFlag::API2D)) {
                Engine::Renderer::Settings::clear(false, clearDepth, false); //clear depth only
                m_Renderer.bind(m_InternalShaderPrograms[ShaderProgramEnum::Deferred2DAPI].get<ShaderProgram>());
                Engine::Renderer::sendUniformMatrix4("VP", m_2DProjectionMatrix);
                Engine::Renderer::sendUniform1Safe("ScreenGamma", m_Renderer.m_Gamma);
                Engine::Renderer::GLEnable(GL_SCISSOR_TEST);

                for (const auto& command : commands) {
                    command.func();
                }
                Engine::Renderer::GLDisable(GL_SCISSOR_TEST);
            }
        }
        Engine::Renderer::GLDisablei(GL_BLEND, 0);

        //Engine::Renderer::GLEnable(GL_DEPTH_TEST);
        //GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        //GLCall(glDepthMask(GL_FALSE));
    }
}

void DeferredPipeline::render(Engine::priv::RenderModule& renderer, Viewport& viewport, bool mainRenderFunction) {
    auto& camera             = viewport.getCamera();
    auto& scene              = viewport.getScene();
    auto& viewportDimensions = viewport.getViewportDimensions();
    auto winSize             = glm::vec2(m_GBuffer.width(), m_GBuffer.height());

    internal_render_per_frame_preparation(viewport, camera);

    if (mainRenderFunction) {
        #pragma region Camera UBO
        if (Engine::priv::RenderModule::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA) {
            //TODO: change the manual camera uniform sending (for when glsl version < 140) to give a choice between the two render spaces

            //same simulation and render space
            //m_UBOCameraDataStruct.View        = camera.getView();
            //m_UBOCameraDataStruct.Proj        = camera.getProjection();
            //m_UBOCameraDataStruct.ViewProj    = camera.getViewProjection();
            //m_UBOCameraDataStruct.InvProj     = camera.getProjectionInverse();
            //m_UBOCameraDataStruct.InvView     = camera.getViewInverse();
            //m_UBOCameraDataStruct.InvViewProj = camera.getViewProjectionInverse();
            //m_UBOCameraDataStruct.Info1       = glm::vec4(camera.getPosition(),camera.getNear());
            //m_UBOCameraDataStruct.Info2       = glm::vec4(camera.getViewVector(),camera.getFar());
            //m_UBOCameraDataStruct.Info3       = glm::vec4(0.0f,0.0f,0.0f, 0.0f);
            //m_UBOCameraDataStruct.Info4       = glm::vec4(winSize.x, winSize.y, dimensions.z, dimensions.w);


            //this render space places the camera at the origin and offsets submitted model matrices to the vertex shaders
            //by the camera's real simulation position
            //this helps to deal with shading inaccuracies for when the camera is very far away from the origin

            m_UBOCameraDataStruct.View          = ComponentCamera_Functions::GetViewNoTranslation(camera);
            m_UBOCameraDataStruct.Proj          = camera.getProjection();
            m_UBOCameraDataStruct.ViewProj      = ComponentCamera_Functions::GetViewProjectionNoTranslation(camera);
            m_UBOCameraDataStruct.InvProj       = camera.getProjectionInverse();
            m_UBOCameraDataStruct.InvView       = ComponentCamera_Functions::GetViewInverseNoTranslation(camera);
            m_UBOCameraDataStruct.InvViewProj   = ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(camera);
            m_UBOCameraDataStruct.Info1         = glm::vec4(0.0001f, 0.0001f, 0.0001f, camera.getNear());
            m_UBOCameraDataStruct.Info2         = glm::vec4(ComponentCamera_Functions::GetViewVectorNoTranslation(camera), camera.getFar());
            m_UBOCameraDataStruct.Info3         = glm::vec4(camera.getPosition(), 0.0f);
            m_UBOCameraDataStruct.Info4         = glm::vec4(winSize.x, winSize.y, viewportDimensions.z, viewportDimensions.w);

            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraDataStruct);
        }
        #pragma endregion
    }
    bool depthPrepass = false;
    //depthPrepass      = internal_pass_depth_prepass(viewport, camera);
    
    internal_pass_geometry(viewport, camera);

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    GLCall(glDepthMask(GL_FALSE));
    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_ssao(viewport, camera);

    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_stencil();

    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    GLCall(glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE));

    //this needs to be cleaned up
    m_GBuffer.bindFramebuffers(GBufferType::Lighting, "RGB");
    Engine::Renderer::Settings::clear(true, false, false);//lighting rgb channels cleared to black

    if(m_Renderer.m_Lighting){
        internal_pass_lighting(viewport, camera, mainRenderFunction);
    }
    Engine::Renderer::GLDisablei(GL_BLEND, 0);
    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    GLCall(glDepthMask(GL_TRUE));


    m_GBuffer.bindFramebuffers(GBufferType::Diffuse, "RGBA");
    glEnablei(GL_BLEND, 0);
    GLCall(glDepthRange(0.0f, 0.98f));
    render2DAPI(m_Background2DAPICommands, mainRenderFunction, viewport, false);
    GLCall(glDepthRange(0.0f, 1.0f));
    glDisablei(GL_BLEND, 0);


    internal_pass_forward(viewport, camera, depthPrepass);

    //this is god awful and ugly, but it's needed. find a way to refactor this properly
    for (uint32_t i = 0; i < OpenGLState::MAX_TEXTURE_UNITS; ++i) {
        Engine::Renderer::clearTexture(i, GL_TEXTURE_2D);
        Engine::Renderer::clearTexture(i, GL_TEXTURE_CUBE_MAP);
    }

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    internal_pass_god_rays(viewport, camera);

    internal_pass_normaless_diffuse();

    internal_pass_hdr(viewport, camera, GBufferType::Normal, GBufferType::Misc);//out textures

    internal_pass_bloom(viewport, GBufferType::Normal);//in texture

    GBufferType::Type sceneTexture = GBufferType::Misc;
    GBufferType::Type outTexture   = GBufferType::Lighting;

    internal_pass_depth_of_field(viewport, sceneTexture, outTexture);

    GLCall(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    internal_pass_aa(mainRenderFunction, viewport, camera, sceneTexture, outTexture);
    
    renderPhysicsAPI(mainRenderFunction, viewport, camera, scene);

    render2DAPI(m_2DAPICommands, mainRenderFunction, viewport);
}




void DeferredPipeline::internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>& commands, Handle textureHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    commands.emplace_back([textureHandle, p, c, a, s, d, align, scissor, this]() { DeferredPipeline::render2DTexture(textureHandle, p, c, a, s, d, align, scissor); }, d);
}
void DeferredPipeline::internal_renderText(std::vector<IRenderingPipeline::API2DCommand>& commands, const std::string& t, Handle fontHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    commands.emplace_back([t, fontHandle, p, c, a, s, d, align, scissor, this]() { DeferredPipeline::render2DText(t, fontHandle, p, c, a, s, d, align, scissor); }, d);
}
void DeferredPipeline::internal_renderBorder(std::vector<IRenderingPipeline::API2DCommand>& commands, float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    float doubleBorder = borderSize * 2.0f;
    float halfWidth    = w / 2.0f;
    float halfHeight   = h / 2.0f;

    float translationX = pos.x;
    float translationY = pos.y;
    Engine::Renderer::alignmentOffset(align, translationX, translationY, w, h);
    glm::vec2 newPos(translationX, translationY);

    internal_renderRectangle(commands, newPos - glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth, Alignment::Right, scissor);
    internal_renderRectangle(commands, newPos + glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth, Alignment::Left, scissor);
    internal_renderRectangle(commands, newPos - glm::vec2(0.0f, halfHeight), col, w, borderSize, angle, depth, Alignment::TopCenter, scissor);
    internal_renderRectangle(commands, newPos + glm::vec2(0.0f, halfHeight + borderSize), col, w, borderSize, angle, depth, Alignment::BottomCenter, scissor);
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

void DeferredPipeline::renderFullscreenTriangle() {
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize());

    glm::mat4 projection = glm::ortho(0.0f, winSize.x, 0.0f, winSize.y);
    Engine::Renderer::sendUniformMatrix4Safe("Model", glm::mat4(1.0f));
    Engine::Renderer::sendUniformMatrix4Safe("VP", projection);
    m_FullscreenTriangle.render();
}
void DeferredPipeline::renderFullscreenQuad() {
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize());

    glm::mat4 projection = glm::ortho(0.0f, winSize.x, 0.0f, winSize.y);
    Engine::Renderer::sendUniformMatrix4Safe("Model", glm::mat4(1.0f));
    Engine::Renderer::sendUniformMatrix4Safe("VP", projection);
    m_FullscreenQuad.render();
}