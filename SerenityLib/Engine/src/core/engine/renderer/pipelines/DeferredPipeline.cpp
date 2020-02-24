#include <core/engine/renderer/pipelines/DeferredPipeline.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/system/Engine.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/renderer/FramebufferObject.h>

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
#include <execution>


using namespace std;
using namespace Engine;
using namespace Engine::priv;
using namespace Engine::Renderer;

priv::DeferredPipeline* pipeline;

struct ShaderEnum final {
    enum Shader {
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
        _TOTAL,
    };
};

struct ShaderProgramEnum final {
    enum Program : unsigned int {
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
        _TOTAL,
    };
};


void GLScissorDisable() {
    const auto winSize = Resources::getWindowSize();
    glScissor(0, 0, winSize.x, winSize.y);
}
void GLScissor(const glm::vec4& s) {
    if (s == glm::vec4(-1.0f)) {
        GLScissorDisable();
    }else{
        glScissor(static_cast<GLint>(s.x), static_cast<GLint>(s.y), static_cast<GLsizei>(s.z), static_cast<GLsizei>(s.w));
    }
}




DeferredPipeline::DeferredPipeline(Engine::priv::Renderer& renderer) : m_Renderer(renderer) {
    pipeline               = this;
}
DeferredPipeline::~DeferredPipeline() {
    SAFE_DELETE(UniformBufferObject::UBO_CAMERA);

    SAFE_DELETE(Texture::White);
    SAFE_DELETE(Texture::Black);
    SAFE_DELETE(Texture::Checkers);
    SAFE_DELETE(Texture::BRDF);
    SAFE_DELETE(Material::Checkers);
    SAFE_DELETE(Material::WhiteShadeless);

    SAFE_DELETE(ShaderProgram::Deferred);
    SAFE_DELETE(ShaderProgram::Forward);
    SAFE_DELETE(ShaderProgram::Decal);

    SAFE_DELETE_VECTOR(m_InternalShaderPrograms);
    SAFE_DELETE_VECTOR(m_InternalShaders);
    //TODO: add cleanup() from ssao / smaa here?
}

void DeferredPipeline::init() {
    const auto window_size = Engine::Resources::getWindowSize();
    m_2DProjectionMatrix   = glm::ortho(0.0f, static_cast<float>(window_size.x), 0.0f, static_cast<float>(window_size.y), 0.005f, 3000.0f);

    float init_border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_InternalShaders.resize(ShaderEnum::_TOTAL, nullptr);
    m_InternalShaderPrograms.resize(ShaderProgramEnum::_TOTAL, nullptr);

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, init_border_color);

    m_OpenGLExtensionsManager.INIT();


    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
    m_OpenGLStateMachine.GL_glDisable(GL_STENCIL_TEST);
    m_OpenGLStateMachine.GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for non Power of Two textures
    //m_OpenGLStateMachine.GL_glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //very odd, supported on my gpu and opengl version but it runs REAL slowly, dropping fps to 1
    m_OpenGLStateMachine.GL_glEnable(GL_DEPTH_CLAMP);
    Engine::Renderer::setDepthFunc(GL_LEQUAL);

    priv::EShaders::init();


    UniformBufferObject::UBO_CAMERA = NEW UniformBufferObject("Camera", sizeof(UBOCameraDataStruct));
    UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraDataStruct);


    m_FullscreenQuad.init();
    m_FullscreenTriangle.init();


    FXAA::fxaa.init_shaders();
    SSAO::ssao.init_shaders();
    HDR::hdr.init_shaders();
    DepthOfField::DOF.init_shaders();
    Bloom::bloom.init_shaders();
    GodRays::godRays.init_shaders();
    SMAA::smaa.init_shaders();

    m_Text_Points.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 points per char, 4096 chars
    m_Text_UVs.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 uvs per char
    m_Text_Indices.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6);//6 ind per char
    for (size_t i = 0; i < m_Text_Points.capacity(); ++i)
        m_Text_Points.emplace_back(0.0f);
    for (size_t i = 0; i < m_Text_UVs.capacity(); ++i)
        m_Text_UVs.emplace_back(0.0f);
    for (size_t i = 0; i < m_Text_Indices.capacity(); ++i)
        m_Text_Indices.emplace_back(0);

    auto& fontPlane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();

    fontPlane.modifyVertices(0, m_Text_Points, MeshModifyFlags::Default);
    fontPlane.modifyVertices(1, m_Text_UVs, MeshModifyFlags::Default);
    fontPlane.modifyIndices(m_Text_Indices, MeshModifyFlags::Default);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    auto emplaceShader = [](const unsigned int index, const string& str, vector<Shader*>& collection, const ShaderType::Type type) {
        Shader* s = NEW Shader(str, type, false);
        collection[index] = s;
    };
    
    priv::threading::addJob([&]() {emplaceShader(0, EShaders::decal_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(1, EShaders::decal_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(2, EShaders::fullscreen_quad_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(3, EShaders::bullet_physics_vert, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(4, EShaders::bullet_physcis_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(5, EShaders::vertex_basic, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(6, EShaders::vertex_2DAPI, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(7, EShaders::vertex_skybox, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(8, EShaders::lighting_vert, m_InternalShaders, ShaderType::Vertex); });

    priv::threading::addJob([&]() {emplaceShader(9, EShaders::forward_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(10, EShaders::deferred_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(11, EShaders::zprepass_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(12, EShaders::deferred_frag_hud, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(13, EShaders::deferred_frag_skybox, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([&]() {emplaceShader(14, EShaders::copy_depth_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(15, EShaders::blur_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([&]() {emplaceShader(16, EShaders::final_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(17, EShaders::depth_and_transparency_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::addJob([&]() {emplaceShader(18, EShaders::lighting_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(19, EShaders::lighting_frag_gi, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(20, EShaders::cubemap_convolude_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(21, EShaders::cubemap_prefilter_envmap_frag, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(22, EShaders::brdf_precompute, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(23, EShaders::stencil_passover, m_InternalShaders, ShaderType::Fragment); });
    priv::threading::addJob([&]() {emplaceShader(24, EShaders::particle_vertex, m_InternalShaders, ShaderType::Vertex); });
    priv::threading::addJob([&]() {emplaceShader(25, EShaders::particle_frag, m_InternalShaders, ShaderType::Fragment); });

    priv::threading::waitForAll();

    
    ShaderProgram::Deferred = NEW ShaderProgram("Deferred", *m_InternalShaders[ShaderEnum::VertexBasic], *m_InternalShaders[ShaderEnum::DeferredFrag]);
    ShaderProgram::Forward = NEW ShaderProgram("Forward", *m_InternalShaders[ShaderEnum::VertexBasic], *m_InternalShaders[ShaderEnum::ForwardFrag]);
    ShaderProgram::Decal = NEW ShaderProgram("Decal", *m_InternalShaders[ShaderEnum::DecalVertex], *m_InternalShaders[ShaderEnum::DecalFrag]);

    m_InternalShaderPrograms[ShaderProgramEnum::BulletPhysics] = NEW ShaderProgram("Bullet_Physics", *m_InternalShaders[ShaderEnum::BulletPhysicsVertex], *m_InternalShaders[ShaderEnum::BulletPhysicsFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::ZPrepass] = NEW ShaderProgram("ZPrepass", *m_InternalShaders[ShaderEnum::VertexBasic], *m_InternalShaders[ShaderEnum::ZPrepassFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::Deferred2DAPI] = NEW ShaderProgram("Deferred_2DAPI", *m_InternalShaders[ShaderEnum::Vertex2DAPI], *m_InternalShaders[ShaderEnum::DeferredFrag2DAPI]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredBlur] = NEW ShaderProgram("Deferred_Blur", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::BlurFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredFinal] = NEW ShaderProgram("Deferred_Final", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::FinalFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DepthAndTransparency] = NEW ShaderProgram("DepthAndTransparency", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::DepthAndTransparencyFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox] = NEW ShaderProgram("Deferred_Skybox", *m_InternalShaders[ShaderEnum::VertexSkybox], *m_InternalShaders[ShaderEnum::DeferredFragSkybox]);
    m_InternalShaderPrograms[ShaderProgramEnum::CopyDepth] = NEW ShaderProgram("Copy_Depth", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::CopyDepthFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLighting] = NEW ShaderProgram("Deferred_Light", *m_InternalShaders[ShaderEnum::LightingVertex], *m_InternalShaders[ShaderEnum::LightingFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGI] = NEW ShaderProgram("Deferred_Light_GI", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::LightingGIFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::CubemapConvolude] = NEW ShaderProgram("Cubemap_Convolude", *m_InternalShaders[ShaderEnum::VertexSkybox], *m_InternalShaders[ShaderEnum::CubemapConvoludeFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::CubemapPrefilterEnv] = NEW ShaderProgram("Cubemap_Prefilter_Env", *m_InternalShaders[ShaderEnum::VertexSkybox], *m_InternalShaders[ShaderEnum::CubemapPrefilterEnvFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance] = NEW ShaderProgram("BRDF_Precompute_CookTorrance", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::BRDFPrecomputeFrag]);
    //m_InternalShaderPrograms[ShaderProgramEnum::Grayscale] = NEW ShaderProgram("Greyscale_Frag", *m_InternalShaders[ShaderEnum::FullscreenVertex], *m_InternalShaders[ShaderEnum::GrayscaleFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::StencilPass] = NEW ShaderProgram("Stencil_Pass", *m_InternalShaders[ShaderEnum::LightingVertex], *m_InternalShaders[ShaderEnum::StencilPassFrag]);
    m_InternalShaderPrograms[ShaderProgramEnum::Particle] = NEW ShaderProgram("Particle", *m_InternalShaders[ShaderEnum::ParticleVertex], *m_InternalShaders[ShaderEnum::ParticleFrag]);



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


    Texture::White = NEW Texture(sfImageWhite, "WhiteTexturePlaceholder", false, ImageInternalFormat::RGBA8);
    Texture::Black = NEW Texture(sfImageBlack, "BlackTexturePlaceholder", false, ImageInternalFormat::RGBA8);
    Texture::Checkers = NEW Texture(sfImageCheckers, "CheckersTexturePlaceholder", false, ImageInternalFormat::RGBA8);
    Texture::Checkers->setFilter(TextureFilter::Nearest);
    Material::Checkers = NEW Material("MaterialDefaultCheckers", Texture::Checkers);
    Material::Checkers->setSpecularModel(SpecularModel::None);
    Material::Checkers->setSmoothness(0.0f);

    Material::WhiteShadeless = NEW Material("MaterialDefaultWhiteShadeless", Texture::White);
    Material::WhiteShadeless->setSpecularModel(SpecularModel::None);
    Material::WhiteShadeless->setSmoothness(0.0f);
    Material::WhiteShadeless->setShadeless(true);

    Texture::BRDF = NEW Texture(512, 512, ImagePixelType::FLOAT, ImagePixelFormat::RG, ImageInternalFormat::RG16F);
    Texture::BRDF->setWrapping(TextureWrap::ClampToEdge);

    SSAO::ssao.init();
    SMAA::smaa.init();

    internal_generate_brdf_lut(*m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance], 512);
}
//DONE
void DeferredPipeline::internal_generate_pbr_data_for_texture(ShaderProgram& covoludeShaderProgram, ShaderProgram& prefilterShaderProgram, Texture& texture, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize) {
    const auto texType = texture.type();
    if (texType != GL_TEXTURE_CUBE_MAP) {
        //cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl;
        return;
    }
    unsigned int size = convoludeTextureSize;
    Engine::Renderer::bindTextureForModification(texType, texture.address(1));
    Engine::Renderer::unbindFBO();
    priv::FramebufferObject fbo(size, size); //try without a depth format
    fbo.bind();

    //make these 2 variables global in the renderer class?
    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f);
    const glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f),glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
    };
    covoludeShaderProgram.bind();

    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType);
    Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(size), static_cast<float>(size));
    for (unsigned int i = 0; i < 6; ++i) {
        const glm::mat4 vp = captureProjection * captureViews[i];
        Engine::Renderer::sendUniformMatrix4("VP", vp);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture.address(1), 0);
        Engine::Renderer::Settings::clear(true, true, false);
        Skybox::bindMesh();
    }
    //TODO: re-check if this is needed
    //Resources::getWindow().display(); //prevent opengl & windows timeout


    //now gen EnvPrefilterMap for specular IBL
    size = preEnvFilterSize;
    Engine::Renderer::bindTextureForModification(texType, texture.address(2));

    prefilterShaderProgram.bind();
    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType);
    Engine::Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix", 12.56637f / float((texture.width() * texture.width()) * 6));
    Engine::Renderer::sendUniform1("NUM_SAMPLES", 32);
    const unsigned int maxMipLevels = 5;
    for (unsigned int m = 0; m < maxMipLevels; ++m) {
        const unsigned int mipSize(size * glm::pow(0.5, m)); // reisze framebuffer according to mip-level size.
        fbo.resize(mipSize, mipSize);
        const float roughness = static_cast<float>(m) / static_cast<float>(maxMipLevels - 1);
        Engine::Renderer::sendUniform1("roughness", roughness);
        const float a = roughness * roughness;
        Engine::Renderer::sendUniform1("a2", a * a);
        for (unsigned int i = 0; i < 6; ++i) {
            const glm::mat4 vp = captureProjection * captureViews[i];
            Engine::Renderer::sendUniformMatrix4("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture.address(2), m);
            Engine::Renderer::Settings::clear(true, true, false);
            Skybox::bindMesh();
        }
    }
    //TODO: re-check if this is needed
    //Resources::getWindow().display(); //prevent opengl & windows timeout
    fbo.unbind();
}
//DONE
void DeferredPipeline::internal_generate_brdf_lut(ShaderProgram& program, const unsigned int& brdfSize) {
    FramebufferObject fbo(brdfSize, brdfSize); //try without a depth format
    fbo.bind();

    Engine::Renderer::bindTextureForModification(GL_TEXTURE_2D, Texture::BRDF->address());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToEdge);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture::BRDF->address(), 0);

    program.bind();
    Engine::Renderer::sendUniform1("NUM_SAMPLES", 256);
    Engine::Renderer::Settings::clear(true, true, false);
    Engine::Renderer::colorMask(true, true, false, false);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::onPipelineChanged() {

}

const unsigned int DeferredPipeline::getUniformLocation(const char* location) {
    const auto& uniforms = m_RendererState.current_bound_shader_program->uniforms();
    if (!uniforms.count(location)) {
        return -1;
    }
    return uniforms.at(location);
}
const unsigned int DeferredPipeline::getUniformLocationUnsafe(const char* location) {
    return m_RendererState.current_bound_shader_program->uniforms().at(location);
}
const unsigned int DeferredPipeline::getMaxNumTextureUnits() {
    return m_OpenGLStateMachine.getMaxTextureUnits();
}
void DeferredPipeline::restoreDefaultState() {
    const auto winWidth = Resources::getWindowSize();
    m_OpenGLStateMachine.GL_RESTORE_DEFAULT_STATE_MACHINE(winWidth.x, winWidth.y);
}
void DeferredPipeline::restoreCurrentState() {
    m_OpenGLStateMachine.GL_RESTORE_CURRENT_STATE_MACHINE();
}
void DeferredPipeline::clear2DAPI() {
    m_2DAPICommands.clear();
}
void DeferredPipeline::sort2DAPI() {
    const auto& lambda_sorter = [&](API2DCommand& lhs, API2DCommand& rhs) {
        return lhs.depth > rhs.depth;
    };
    std::sort(std::execution::par_unseq, m_2DAPICommands.begin(), m_2DAPICommands.end(), lambda_sorter);
}

const bool DeferredPipeline::stencilOperation(const unsigned int stencilFail, const unsigned int depthFail, const unsigned int depthPass) {
    return m_OpenGLStateMachine.GL_glStencilOp(stencilFail, depthFail, depthPass);
}
const bool DeferredPipeline::stencilMask(const unsigned int mask) {
    return m_OpenGLStateMachine.GL_glStencilMask(mask);
}
const bool DeferredPipeline::stencilFunction(const unsigned int stencilFunction, const unsigned int reference, const unsigned int mask) {
    return m_OpenGLStateMachine.GL_glStencilFunc(stencilFunction, reference, mask);
}
const bool DeferredPipeline::setDepthFunction(const unsigned int depthFunction) {
    return m_OpenGLStateMachine.GL_glDepthFunc(depthFunction);
}
const bool DeferredPipeline::setViewport(const float x, const float y, const float width, const float height) {
    return m_OpenGLStateMachine.GL_glViewport(
        static_cast<GLint>(x), 
        static_cast<GLint>(y), 
        static_cast<GLsizei>(width), 
        static_cast<GLsizei>(height)
    );
}
void DeferredPipeline::clear(const bool color, const bool depth, const bool stencil) {
    if (!color && !depth && !stencil) {
        return;
    }
    GLuint clearBit = 0x00000000;
    if (color)   clearBit |= GL_COLOR_BUFFER_BIT;
    if (depth)   clearBit |= GL_DEPTH_BUFFER_BIT;
    if (stencil) clearBit |= GL_STENCIL_BUFFER_BIT;
    glClear(clearBit);
}
const bool DeferredPipeline::colorMask(const bool r, const bool g, const bool b, const bool alpha) {
    return m_OpenGLStateMachine.GL_glColorMask(r, g, b, alpha);
}
const bool DeferredPipeline::clearColor(const bool r, const bool g, const bool b, const bool alpha) {
    return m_OpenGLStateMachine.GL_glClearColor(r, g, b, alpha);
}
const bool DeferredPipeline::bindTextureForModification(const unsigned int textureType, const unsigned int textureObject) {
    return m_OpenGLStateMachine.GL_glBindTextureForModification(textureType, textureObject);
}
const bool DeferredPipeline::bindVAO(const unsigned int vaoObject) {
    return m_OpenGLStateMachine.GL_glBindVertexArray(vaoObject);
}
const bool DeferredPipeline::deleteVAO(unsigned int& vaoObject) {
    if (vaoObject) {
        glDeleteVertexArrays(1, &vaoObject);
        vaoObject = 0;
        return true;
    }
    return false;
}
void DeferredPipeline::generateAndBindTexture(const unsigned int textureType, unsigned int& textureObject) {
    glGenTextures(1, &textureObject);
    m_OpenGLStateMachine.GL_glBindTextureForModification(textureType, textureObject);
}
void DeferredPipeline::generateAndBindVAO(unsigned int& vaoObject) {
    glGenVertexArrays(1, &vaoObject);
    DeferredPipeline::bindVAO(vaoObject);
}
const bool DeferredPipeline::enableAPI(const unsigned int apiEnum) {
    return m_OpenGLStateMachine.GL_glEnable(apiEnum);
}
const bool DeferredPipeline::disableAPI(const unsigned int apiEnum) {
    return m_OpenGLStateMachine.GL_glDisable(apiEnum);
}
const bool DeferredPipeline::enableAPI_i(const unsigned int apiEnum, const unsigned int index) {
    return m_OpenGLStateMachine.GL_glEnablei(apiEnum, index);
}
const bool DeferredPipeline::disableAPI_i(const unsigned int apiEnum, const unsigned int index) {
    return m_OpenGLStateMachine.GL_glDisablei(apiEnum, index);
}
void DeferredPipeline::sendTexture(const char* location, const Texture& texture, const int slot) {
    m_OpenGLStateMachine.GL_glActiveTexture(slot);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(texture.type(), texture.address());
    Engine::Renderer::sendUniform1(location, slot);
}
void DeferredPipeline::sendTexture(const char* location, const unsigned int textureObject, const int slot, const unsigned int& textureTarget) {
    m_OpenGLStateMachine.GL_glActiveTexture(slot);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    Engine::Renderer::sendUniform1(location, slot);
}
void DeferredPipeline::sendTextureSafe(const char* location, const Texture& texture, const int slot) {
    m_OpenGLStateMachine.GL_glActiveTexture(slot);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(texture.type(), texture.address());
    Engine::Renderer::sendUniform1Safe(location, slot);
}
void DeferredPipeline::sendTextureSafe(const char* location, const unsigned int textureObject, const int slot, const unsigned int& textureTarget) {
    m_OpenGLStateMachine.GL_glActiveTexture(slot);
    m_OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    Engine::Renderer::sendUniform1Safe(location, slot);
}





const bool DeferredPipeline::cullFace(const unsigned int face) {
    return m_OpenGLStateMachine.GL_glCullFace(face);
}
const bool DeferredPipeline::bindReadFBO(const unsigned int fbo) {
    return m_OpenGLStateMachine.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
}
const bool DeferredPipeline::bindDrawFBO(const unsigned int fbo) {
    return m_OpenGLStateMachine.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}
const bool DeferredPipeline::bindRBO(const unsigned int rbo) {
    return m_OpenGLStateMachine.GL_glBindRenderbuffer(rbo);
}

const bool DeferredPipeline::bindShaderProgram(ShaderProgram* program) {
    auto& currentShaderPgrm = m_RendererState.current_bound_shader_program;
    if (currentShaderPgrm != program) {
        m_OpenGLStateMachine.GL_glUseProgram(program->program());
        currentShaderPgrm = program;
        currentShaderPgrm->BindableResource::bind();
        return true;
    }
    return false;
}
const bool DeferredPipeline::bindMaterial(Material* material) {
    auto& currentMaterial = m_RendererState.current_bound_material;
    if (currentMaterial != material) {
        currentMaterial = material;
        currentMaterial->BindableResource::bind();
        return true;
    }
    return false;
}
const bool DeferredPipeline::bindMesh(Mesh* mesh) {
    return false;
}
const bool DeferredPipeline::unbindShaderProgram() {
    auto& currentShaderPgrm = m_RendererState.current_bound_shader_program;
    if (currentShaderPgrm) {
        currentShaderPgrm->BindableResource::unbind();
        currentShaderPgrm = nullptr;
        m_OpenGLStateMachine.GL_glUseProgram(0);
        return true;
    }
    return false;
}
const bool DeferredPipeline::unbindMaterial() {
    auto& currentMaterial = m_RendererState.current_bound_material;
    if (currentMaterial) {
        currentMaterial->BindableResource::unbind();
        currentMaterial = nullptr;
        return true;
    }
    return false;
}
const bool DeferredPipeline::unbindMesh() {
    return false;
}
void DeferredPipeline::generatePBRData(Texture& texture, const unsigned int& convoludeSize, const unsigned int& prefilterSize) {
    internal_generate_pbr_data_for_texture(
        *m_InternalShaderPrograms[ShaderProgramEnum::CubemapConvolude], 
        *m_InternalShaderPrograms[ShaderProgramEnum::CubemapPrefilterEnv], 
        texture, 
        convoludeSize, 
        prefilterSize
    );
}

void DeferredPipeline::onFullscreen() {
    //TODO: move these lines to a more generic area, all rendering pipelines will pretty much do this
    restoreCurrentOpenGLState();

    Engine::Renderer::GLEnable(GL_CULL_FACE);
    Engine::Renderer::GLEnable(GL_DEPTH_CLAMP);

    const auto winSize = Resources::getWindowSize();
    m_GBuffer.init(winSize.x, winSize.y);
}
//DONE
void DeferredPipeline::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    const auto floatWidth  = static_cast<float>(newWidth);
    const auto floatHeight = static_cast<float>(newHeight);

    m_2DProjectionMatrix = glm::ortho(0.0f, floatWidth, 0.0f, floatHeight, 0.005f, 3000.0f);

    m_FullscreenQuad.changeDimensions(floatWidth, floatHeight);
    m_FullscreenTriangle.changeDimensions(floatWidth, floatHeight);

    m_GBuffer.resize(newWidth, newHeight);
}
void DeferredPipeline::onOpenGLContextCreation(const unsigned int windowWidth, const unsigned int windowHeight, const unsigned int glslVersion, const unsigned int openglVersion) {
    //TODO: move to a more generic area
    Engine::priv::Renderer::GLSL_VERSION   = glslVersion;
    Engine::priv::Renderer::OPENGL_VERSION = openglVersion;
    m_OpenGLStateMachine.GL_INIT_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
    Engine::Renderer::GLEnable(GL_CULL_FACE);
    m_GBuffer.init(windowWidth, windowHeight);
}
//DONE
void DeferredPipeline::renderSkybox(Skybox* skybox, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera) {
    glm::mat4 view_no_position = camera.getView();
    Math::removeMatrixPosition(view_no_position);

    shaderProgram.bind();
    if (skybox) {
        Engine::Renderer::sendUniform1("IsFake", 0);
        Engine::Renderer::sendTextureSafe("Texture", skybox->texture()->address(0), 0, GL_TEXTURE_CUBE_MAP);
    }else{
        Engine::Renderer::sendUniform1("IsFake", 1);
        const auto& bgColor = scene.getBackgroundColor();
        Engine::Renderer::sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    }
    Engine::Renderer::sendUniformMatrix4("VP", camera.getProjection() * view_no_position);
    Skybox::bindMesh();

    Engine::Renderer::sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
    //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...
}
//DONE
void DeferredPipeline::renderDirectionalLight(const Camera& c, const DirectionalLight& d, const Viewport& viewport) {
    if (!d.isActive())
        return;
    const auto& body    = *d.getComponent<ComponentBody>();
    const auto forward  = glm::vec3(body.forward());
    const auto& col     = d.color();
    sendUniform4("light.DataA", d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), forward.x);
    sendUniform4("light.DataB", forward.y, forward.z, 0.0f, 0.0f);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(d.type()));
    sendUniform1Safe("Type", 0.0f);

    renderFullscreenQuad();
}
//DONE
void DeferredPipeline::renderSunLight(const Camera& c, const SunLight& s, const Viewport& viewport) {
    if (!s.isActive())
        return;
    const auto& body = *s.getComponent<ComponentBody>();
    const auto pos   = glm::vec3(body.position());
    const auto& col  = s.color();
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
    sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    sendUniform1Safe("Type", 0.0f);

    renderFullscreenQuad();
}
//DONE
void DeferredPipeline::renderPointLight(const Camera& c, const PointLight& p) {
    if (!p.isActive())
        return;
    const auto& body  = *p.getComponent<ComponentBody>();
    const auto pos    = glm::vec3(body.position());
    const auto cull   = p.getCullingRadius();
    const auto factor = 1100.0f * cull;
    const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
    if ((!c.sphereIntersectTest(pos, cull)) || (distSq > factor * factor))
        return;

    const auto& col = p.color();
    sendUniform4("light.DataA", p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
    sendUniform4("light.DataB", 0.0f, 0.0f, p.getConstant(), p.getLinear());
    sendUniform4("light.DataC", p.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(p.type()));
    sendUniform4Safe("light.DataE", 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& pointLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPointLightBounds();

    pointLightMesh.bind();
    pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    pointLightMesh.unbind();
    cullFace(GL_BACK);
}
//DONE
void DeferredPipeline::renderSpotLight(const Camera& c, const SpotLight& s) {
    return;
    const auto& body    = *s.entity().getComponent<ComponentBody>();
    const auto pos      = glm::vec3(body.position());
    const auto forward  = glm::vec3(body.forward());
    const auto cull     = s.getCullingRadius();
    const auto factor   = 1100.0f * cull;
    const auto distSq   = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cull) || (distSq > factor * factor))
        return;
    const auto& col = s.color();
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), forward.x);
    sendUniform4("light.DataB", forward.y, forward.z, s.getConstant(), s.getLinear());
    sendUniform4("light.DataC", s.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    sendUniform4Safe("light.DataE", s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
    sendUniform2Safe("VertexShaderData", s.getCutoffOuter(), cull);
    sendUniform1Safe("Type", 2.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume                                                 
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& spotLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getSpotLightBounds();

    spotLightMesh.bind();
    spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    spotLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
}
//DONE
void DeferredPipeline::renderRodLight(const Camera& c, const RodLight& r) {
    if (!r.isActive())
        return;
    const auto& body           = *r.entity().getComponent<ComponentBody>();
    const auto pos             = glm::vec3(body.position());
    const auto cullingDistance = r.rodLength() + (r.getCullingRadius() * 2.0f);
    const auto factor          = 1100.0f * cullingDistance;
    const auto distSq          = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cullingDistance) || (distSq > factor * factor))
        return;
    const auto& col       = r.color();
    const float half      = r.rodLength() / 2.0f;
    const auto firstEndPt = pos + (glm::vec3(body.forward()) * half);
    const auto secndEndPt = pos - (glm::vec3(body.forward()) * half);
    sendUniform4("light.DataA", r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
    sendUniform4("light.DataB", firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
    sendUniform4("light.DataC", r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(r.type()));
    sendUniform4Safe("light.DataE", r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cullingDistance * cullingDistance)) {
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& rodLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getRodLightBounds();

    rodLightMesh.bind();
    rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    rodLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
}
void DeferredPipeline::renderDecal(ModelInstance& decalModelInstance) {
    const auto& stage      = decalModelInstance.stage();
    const Entity& parent   = decalModelInstance.parent();
    auto& scene            = parent.scene();
    Camera& cam            = *scene.getActiveCamera();
    glm::vec3 camPos       = cam.getPosition();
    auto& body             = *(parent.getComponent<ComponentBody>());
    glm::mat4 parentModel  = body.modelMatrix();
    const auto maxTextures = getMaxNumTextureUnits() - 1U;

    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform1Safe("Object_Color", decalModelInstance.color().toPackedInt());
    Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", decalModelInstance.godRaysColor().toPackedInt());

    glm::mat4 modelMatrix  = parentModel * decalModelInstance.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    const auto& winSize    = Resources::getWindowSize();

    Engine::Renderer::sendUniform4Safe("Resolution", static_cast<float>(winSize.x), static_cast<float>(winSize.y), 0.0f, 0.0f);
    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}
void DeferredPipeline::renderParticle(const Particle& particle) {
    particle.getMaterial()->bind();

    auto maxTextures = m_OpenGLStateMachine.getMaxTextureUnits() - 1;

    Camera& camera = *particle.scene().getActiveCamera();
    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(Engine::priv::GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform1Safe("Object_Color", particle.color().toPackedInt());
    Engine::Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, particle.position());
    modelMatrix          *= glm::mat4_cast(camera.getOrientation());
    modelMatrix           = glm::rotate(modelMatrix, particle.angle(), glm::vec3(0, 0, 1));
    const auto& scale     = particle.getScale();
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));

    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

    priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().render();
}
void DeferredPipeline::renderMesh(const Mesh& mesh, const unsigned int mode) {
    const auto indicesSize = mesh.getVertexData().indices.size();
    if (indicesSize == 0)
        return;
    //if (instancing && priv::InternalMeshPublicInterface::SupportsInstancing()) {
        //const uint& instancesCount = m_InstanceCount;
        //if (instancesCount == 0) 
        //    return;
        //if (Renderer::OPENGL_VERSION >= 31) {
        //    glDrawElementsInstanced(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (OpenGLExtension::supported(OpenGLExtension::EXT_draw_instanced)) {
        //    glDrawElementsInstancedEXT(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (OpenGLExtension::supported(OpenGLExtension::ARB_draw_instanced)) {
        //    glDrawElementsInstancedARB(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //}
    //}
    //else {
        glDrawElements(mode, (GLsizei)indicesSize, GL_UNSIGNED_SHORT, nullptr);
    //}
}

void DeferredPipeline::internal_render_2d_text_left(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    unsigned int i = 0;
    for (auto& character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight;
            x = 0.0f;
        }else if (character != '\0') {
            const unsigned int accum = i * 4;
            ++i;
            const FontGlyph& chr = font.getGlyphData(character);
            const float startingY = -int(chr.height + chr.yoffset) - y;

            m_Text_Indices.emplace_back(accum + 0);
            m_Text_Indices.emplace_back(accum + 1);
            m_Text_Indices.emplace_back(accum + 2);
            m_Text_Indices.emplace_back(accum + 3);
            m_Text_Indices.emplace_back(accum + 1);
            m_Text_Indices.emplace_back(accum + 0);

            const float startingX = x + chr.xoffset;
            x += chr.xadvance;

            for(unsigned int i = 0; i < 4; ++i)
                m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

            for (unsigned int i = 0; i < 4; ++i)
                m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
        }
    }
}
void DeferredPipeline::internal_render_2d_text_center(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string> lines;
    vector<unsigned short> lines_sizes;
    string line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            lines_sizes.push_back(static_cast<unsigned short>(x));
            line_accumulator = "";
            x = 0.0f;
            continue;
        }else if (character != '\0') {
            const FontGlyph& chr = font.getGlyphData(character);
            line_accumulator += character;
            x += chr.xadvance;
        }
    }
    if (!line_accumulator.empty()) {
        lines.push_back(line_accumulator);
        lines_sizes.push_back(static_cast<unsigned short>(x));
    }

    x = 0.0f;
    unsigned int i = 0;
    for (uint l = 0; l < lines.size(); ++l) {
        const auto& line = lines[l];
        const auto& line_size = lines_sizes[l] / 2;
        for (auto& character : line) {
            if (character != '\0') {
                const unsigned int accum = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                const float startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accum + 0);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 2);
                m_Text_Indices.emplace_back(accum + 3);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 0);

                const float startingX = x + chr.xoffset;
                x += chr.xadvance;

                for (unsigned int i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x - line_size, startingY + chr.pts[i].y, z);

                for (unsigned int i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}
void DeferredPipeline::internal_render_2d_text_right(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string> lines;
    string line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            line_accumulator = "";
            continue;
        }else if (character != '\0') {
            line_accumulator += character;
        }
    }
    if (lines.size() == 0)
        lines.push_back(line_accumulator);

    unsigned int i = 0;
    for (auto& line : lines) {
        const int line_size = static_cast<int>(line.size());
        int k = 0;
        for (int j = line_size; j >= 0; --j) {
            const auto& character = line[j];
            if (character != '\0') {
                const unsigned int accum = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                const float startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accum + 0);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 2);
                m_Text_Indices.emplace_back(accum + 3);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 0);

                if (k == 0) {
                    x -= chr.width;
                }
                const float startingX = x + chr.xoffset;
                x -= chr.xadvance;

                for (unsigned int i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

                for (unsigned int i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);

                ++k;
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}


void DeferredPipeline::render2DText(const string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    auto& fontPlane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();
    fontPlane.bind();

    const auto  newLineGlyphHeight = font.getGlyphData('X').height + 12;
    const auto& texture = font.getGlyphTexture();
    float y = 0.0f;
    float x = 0.0f;
    float z = -0.001f - depth;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1));

    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    Engine::Renderer::sendTexture("DiffuseTexture", texture, 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    if (textAlignment == TextAlignment::Left) {
        internal_render_2d_text_left(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Right) {
        internal_render_2d_text_right(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Center) {
        internal_render_2d_text_center(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }
    fontPlane.modifyVertices(0, m_Text_Points, MeshModifyFlags::Default); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(1, m_Text_UVs);
    fontPlane.modifyIndices(m_Text_Indices);
    fontPlane.render();


}
void DeferredPipeline::render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& plane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    plane.bind();

    float translationX = position.x;
    float translationY = position.y;
    float totalSizeX   = scale.x;
    float totalSizeY   = scale.y;
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

    plane.render();
}
void DeferredPipeline::render2DTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& alignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& triangle = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh();
    triangle.bind();

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

    triangle.render();
}


void DeferredPipeline::internal_render_per_frame_preparation(const Viewport& viewport, const Camera& camera) {
    const auto& winSize    = Resources::getWindowSize();
    const auto& dimensions = glm::vec4(viewport.getViewportDimensions());
    if (viewport.isAspectRatioSynced()) {
        camera.setAspect(dimensions.z / dimensions.w);
    }
    //Engine::Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w); //gbuffer.bind already does this
    glScissor(0, 0, winSize.x, winSize.y);

    m_2DProjectionMatrix = glm::ortho(0.0f, dimensions.z, 0.0f, dimensions.w, 0.005f, 3000.0f); //might have to recheck this
    //this is god awful and ugly, but it's needed. find a way to refactor this properly
    for (uint i = 0; i < 9; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}
void DeferredPipeline::internal_pass_geometry(const Viewport& viewport, const Camera& camera) {
    const Scene& scene     = viewport.getScene();
    const glm::vec4& clear = viewport.getBackgroundColor();
    const float colors[4]  = { clear.r, clear.g, clear.b, clear.a };

    m_GBuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

    Engine::Renderer::Settings::clear(true, true, true); // (0,0,0,0)

    Engine::Renderer::setDepthFunc(GL_LEQUAL);

    glClearBufferfv(GL_COLOR, 0, colors);
    auto& godRays = GodRays::godRays;
    if (godRays.godRays_active) {
        const float godraysclearcolor[4] = { godRays.clearColor.r, godRays.clearColor.g, godRays.clearColor.b, godRays.clearColor.a };
        glClearBufferfv(GL_COLOR, 2, godraysclearcolor);
    }
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure
    InternalScenePublicInterface::RenderGeometryOpaque(m_Renderer, scene, viewport, camera);
    if ((viewport.getRenderFlags() & ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), *m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    InternalScenePublicInterface::RenderGeometryTransparent(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(m_Renderer, scene, viewport, camera, true);
}
void DeferredPipeline::internal_pass_ssao(const Viewport& viewport, const Camera& camera) {
    //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes
    m_GBuffer.bindFramebuffers(GBufferType::Bloom, GBufferType::GodRays, "A", false);
    Engine::Renderer::Settings::clear(true, false, false); //bloom and god rays alpha channels cleared to black 
    if (SSAO::ssao.m_ssao && (viewport.getRenderFlags() & ViewportRenderingFlag::SSAO)) {
        Engine::Renderer::GLEnablei(GL_BLEND, 0);//i dont think this is needed anymore
        m_GBuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
        SSAO::ssao.passSSAO(m_GBuffer, viewport, camera); 
        if (SSAO::ssao.m_ssao_do_blur) {
            Engine::Renderer::GLDisablei(GL_BLEND, 0); //yes this is absolutely needed
            for (unsigned int i = 0; i < SSAO::ssao.m_ssao_blur_num_passes; ++i) {
                m_GBuffer.bindFramebuffers(GBufferType::GodRays, "A", false);
                SSAO::ssao.passBlur(m_GBuffer, viewport, "H", GBufferType::Bloom);
                m_GBuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
                SSAO::ssao.passBlur(m_GBuffer, viewport, "V", GBufferType::GodRays);
            }
        }  
    }
}
void DeferredPipeline::internal_pass_stencil() {
    Engine::Renderer::colorMask(false, false, false, false);
    m_InternalShaderPrograms[ShaderProgramEnum::StencilPass]->bind();

    m_GBuffer.getMainFBO().bind();

    Engine::Renderer::GLEnable(GL_STENCIL_TEST);
    Engine::Renderer::Settings::clear(false, false, true); //stencil is completely filled with 0's
    Engine::Renderer::stencilMask(0xFFFFFFFF);

    Engine::Renderer::stencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
    //exclude shadeless normals
    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);

    Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 0);
    Engine::Renderer::sendUniform1("Type", 0.0f);
    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil
    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::internal_pass_lighting(const Viewport& viewport, const Camera& camera, const bool mainRenderFunction) {
    const Scene& scene = viewport.getScene();
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredLighting]->bind();

    const auto& winSize = glm::vec2(Engine::Resources::getWindowSize());

    if (Renderer::GLSL_VERSION < 140) {
        Engine::Renderer::sendUniformMatrix4Safe("CameraView", camera.getView());
        Engine::Renderer::sendUniformMatrix4Safe("CameraProj", camera.getProjection());
        //sendUniformMatrix4Safe("CameraViewProj",camera.getViewProjection()); //moved to shader binding function
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
        Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
        Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
        Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
    }

    Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, static_cast<unsigned int>(Material::m_MaterialProperities.size()));
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
    Engine::Renderer::setDepthFunc(GL_LEQUAL);
    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    for (const auto& light : InternalScenePublicInterface::GetSunLights(scene)) {
        renderSunLight(camera, *light, viewport);
    }
    for (const auto& light : InternalScenePublicInterface::GetDirectionalLights(scene)) {
        renderDirectionalLight(camera, *light, viewport);
    }
    
    if (mainRenderFunction) {
        //do GI here. (only doing GI during the main render pass, not during light probes
        m_InternalShaderPrograms[ShaderProgramEnum::DeferredLightingGI]->bind();
        if (Renderer::GLSL_VERSION < 140) {
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
            Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
            Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
            Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
        }

        Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, static_cast<unsigned int>(Material::m_MaterialProperities.size()));
        Engine::Renderer::sendUniform4Safe("ScreenData", m_Renderer.m_GI_Pack, m_Renderer.m_Gamma, winSize.x, winSize.y);
        Engine::Renderer::sendTexture("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 0);
        Engine::Renderer::sendTexture("gNormalMap", m_GBuffer.getTexture(GBufferType::Normal), 1);
        Engine::Renderer::sendTexture("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 2);
        Engine::Renderer::sendTexture("gSSAOMap", m_GBuffer.getTexture(GBufferType::Bloom), 3);
        Engine::Renderer::sendTexture("gMiscMap", m_GBuffer.getTexture(GBufferType::Misc), 4);
        Skybox* skybox = scene.skybox();
        if (skybox && skybox->texture()->numAddresses() >= 3) {
            Engine::Renderer::sendTextureSafe("irradianceMap", skybox->texture()->address(1), 5, GL_TEXTURE_CUBE_MAP);
            Engine::Renderer::sendTextureSafe("prefilterMap", skybox->texture()->address(2), 6, GL_TEXTURE_CUBE_MAP);
            Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, 7);
        }else{
            Engine::Renderer::sendTextureSafe("irradianceMap", Texture::Black->address(0), 5, GL_TEXTURE_2D);
            Engine::Renderer::sendTextureSafe("prefilterMap", Texture::Black->address(0), 6, GL_TEXTURE_2D);
            Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, 7);
        }
        Engine::Renderer::renderFullscreenQuad();
    }
}
void DeferredPipeline::internal_pass_forward(const Viewport& viewport, const Camera& camera) {
    const Scene& scene = viewport.getScene();

    m_GBuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, GBufferType::Lighting, "RGBA");
    InternalScenePublicInterface::RenderForwardOpaque(m_Renderer, scene, viewport, camera);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (unsigned int i = 0; i < 4; ++i) {
        Engine::Renderer::GLEnablei(GL_BLEND, i);
    }

    glDepthMask(GL_TRUE);
    InternalScenePublicInterface::RenderForwardTransparent(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(m_Renderer, scene, viewport, camera);
    glDepthMask(GL_FALSE);
    InternalScenePublicInterface::RenderDecals(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderForwardParticles(m_Renderer, scene, viewport, camera);
    InternalScenePublicInterface::RenderParticles(m_Renderer, scene, viewport, camera, *m_InternalShaderPrograms[ShaderProgramEnum::Particle], m_GBuffer);

    for (unsigned int i = 0; i < 4; ++i) {
        Engine::Renderer::GLDisablei(GL_BLEND, i); //this is needed for smaa at least
    }
}
void DeferredPipeline::internal_pass_god_rays(const Viewport& viewport, const Camera& camera) {
    m_GBuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
    Engine::Renderer::Settings::clear(true, false, false); //godrays rgb channels cleared to black
    auto& godRaysPlatform = GodRays::godRays;
    auto* sun = Engine::Renderer::godRays::getSun();
    if (sun && (viewport.getRenderFlags() & ViewportRenderingFlag::GodRays) && godRaysPlatform.godRays_active) {
        const auto& body       = *sun->getComponent<ComponentBody>();
        const glm::vec3 oPos   = body.position();
        const glm::vec3 camPos = camera.getPosition();
        const glm::vec3 camVec = camera.getViewVector();
        const bool infront     = Engine::Math::isPointWithinCone(camPos, -camVec, oPos, Engine::Math::toRadians(godRaysPlatform.fovDegrees));
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
            godRaysPlatform.pass(m_GBuffer, viewport, glm::vec2(sp.x, sp.y), alpha);
        }
    }
}
void DeferredPipeline::internal_pass_hdr(const Viewport& viewport, const Camera& camera) {
    const glm::uvec4& dimensions = viewport.getViewportDimensions();
    m_GBuffer.bindFramebuffers(GBufferType::Misc);
    HDR::hdr.pass(m_GBuffer, viewport, GodRays::godRays.godRays_active, m_Renderer.m_Lighting, GodRays::godRays.factor);
}
void DeferredPipeline::internal_pass_bloom(const Viewport& viewport) {
    if (Bloom::bloom.bloom_active && (viewport.getRenderFlags() & ViewportRenderingFlag::Bloom)) {
        m_GBuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
        Bloom::bloom.pass(m_GBuffer, viewport, GBufferType::Lighting);
        for (unsigned int i = 0; i < Bloom::bloom.num_passes; ++i) {
            m_GBuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
            internal_pass_blur(viewport, GBufferType::Bloom, "H");
            m_GBuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
            internal_pass_blur(viewport, GBufferType::GodRays, "V");
        }
    }
}
void DeferredPipeline::internal_pass_depth_of_field(const Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (DepthOfField::DOF.dof && (viewport.getRenderFlags() & ViewportRenderingFlag::DepthOfField)) {
        m_GBuffer.bindFramebuffers(outTexture);
        DepthOfField::DOF.pass(m_GBuffer, viewport, sceneTexture);
        sceneTexture = GBufferType::Lighting;
        outTexture   = GBufferType::Misc;
    }
}
void DeferredPipeline::internal_pass_aa(const bool mainRenderFunction, const Viewport& viewport, const Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture) {
    if (!mainRenderFunction || m_Renderer.m_AA_algorithm == AntiAliasingAlgorithm::None || !(viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)) {
        m_GBuffer.bindFramebuffers(outTexture);
        internal_pass_final(sceneTexture);
        m_GBuffer.bindBackbuffer(viewport/*, fbo, rbo*/);
        internal_pass_depth_and_transparency(viewport, outTexture);
    }else if (mainRenderFunction && m_Renderer.m_AA_algorithm == AntiAliasingAlgorithm::FXAA && (viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)) {
        m_GBuffer.bindFramebuffers(outTexture);
        internal_pass_final(sceneTexture);
        m_GBuffer.bindFramebuffers(sceneTexture);
        FXAA::fxaa.pass(m_GBuffer, viewport, outTexture);

        m_GBuffer.bindBackbuffer(viewport/*, fbo, rbo*/);
        internal_pass_depth_and_transparency(viewport, sceneTexture);

    }else if (mainRenderFunction && (m_Renderer.m_AA_algorithm >= AntiAliasingAlgorithm::SMAA_LOW || m_Renderer.m_AA_algorithm <= AntiAliasingAlgorithm::SMAA_ULTRA) && (viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)) {
        m_GBuffer.bindFramebuffers(outTexture);
        internal_pass_final(sceneTexture);

        std::swap(sceneTexture, outTexture);

        const auto winSize = glm::vec2(Resources::getWindowSize());
        const glm::vec4& SMAA_PIXEL_SIZE = glm::vec4(1.0f / winSize.x, 1.0f / winSize.y, winSize.x, winSize.y);

        SMAA::smaa.passEdge(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture, outTexture);
        SMAA::smaa.passBlend(m_GBuffer, SMAA_PIXEL_SIZE, viewport, outTexture);
        m_GBuffer.bindFramebuffers(outTexture);
        SMAA::smaa.passNeighbor(m_GBuffer, SMAA_PIXEL_SIZE, viewport, sceneTexture);
        //m_GBuffer.bindFramebuffers(sceneTexture);

        //SMAA::smaa.passFinal(m_GBuffer, viewport);//unused

        m_GBuffer.bindBackbuffer(viewport/*, fbo, rbo*/);
        internal_pass_depth_and_transparency(viewport, outTexture);
    }
}
void DeferredPipeline::internal_pass_final(const GBufferType::Type& sceneTexture) {
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredFinal]->bind();
    Engine::Renderer::sendUniform1Safe("HasBloom", static_cast<int>(Bloom::bloom.bloom_active));
    Engine::Renderer::sendUniform1Safe("HasFog", static_cast<int>(Fog::fog.fog_active));

    if (Fog::fog.fog_active) {
        Engine::Renderer::sendUniform1Safe("FogDistNull", Fog::fog.distNull);
        Engine::Renderer::sendUniform1Safe("FogDistBlend", Fog::fog.distBlend);
        Engine::Renderer::sendUniform4Safe("FogColor", Fog::fog.color);
        Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 2);
    }
    Engine::Renderer::sendTextureSafe("SceneTexture", m_GBuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("gBloomMap", m_GBuffer.getTexture(GBufferType::Bloom), 1);
    Engine::Renderer::sendTextureSafe("gDiffuseMap", m_GBuffer.getTexture(GBufferType::Diffuse), 2);
    Engine::Renderer::renderFullscreenQuad();
}

void DeferredPipeline::internal_pass_depth_and_transparency(const Viewport& viewport, const GBufferType::Type& sceneTexture) {
    m_InternalShaderPrograms[ShaderProgramEnum::DepthAndTransparency]->bind();

    Engine::Renderer::sendTextureSafe("SceneTexture", m_GBuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), 1);

    Engine::Renderer::GLEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //sendUniform4Safe("TransparencyMaskColor", viewport.getTransparencyMaskColor());
    //sendUniform1Safe("TransparencyMaskActive", static_cast<int>(viewport.isTransparencyMaskActive()));
    Engine::Renderer::sendUniform1Safe("DepthMaskValue", viewport.getDepthMaskValue());
    Engine::Renderer::sendUniform1Safe("DepthMaskActive", static_cast<int>(viewport.isDepthMaskActive()));

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::GLDisable(GL_BLEND);
}
void DeferredPipeline::internal_pass_copy_depth() {

}
void DeferredPipeline::internal_pass_blur(const Viewport& viewport, const GLuint texture, const std::string& type) {
    m_InternalShaderPrograms[ShaderProgramEnum::DeferredBlur]->bind();

    const auto& dimensions = viewport.getViewportDimensions();

    const float& divisor = m_GBuffer.getSmallFBO().divisor();
    glm::vec2 hv(0.0f);
    if (type == "H") { 
        hv = glm::vec2(1.0f, 0.0f); 
    }else{ 
        hv = glm::vec2(0.0f, 1.0f); 
    }

    const glm::ivec2 Res(dimensions.z, dimensions.w);
    auto& bloom = Bloom::bloom;
    Engine::Renderer::sendUniform4("strengthModifier",
        bloom.blur_strength,
        bloom.blur_strength,
        bloom.blur_strength,
        SSAO::ssao.m_ssao_blur_strength
    );
    Engine::Renderer::sendUniform2("Resolution", Res);
    Engine::Renderer::sendUniform4("DataA", bloom.blur_radius, 0.0f, hv.x, hv.y);
    Engine::Renderer::sendTexture("image", m_GBuffer.getTexture(texture), 0);

    Engine::Renderer::renderFullscreenQuad();
}



void DeferredPipeline::update(const float& dt) {

}
void DeferredPipeline::render(Engine::priv::Renderer& renderer, const Viewport& viewport, const bool mainRenderFunction) {
    const auto& camera = viewport.getCamera();
    const auto& scene  = viewport.getScene();

    internal_render_per_frame_preparation(viewport, camera);

    if (mainRenderFunction) {
        #pragma region Camera UBO
        if (Renderer::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA) {
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


            //this render space places the camera at the origin and offsets submitted model matrices to the vertex shaders
            //by the camera's real simulation position
            //this helps to deal with shading inaccuracies for when the camera is very far away from the origin
            m_UBOCameraDataStruct.View        = ComponentCamera_Functions::GetViewNoTranslation(camera);
            m_UBOCameraDataStruct.Proj        = camera.getProjection();
            m_UBOCameraDataStruct.ViewProj    = ComponentCamera_Functions::GetViewProjectionNoTranslation(camera);
            m_UBOCameraDataStruct.InvProj     = camera.getProjectionInverse();
            m_UBOCameraDataStruct.InvView     = ComponentCamera_Functions::GetViewInverseNoTranslation(camera);
            m_UBOCameraDataStruct.InvViewProj = ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(camera);
            m_UBOCameraDataStruct.Info1       = glm::vec4(0.0001f, 0.0001f, 0.0001f, camera.getNear());
            m_UBOCameraDataStruct.Info2       = glm::vec4(ComponentCamera_Functions::GetViewVectorNoTranslation(camera), camera.getFar());
            m_UBOCameraDataStruct.Info3       = glm::vec4(camera.getPosition(), 0.0f);

            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraDataStruct);
        }
        #pragma endregion
    }

    internal_pass_geometry(viewport, camera);

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_ssao(viewport, camera);

    Engine::Renderer::GLDisablei(GL_BLEND, 0);

    internal_pass_stencil();

    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    //this needs to be cleaned up
    m_GBuffer.bindFramebuffers(GBufferType::Lighting, "RGB");
    Engine::Renderer::Settings::clear(true, false, false);//lighting rgb channels cleared to black

    if(m_Renderer.m_Lighting){
        internal_pass_lighting(viewport, camera, mainRenderFunction);
    }
    Engine::Renderer::GLDisablei(GL_BLEND, 0);
    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);


    internal_pass_forward(viewport, camera);

    Engine::Renderer::GLDisable(GL_DEPTH_TEST);

    internal_pass_god_rays(viewport, camera);

    internal_pass_hdr(viewport, camera);

    internal_pass_bloom(viewport);

    GBufferType::Type sceneTexture = GBufferType::Misc;
    GBufferType::Type outTexture   = GBufferType::Lighting;

    internal_pass_depth_of_field(viewport, sceneTexture, outTexture);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    internal_pass_aa(mainRenderFunction, viewport, camera, sceneTexture, outTexture);

    internal_pass_copy_depth();

    #pragma region RenderPhysics
    Engine::Renderer::GLEnablei(GL_BLEND, 0);
    if (mainRenderFunction && (viewport.getRenderFlags() & ViewportRenderingFlag::PhysicsDebug)) {
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
            if (m_Renderer.m_DrawPhysicsDebug && &camera == scene.getActiveCamera()) {
        #endif
                Engine::Renderer::GLDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                m_InternalShaderPrograms[ShaderProgramEnum::BulletPhysics]->bind();
                Core::m_Engine->m_PhysicsManager._render(camera);
        #ifndef ENGINE_FORCE_PHYSICS_DEBUG_DRAW
            }
        #endif
    }
    #pragma endregion

    #pragma region 2DAPI
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    if (mainRenderFunction) {
        if (viewport.getRenderFlags() & ViewportRenderingFlag::API2D) {
            Engine::Renderer::Settings::clear(false, true, false); //clear depth only
            m_InternalShaderPrograms[ShaderProgramEnum::Deferred2DAPI]->bind();
            Engine::Renderer::sendUniformMatrix4("VP", m_2DProjectionMatrix);
            Engine::Renderer::sendUniform1Safe("ScreenGamma", m_Renderer.m_Gamma);
            Engine::Renderer::GLEnable(GL_SCISSOR_TEST);
            for (auto& command : m_2DAPICommands) {
                command.func();
            }
            Engine::Renderer::GLDisable(GL_SCISSOR_TEST);
        }
    }
    #pragma endregion
}



void DeferredPipeline::renderTexture(const Texture& tex, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const Alignment::Type& align, const glm::vec4& scissor) {
    API2DCommand command;
    command.func = [=, &tex]() { DeferredPipeline::render2DTexture(&tex, p, c, a, s, d, align, scissor); };
    command.depth = d;
    m_2DAPICommands.push_back(std::move(command));
}
void DeferredPipeline::renderText(const string& t, const Font& fnt, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type& align, const glm::vec4& scissor) {
    API2DCommand command;
    command.func = [=, &fnt]() { DeferredPipeline::render2DText(t, fnt, p, c, a, s, d, align, scissor); };
    command.depth = d;
    m_2DAPICommands.push_back(std::move(command));
}
void DeferredPipeline::renderBorder(const float borderSize, const glm::vec2& pos, const glm::vec4& col, const float w, const float h, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    const float doubleBorder = borderSize * 2.0f;
    const float halfWidth    = w / 2.0f;
    const float halfHeight   = h / 2.0f;

    float translationX       = pos.x;
    float translationY       = pos.y;
    Engine::Renderer::alignmentOffset(align, translationX, translationY, w, h);
    glm::vec2 newPos(translationX, translationY);

    Engine::Renderer::renderRectangle(newPos - glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth, Alignment::Right, scissor);
    Engine::Renderer::renderRectangle(newPos + glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth, Alignment::Left, scissor);
    Engine::Renderer::renderRectangle(newPos - glm::vec2(0.0f, halfHeight), col, w, borderSize, angle, depth, Alignment::TopCenter, scissor);
    Engine::Renderer::renderRectangle(newPos + glm::vec2(0.0f, halfHeight + borderSize), col, w, borderSize, angle, depth, Alignment::BottomCenter, scissor);
}
void DeferredPipeline::renderRectangle(const glm::vec2& pos, const glm::vec4& col, const float width, const float height, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    API2DCommand command;
    command.func = [=]() { DeferredPipeline::render2DTexture(nullptr, pos, col, angle, glm::vec2(width, height), depth, align, scissor); };
    command.depth = depth;
    m_2DAPICommands.push_back(std::move(command));
}
void DeferredPipeline::renderTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    API2DCommand command;
    command.func = [=]() { DeferredPipeline::render2DTriangle(position, color, angle, width, height, depth, align, scissor); };
    command.depth = depth;
    m_2DAPICommands.push_back(std::move(command));
}

void DeferredPipeline::renderFullscreenTriangle() {
    const auto& winSize = glm::vec2(Resources::getWindowSize());

    const glm::mat4 p = glm::ortho(0.0f, winSize.x, 0.0f, winSize.y);
    Engine::Renderer::sendUniformMatrix4("Model", glm::mat4(1.0f));
    Engine::Renderer::sendUniformMatrix4Safe("VP", p);
    Engine::Renderer::sendUniform2Safe("screenSizeDivideBy2", 1.0f, 1.0f);
    m_FullscreenTriangle.render();
}
void DeferredPipeline::renderFullscreenQuad() {
    const auto& winSize = glm::vec2(Resources::getWindowSize());

    const glm::mat4 p = glm::ortho(0.0f, winSize.x, 0.0f, winSize.y);
    Engine::Renderer::sendUniformMatrix4("Model", glm::mat4(1.0f));
    Engine::Renderer::sendUniformMatrix4Safe("VP", p);
    Engine::Renderer::sendUniform2Safe("screenSizeDivideBy2", 1.0f, 1.0f);
    m_FullscreenQuad.render();
}