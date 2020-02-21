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
#include <core/engine/renderer/FullscreenItems.h>

#include <boost/math/special_functions/fpclassify.hpp>


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




DeferredPipeline::DeferredPipeline() {
    const auto window_size = Resources::getWindowSize();
    m_2DProjectionMatrix   = glm::ortho(0.0f, static_cast<float>(window_size.x), 0.0f, static_cast<float>(window_size.y), 0.005f, 3000.0f);
    m_UBOCamera            = nullptr;
    pipeline               = this;
}
DeferredPipeline::~DeferredPipeline() {

}

void DeferredPipeline::init() {
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS);

    float init_border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, init_border_color);

    pipeline->OpenGLExtensionsManager.INIT();


    pipeline->OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
    pipeline->OpenGLStateMachine.GL_glDisable(GL_STENCIL_TEST);
    pipeline->OpenGLStateMachine.GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for non Power of Two textures
    //pipeline->OpenGLStateMachine.GL_glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //very odd, supported on my gpu and opengl version but it runs REAL slowly, dropping fps to 1
    pipeline->OpenGLStateMachine.GL_glEnable(GL_DEPTH_CLAMP);
    Engine::Renderer::setDepthFunc(GL_LEQUAL);

    priv::EShaders::init();

    m_UBOCamera = NEW UniformBufferObject("Camera", sizeof(UBOCameraDataStruct));
    m_UBOCamera->updateData(&m_UBOCameraDataStruct);


    m_FullscreenQuad = NEW FullscreenQuad();
    m_FullscreenTriangle = NEW FullscreenTriangle();

    m_InternalShaders.resize(ShaderEnum::_TOTAL, nullptr);
    m_InternalShaderPrograms.resize(ShaderProgramEnum::_TOTAL, nullptr);

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

    /*
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

#pragma region MeshData


#pragma endregion


    sf::Image sfImageWhite;
    sfImageWhite.create(2, 2, sf::Color::White);
    sf::Image sfImageBlack;
    sfImageBlack.create(2, 2, sf::Color::Black);

    sf::Image sfImageCheckers;
    sfImageCheckers.create(8, 8, sf::Color::White);

    uint count = 0;
    for (uint i = 0; i < sfImageCheckers.getSize().x; ++i) {
        for (uint j = 0; j < sfImageCheckers.getSize().y; ++j) {
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

    */
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

void DeferredPipeline::onFullscreen() {
    //TODO: move these lines to a more generic area, all rendering pipelines will pretty much do this
    restoreCurrentOpenGLState();

    GLEnable(GL_CULL_FACE);
    GLEnable(GL_DEPTH_CLAMP);

    const auto winSize = Resources::getWindowSize();
    m_GBuffer.init(winSize.x, winSize.y);
}
//DONE
void DeferredPipeline::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(newWidth), 0.0f, static_cast<float>(newHeight), 0.005f, 3000.0f);
    m_GBuffer.resize(newWidth, newHeight);
}
void DeferredPipeline::onOpenGLContextCreation() {
    const auto winSize = Resources::getWindowSize();

    //TODO: move to a more generic area
    //epriv::RenderManager::GLSL_VERSION = _glslVersion;
    //epriv::RenderManager::OPENGL_VERSION = _openglVersion;

    GLEnable(GL_CULL_FACE);
    m_GBuffer.init(winSize.x, winSize.y);
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
//DONE
void DeferredPipeline::renderParticle(const Particle& particle) {
    particle.getMaterial()->bind();

    auto maxTextures = priv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;

    Camera& camera = *particle.scene().getActiveCamera();
    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(Engine::priv::GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform4Safe("Object_Color", particle.color());
    Engine::Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, particle.position());
    modelMatrix *= glm::mat4_cast(camera.getOrientation());
    modelMatrix = glm::rotate(modelMatrix, particle.angle(), glm::vec3(0, 0, 1));
    const auto& scale = particle.getScale();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));

    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

    priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().render();
}
//DONE
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
//DONE
void DeferredPipeline::internal_render_2d_text_left(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    unsigned int i = 0;
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (auto& character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight;
            x = 0.0f;
        }else if (character != '\0') {
            accumulator = i * 4;
            ++i;
            const FontGlyph& chr = font.getGlyphData(character);
            startingY = -int(chr.height + chr.yoffset) - y;

            m_Text_Indices.emplace_back(accumulator + 0);
            m_Text_Indices.emplace_back(accumulator + 1);
            m_Text_Indices.emplace_back(accumulator + 2);
            m_Text_Indices.emplace_back(accumulator + 3);
            m_Text_Indices.emplace_back(accumulator + 1);
            m_Text_Indices.emplace_back(accumulator + 0);

            startingX = x + chr.xoffset;
            x        += chr.xadvance;

            for (unsigned char i = 0; i < 4; ++i)
                m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

            for (unsigned char i = 0; i < 4; ++i)
                m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
        }
    }
}
//DONE
void DeferredPipeline::internal_render_2d_text_center(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string>          lines;
    vector<unsigned short>  lines_sizes;
    string                  line_accumulator = "";
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
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (uint l = 0; l < lines.size(); ++l) {
        const auto& line = lines[l];
        const auto& line_size = lines_sizes[l] / 2;
        for (auto& character : line) {
            if (character != '\0') {
                accumulator = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accumulator + 0);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 2);
                m_Text_Indices.emplace_back(accumulator + 3);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 0);

                startingX = x + chr.xoffset;
                x += chr.xadvance;

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x - line_size, startingY + chr.pts[i].y, z);

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}
//DONE
void DeferredPipeline::internal_render_2d_text_right(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string> lines;
    string line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            line_accumulator = "";
            continue;
        }
        else if (character != '\0') {
            line_accumulator += character;
        }
    }
    if (lines.size() == 0)
        lines.push_back(line_accumulator);

    unsigned int i = 0;
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (auto& line : lines) {
        const int line_size = static_cast<int>(line.size());
        int k = 0;
        for (int j = line_size; j >= 0; --j) {
            const auto& character = line[j];
            if (character != '\0') {
                accumulator = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accumulator + 0);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 2);
                m_Text_Indices.emplace_back(accumulator + 3);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 0);

                if (k == 0) {
                    x -= chr.width;
                }
                startingX = x + chr.xoffset;
                x -= chr.xadvance;

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
                ++k;
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}

//DONE
void DeferredPipeline::render2DText(const string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    auto& fontPlane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();
    fontPlane.bind();

    const auto  newLineGlyphHeight = font.getGlyphData('X').height;
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
//DONE
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
//DONE
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


void DeferredPipeline::internal_render_per_frame_preparation() {

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
    InternalScenePublicInterface::RenderGeometryOpaque(scene, viewport, camera);
    if ((viewport.getRenderFlags() & ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), *m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    InternalScenePublicInterface::RenderGeometryTransparent(scene, viewport, camera);
    InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(scene, viewport, camera, true);
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
            for (uint i = 0; i < SSAO::ssao.m_ssao_blur_num_passes; ++i) {
                m_GBuffer.bindFramebuffers(GBufferType::GodRays, "A", false);
                SSAO::ssao.passBlur(m_GBuffer, viewport, "H", GBufferType::Bloom);
                m_GBuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
                SSAO::ssao.passBlur(m_GBuffer, viewport, "V", GBufferType::GodRays);
            }
        }
    }
}
void DeferredPipeline::internal_pass_stencil() {

}
void DeferredPipeline::internal_pass_lighting() {

}
void DeferredPipeline::internal_pass_forward() {

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
        const bool infront = Engine::Math::isPointWithinCone(camPos, -camVec, oPos, Engine::Math::toRadians(godRaysPlatform.fovDegrees));
        if (infront) {
            const auto sp = Engine::Math::getScreenCoordinates(oPos, camera, false);
            const auto b = glm::normalize(camPos - oPos);
            float alpha = Engine::Math::getAngleBetweenTwoVectors(camVec, b, true) / godRaysPlatform.fovDegrees;
            alpha = glm::pow(alpha, godRaysPlatform.alphaFalloff);
            alpha = glm::clamp(alpha, 0.01f, 0.99f);
            if (boost::math::isnan(alpha) || boost::math::isinf(alpha)) { //yes this is needed...
                alpha = 0.01f;
            }
            alpha = 1.0f - alpha;
            godRaysPlatform.pass(m_GBuffer, viewport, glm::vec2(sp.x, sp.y), alpha);
        }
    }
}
void DeferredPipeline::internal_pass_hdr(const Viewport& viewport, const Camera& camera) {
    const glm::uvec4& dimensions = viewport.getViewportDimensions();
    m_GBuffer.bindFramebuffers(GBufferType::Misc);
    //HDR::hdr.pass(*m_GBuffer, dimensions.z, dimensions.w, GodRays::godRays.godRays_active, lighting, GodRays::godRays.factor);
}
void DeferredPipeline::internal_pass_bloom() {

}
void DeferredPipeline::internal_pass_depth_of_field() {

}
void DeferredPipeline::internal_pass_aa() {

}
void DeferredPipeline::internal_pass_final() {

}

void DeferredPipeline::internal_pass_depth_and_transparency() {

}
void DeferredPipeline::internal_pass_copy_depth() {

}
void DeferredPipeline::internal_pass_blur() {

}



void DeferredPipeline::update(const float& dt) {

}
void DeferredPipeline::render(const Viewport& viewport) {
    const auto& camera = viewport.getCamera();

    internal_render_per_frame_preparation();
    internal_pass_geometry(viewport, camera);
    internal_pass_ssao(viewport, camera);
    internal_pass_stencil();
    internal_pass_lighting();
    internal_pass_forward();
    internal_pass_god_rays(viewport, camera);
    internal_pass_hdr(viewport, camera);
    internal_pass_bloom();
    internal_pass_depth_of_field();
    internal_pass_aa();
    internal_pass_final();

    internal_pass_depth_and_transparency(); //TODO: recheck this
    internal_pass_copy_depth();
    //internal_pass_blur();
}