#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/renderer/FullscreenItems.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/materials/Material.h>
#include <ecs/ComponentBody.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/renderer/Decal.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include <queue>

#include <iostream>
#include <execution>
#include <functional>

using namespace Engine;
using namespace std;

priv::Renderer* renderManager;
priv::Renderer::impl* renderManagerImpl;

uint priv::Renderer::GLSL_VERSION;
uint priv::Renderer::OPENGL_VERSION;

namespace Engine{
    namespace priv{

        struct API2DCommand {
            std::function<void()>  func;
            float                  depth;
            glm::vec4              scissor;
        };

        struct srtKeyShaderProgram final{inline bool operator() ( ShaderProgram* _1,  ShaderProgram* _2){return (_1->name() < _2->name());}};

        struct EngineInternalShaders final{enum Shader{
            DecalVertex,
            DecalFrag,
            FullscreenVertex,
            FXAAFrag,
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
            SSAOFrag,
            BloomFrag,
            HDRFrag,
            BlurFrag,
            DOFFrag,
            SSAOBlurFrag,
            GodRaysFrag,
            FinalFrag,
            DepthAndTransparencyFrag,
            LightingFrag,
            LightingGIFrag,
            CubemapConvoludeFrag,
            CubemapPrefilterEnvFrag,
            BRDFPrecomputeFrag,
            GrayscaleFrag,
            StencilPassFrag,
            SMAAVertex1,
            SMAAVertex2,
            SMAAVertex3,
            SMAAVertex4,
            SMAAFrag1,
            SMAAFrag2,
            SMAAFrag3,
            SMAAFrag4,
            ParticleVertex,
            ParticleFrag,
        _TOTAL};};
        struct EngineInternalShaderPrograms final{enum Program{
            BulletPhysics,
            ZPrepass,
            Deferred2DAPI,
            DeferredGodRays,
            DeferredBlur,
            DeferredBlurSSAO,
            DeferredHDR,
            DeferredSSAO,
            DeferredDOF,
            DeferredBloom,
            DeferredFinal,
            DepthAndTransparency,
            DeferredFXAA,
            DeferredSkybox,
            CopyDepth,
            DeferredLighting,
            DeferredLightingGI,
            CubemapConvolude,
            CubemapPrefilterEnv,
            BRDFPrecomputeCookTorrance,
            Grayscale,
            StencilPass,
            SMAA1,
            SMAA2,
            SMAA3,
            SMAA4,
            Particle,
        _TOTAL};};

        struct UBOCamera final{
            glm::mat4 View;
            glm::mat4 Proj;
            glm::mat4 ViewProj;
            glm::mat4 InvView;
            glm::mat4 InvProj;
            glm::mat4 InvViewProj;

            glm::vec4 Info1; //renderPosX,renderPosY,renderPosZ,near
            glm::vec4 Info2; //viewVecX,viewVecY,viewVecZ,far
            glm::vec4 Info3; //realposX,realposY,realposZ,UNUSED
        };
    };
};

void emplaceShader(const unsigned int index, string& str, vector<Shader*>& collection, const ShaderType::Type type) {
    Shader* s = NEW Shader(str, type, false);
    collection[index] = s;
}

class priv::Renderer::impl final{
    public:
        #pragma region LightingInfo
        bool  lighting;
        float lighting_gi_contribution_diffuse;
        float lighting_gi_contribution_specular;
        float lighting_gi_contribution_global;
        float lighting_gi_pack;
        #pragma endregion

        #pragma region GeneralInfo

        float gamma;
        AntiAliasingAlgorithm::Algorithm aa_algorithm;
        DepthFunc::Func depth_func;
        bool draw_physics_debug;

        GBuffer* m_GBuffer;
        glm::mat4 m_2DProjectionMatrix;

        vector<API2DCommand> m_2DAPICommands;

        vector<glm::vec3>   text_pts;
        vector<glm::vec2>   text_uvs;
        vector<ushort>      text_ind;

        glm::vec3           m_RotationAxis2D;
        glm::mat4           m_IdentityMat4;
        glm::mat3           m_IdentityMat3;
        FullscreenQuad*     m_FullscreenQuad;
        FullscreenTriangle* m_FullscreenTriangle;
        #pragma endregion

        #pragma region EngineInternalShadersAndPrograms
        vector<Shader*> m_InternalShaders;
        vector<ShaderProgram*> m_InternalShaderPrograms;

        priv::UBOCamera m_UBOCameraData;
        #pragma endregion

        void _init(const EngineOptions& options){
            #pragma region LightingInfo
            lighting = true;
            lighting_gi_contribution_diffuse = 1.0f;
            lighting_gi_contribution_specular = 1.0f;
            lighting_gi_contribution_global = 1.0f;
            lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(lighting_gi_contribution_diffuse,lighting_gi_contribution_specular,lighting_gi_contribution_global);
            #pragma endregion

            #pragma region GeneralInfo

            gamma = 2.2f;
            aa_algorithm = AntiAliasingAlgorithm::FXAA;
            depth_func = DepthFunc::Less;
            #ifdef _DEBUG
                draw_physics_debug = true;
            #else
                draw_physics_debug = false;
            #endif

            m_GBuffer = nullptr;
            m_2DProjectionMatrix = glm::ortho(0.0f,static_cast<float>(options.width),0.0f, static_cast<float>(options.height),0.0005f,1000.0f);
            m_IdentityMat4 = glm::mat4(1.0f);
            m_IdentityMat3 = glm::mat3(1.0f);
            m_RotationAxis2D = glm::vec3(0, 0, 1);
            #pragma endregion
        }      
        void _postInit() {
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS);

            float init_border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, init_border_color);

#pragma region OpenGLExtensions

            renderManager->OpenGLExtensionsManager.INIT();

#pragma endregion

            priv::EShaders::init();

#pragma region EngineInternalShaderUBOs
            UniformBufferObject::UBO_CAMERA = NEW UniformBufferObject("Camera", sizeof(priv::UBOCamera));
            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
#pragma endregion

#pragma region EngineInternalShadersAndPrograms
            m_InternalShaders.resize(EngineInternalShaders::_TOTAL, nullptr);
            m_InternalShaderPrograms.resize(EngineInternalShaderPrograms::_TOTAL, nullptr);

            priv::threading::addJob(emplaceShader, 0, boost::ref(EShaders::decal_vertex), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 1, boost::ref(EShaders::decal_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 2, boost::ref(EShaders::fullscreen_quad_vertex), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 3, boost::ref(EShaders::fxaa_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 4, boost::ref(EShaders::bullet_physics_vert), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 5, boost::ref(EShaders::bullet_physcis_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 6, boost::ref(EShaders::vertex_basic), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 7, boost::ref(EShaders::vertex_2DAPI), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 8, boost::ref(EShaders::vertex_skybox), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 9, boost::ref(EShaders::lighting_vert), boost::ref(m_InternalShaders), ShaderType::Vertex);

            priv::threading::addJob(emplaceShader, 10, boost::ref(EShaders::forward_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 11, boost::ref(EShaders::deferred_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 12, boost::ref(EShaders::zprepass_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 13, boost::ref(EShaders::deferred_frag_hud), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 14, boost::ref(EShaders::deferred_frag_skybox), boost::ref(m_InternalShaders), ShaderType::Fragment);

            priv::threading::addJob(emplaceShader, 15, boost::ref(EShaders::copy_depth_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 16, boost::ref(EShaders::ssao_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 17, boost::ref(EShaders::bloom_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 18, boost::ref(EShaders::hdr_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 19, boost::ref(EShaders::blur_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);

            priv::threading::addJob(emplaceShader, 20, boost::ref(EShaders::depth_of_field), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 21, boost::ref(EShaders::ssao_blur_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 22, boost::ref(EShaders::godRays_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 23, boost::ref(EShaders::final_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 24, boost::ref(EShaders::depth_and_transparency_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);

            priv::threading::addJob(emplaceShader, 25, boost::ref(EShaders::lighting_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 26, boost::ref(EShaders::lighting_frag_gi), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 27, boost::ref(EShaders::cubemap_convolude_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 28, boost::ref(EShaders::cubemap_prefilter_envmap_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 29, boost::ref(EShaders::brdf_precompute), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 30, boost::ref(EShaders::greyscale_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 31, boost::ref(EShaders::stencil_passover), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 32, boost::ref(EShaders::smaa_vertex_1), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 33, boost::ref(EShaders::smaa_vertex_2), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 34, boost::ref(EShaders::smaa_vertex_3), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 35, boost::ref(EShaders::smaa_vertex_4), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 36, boost::ref(EShaders::smaa_frag_1), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 37, boost::ref(EShaders::smaa_frag_2), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 38, boost::ref(EShaders::smaa_frag_3), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 39, boost::ref(EShaders::smaa_frag_4), boost::ref(m_InternalShaders), ShaderType::Fragment);
            priv::threading::addJob(emplaceShader, 40, boost::ref(EShaders::particle_vertex), boost::ref(m_InternalShaders), ShaderType::Vertex);
            priv::threading::addJob(emplaceShader, 41, boost::ref(EShaders::particle_frag), boost::ref(m_InternalShaders), ShaderType::Fragment);


            priv::threading::waitForAll();

            ShaderProgram::Deferred = NEW ShaderProgram("Deferred", *m_InternalShaders[EngineInternalShaders::VertexBasic], *m_InternalShaders[EngineInternalShaders::DeferredFrag]);
            ShaderProgram::Forward = NEW ShaderProgram("Forward", *m_InternalShaders[EngineInternalShaders::VertexBasic], *m_InternalShaders[EngineInternalShaders::ForwardFrag]);
            ShaderProgram::Decal = NEW ShaderProgram("Decal", *m_InternalShaders[EngineInternalShaders::DecalVertex], *m_InternalShaders[EngineInternalShaders::DecalFrag]);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics] = NEW ShaderProgram("Bullet_Physics", *m_InternalShaders[EngineInternalShaders::BulletPhysicsVertex], *m_InternalShaders[EngineInternalShaders::BulletPhysicsFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::ZPrepass] = NEW ShaderProgram("ZPrepass", *m_InternalShaders[EngineInternalShaders::VertexBasic], *m_InternalShaders[EngineInternalShaders::ZPrepassFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Deferred2DAPI] = NEW ShaderProgram("Deferred_2DAPI", *m_InternalShaders[EngineInternalShaders::Vertex2DAPI], *m_InternalShaders[EngineInternalShaders::DeferredFrag2DAPI]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredGodRays] = NEW ShaderProgram("Deferred_GodsRays", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::GodRaysFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur] = NEW ShaderProgram("Deferred_Blur", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::BlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlurSSAO] = NEW ShaderProgram("Deferred_Blur_SSAO", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::SSAOBlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHDR] = NEW ShaderProgram("Deferred_HDR", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::HDRFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO] = NEW ShaderProgram("Deferred_SSAO", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::SSAOFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF] = NEW ShaderProgram("Deferred_DOF", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::DOFFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBloom] = NEW ShaderProgram("Deferred_Bloom", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::BloomFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal] = NEW ShaderProgram("Deferred_Final", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::FinalFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DepthAndTransparency] = NEW ShaderProgram("DepthAndTransparency", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::DepthAndTransparencyFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA] = NEW ShaderProgram("Deferred_FXAA", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::FXAAFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox] = NEW ShaderProgram("Deferred_Skybox", *m_InternalShaders[EngineInternalShaders::VertexSkybox], *m_InternalShaders[EngineInternalShaders::DeferredFragSkybox]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth] = NEW ShaderProgram("Copy_Depth", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::CopyDepthFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting] = NEW ShaderProgram("Deferred_Light", *m_InternalShaders[EngineInternalShaders::LightingVertex], *m_InternalShaders[EngineInternalShaders::LightingFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI] = NEW ShaderProgram("Deferred_Light_GI", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::LightingGIFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude] = NEW ShaderProgram("Cubemap_Convolude", *m_InternalShaders[EngineInternalShaders::VertexSkybox], *m_InternalShaders[EngineInternalShaders::CubemapConvoludeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv] = NEW ShaderProgram("Cubemap_Prefilter_Env", *m_InternalShaders[EngineInternalShaders::VertexSkybox], *m_InternalShaders[EngineInternalShaders::CubemapPrefilterEnvFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance] = NEW ShaderProgram("BRDF_Precompute_CookTorrance", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::BRDFPrecomputeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Grayscale] = NEW ShaderProgram("Greyscale_Frag", *m_InternalShaders[EngineInternalShaders::FullscreenVertex], *m_InternalShaders[EngineInternalShaders::GrayscaleFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass] = NEW ShaderProgram("Stencil_Pass", *m_InternalShaders[EngineInternalShaders::LightingVertex], *m_InternalShaders[EngineInternalShaders::StencilPassFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA1] = NEW ShaderProgram("Deferred_SMAA_1", *m_InternalShaders[EngineInternalShaders::SMAAVertex1], *m_InternalShaders[EngineInternalShaders::SMAAFrag1]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA2] = NEW ShaderProgram("Deferred_SMAA_2", *m_InternalShaders[EngineInternalShaders::SMAAVertex2], *m_InternalShaders[EngineInternalShaders::SMAAFrag2]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA3] = NEW ShaderProgram("Deferred_SMAA_3", *m_InternalShaders[EngineInternalShaders::SMAAVertex3], *m_InternalShaders[EngineInternalShaders::SMAAFrag3]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA4] = NEW ShaderProgram("Deferred_SMAA_4", *m_InternalShaders[EngineInternalShaders::SMAAVertex4], *m_InternalShaders[EngineInternalShaders::SMAAFrag4]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Particle] = NEW ShaderProgram("Particle", *m_InternalShaders[EngineInternalShaders::ParticleVertex], *m_InternalShaders[EngineInternalShaders::ParticleFrag]);
#pragma endregion

#pragma region MeshData


#pragma endregion


            text_pts.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 points per char, 4096 chars
            text_uvs.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 uvs per char
            text_ind.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6);//6 ind per char

            for (size_t i = 0; i < text_pts.capacity(); ++i)
                text_pts.emplace_back(0.0f);
            for (size_t i = 0; i < text_uvs.capacity(); ++i)
                text_uvs.emplace_back(0.0f);
            for (size_t i = 0; i < text_ind.capacity(); ++i)
                text_ind.emplace_back(0);

            auto& fontPlane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();

            fontPlane.modifyVertices(0, text_pts, MeshModifyFlags::Default);
            fontPlane.modifyVertices(1, text_uvs, MeshModifyFlags::Default);
            fontPlane.modifyIndices(text_ind, MeshModifyFlags::Default);

            text_pts.clear();
            text_uvs.clear();
            text_ind.clear();

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
            Texture::White    = NEW Texture(sfImageWhite,    "WhiteTexturePlaceholder",    false, ImageInternalFormat::RGBA8);
            Texture::Black    = NEW Texture(sfImageBlack,    "BlackTexturePlaceholder",    false, ImageInternalFormat::RGBA8);
            Texture::Checkers = NEW Texture(sfImageCheckers, "CheckersTexturePlaceholder", false, ImageInternalFormat::RGBA8);
            Texture::Checkers->setFilter(TextureFilter::Nearest);
            Material::Checkers = NEW Material("MaterialDefaultCheckers", Texture::Checkers);
            Material::Checkers->setSpecularModel(SpecularModel::None);
            Material::Checkers->setSmoothness(0.0f);

            Material::WhiteShadeless = NEW Material("MaterialDefaultWhiteShadeless", Texture::White);
            Material::WhiteShadeless->setSpecularModel(SpecularModel::None);
            Material::WhiteShadeless->setSmoothness(0.0f);
            Material::WhiteShadeless->setShadeless(true);

            Texture::BRDF = NEW Texture(512,512,ImagePixelType::FLOAT,ImagePixelFormat::RG,ImageInternalFormat::RG16F);
            Texture::BRDF->setWrapping(TextureWrap::ClampToEdge);

            m_FullscreenQuad = NEW FullscreenQuad();
            m_FullscreenTriangle = NEW FullscreenTriangle();

            SSAO::ssao.init();

            renderManager->OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
            Engine::Renderer::setDepthFunc(GL_LEQUAL);
            renderManager->OpenGLStateMachine.GL_glDisable(GL_STENCIL_TEST);
            renderManager->OpenGLStateMachine.GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for non Power of Two textures
            //renderManager->OpenGLStateMachine.GL_glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //very odd, supported on my gpu and opengl version but it runs REAL slowly, dropping fps to 1
            renderManager->OpenGLStateMachine.GL_glEnable(GL_DEPTH_CLAMP);

            SMAA::smaa.init();

            _generateBRDFLUTCookTorrance(512);
        }
        void _destruct(){
            SAFE_DELETE(UniformBufferObject::UBO_CAMERA);

            SAFE_DELETE(m_GBuffer);
            SAFE_DELETE(m_FullscreenQuad);
            SAFE_DELETE(m_FullscreenTriangle);

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
        void _renderSkybox(Skybox* skybox, Scene& scene, Viewport& viewport, Camera& camera){
            glm::mat4 view = camera.getView();
            Math::removeMatrixPosition(view);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox]->bind();
            if (skybox) {
                Engine::Renderer::sendUniform1("IsFake", 0);
                Engine::Renderer::sendTextureSafe("Texture", skybox->texture()->address(0), 0, GL_TEXTURE_CUBE_MAP);
            }else{
                Engine::Renderer::sendUniform1("IsFake", 1);
                const auto& bgColor = scene.getBackgroundColor();
                Engine::Renderer::sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            }
            Engine::Renderer::sendUniformMatrix4("VP", camera.getProjection() * view);
            Skybox::bindMesh();

            Engine::Renderer::sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
            //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...

        }
        void _resize(const uint& w, const uint& h){
            m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h), 0.005f, 3000.0f);
            m_GBuffer->resize(w, h);
        }

        void _onOpenGLContextCreation(const uint& width, const uint& height, const uint& _glslVersion, const uint& _openglVersion){
            priv::Renderer::GLSL_VERSION = _glslVersion;
            priv::Renderer::OPENGL_VERSION = _openglVersion;
            Engine::Renderer::GLEnable(GL_CULL_FACE);
            SAFE_DELETE(m_GBuffer);
            m_GBuffer = NEW GBuffer(width,height);
        }
        void _generatePBREnvMapData(Texture& texture, const uint& convoludeTextureSize, const uint& preEnvFilterSize){
            uint texType = texture.type();
            if(texType != GL_TEXTURE_CUBE_MAP){
                cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
            }
            uint size = convoludeTextureSize;
            Engine::Renderer::bindTextureForModification(texType, texture.address(1));
            Engine::Renderer::unbindFBO();
            priv::FramebufferObject* fbo = NEW priv::FramebufferObject(texture.name() + "_fbo_envData",size,size); //try without a depth format
            fbo->bind();
    
            //make these 2 variables global in the renderer class?
            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f),1.0f,0.1f,3000000.0f);
            glm::mat4 captureViews[] = {
                glm::lookAt(glm::vec3(0.0f),glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
            };
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude]->bind();

            Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType);
            Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(size), static_cast<float>(size));
            for (uint i = 0; i < 6; ++i){
                const glm::mat4 vp = captureProjection * captureViews[i];
                Engine::Renderer::sendUniformMatrix4("VP",vp);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(1),0);
                Engine::Renderer::Settings::clear(true,true,false);
                Skybox::bindMesh();
            }
            Resources::getWindow().display(); //prevent opengl & windows timeout


            //now gen EnvPrefilterMap for specular IBL
            size = preEnvFilterSize;
            Engine::Renderer::bindTextureForModification(texType, texture.address(2));
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv]->bind();
            Engine::Renderer::sendTexture("cubemap",texture.address(),0,texType);
            Engine::Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix",12.56637f / float((texture.width() * texture.width())*6));
            Engine::Renderer::sendUniform1("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
                fbo->resize(mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                Engine::Renderer::sendUniform1("roughness",roughness);
                float a = roughness * roughness;
                Engine::Renderer::sendUniform1("a2",a*a);
                for (uint i = 0; i < 6; ++i){
                    glm::mat4 vp = captureProjection * captureViews[i];
                    Engine::Renderer::sendUniformMatrix4("VP", vp);
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(2),m);
                    Engine::Renderer::Settings::clear(true,true,false);
                    Skybox::bindMesh();
                }
            }
            Resources::getWindow().display(); //prevent opengl & windows timeout
            fbo->unbind();
            SAFE_DELETE(fbo);
        }
        void _generateBRDFLUTCookTorrance(const uint& brdfSize){
            //uint& prevReadBuffer = renderManager->glSM.current_bound_read_fbo;
            //uint& prevDrawBuffer = renderManager->glSM.current_bound_draw_fbo;

            FramebufferObject* fbo = NEW FramebufferObject("BRDFLUT_Gen_CookTorr_FBO", brdfSize, brdfSize); //try without a depth format
            fbo->bind();

            Engine::Renderer::bindTextureForModification(GL_TEXTURE_2D, Texture::BRDF->address());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
            Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToEdge);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture::BRDF->address(), 0);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance]->bind();
            Engine::Renderer::sendUniform1("NUM_SAMPLES", 256);
            Engine::Renderer::Settings::clear(true, true, false);
            Engine::Renderer::colorMask(true, true, false, false);
            Engine::Renderer::renderFullscreenTriangle(brdfSize, brdfSize);
            Engine::Renderer::colorMask(true, true, true, true);

            SAFE_DELETE(fbo);
            //Renderer::bindReadFBO(prevReadBuffer);
            //Renderer::bindDrawFBO(prevDrawBuffer);
        }
        void _renderTextLeft(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
            unsigned int i = 0;
            for (auto& character : text) {
                if (character == '\n') {
                    y += newLineGlyphHeight;
                    x = 0.0f;
                }else if (character != '\0') {
                    const unsigned int accum = i * 4;
                    ++i;
                    const FontGlyph& chr   = font.getGlyphData(character);
                    const float startingY = -int(chr.height + chr.yoffset) - y;

                    text_ind.emplace_back(accum + 0);
                    text_ind.emplace_back(accum + 1);
                    text_ind.emplace_back(accum + 2);
                    text_ind.emplace_back(accum + 3);
                    text_ind.emplace_back(accum + 1);
                    text_ind.emplace_back(accum + 0);

                    const float startingX = x + chr.xoffset;
                    x += chr.xadvance;

                    text_pts.emplace_back(startingX + chr.pts[0].x, startingY + chr.pts[0].y, z);
                    text_pts.emplace_back(startingX + chr.pts[1].x, startingY + chr.pts[1].y, z);
                    text_pts.emplace_back(startingX + chr.pts[2].x, startingY + chr.pts[2].y, z);
                    text_pts.emplace_back(startingX + chr.pts[3].x, startingY + chr.pts[3].y, z);

                    text_uvs.emplace_back(chr.uvs[0].x, chr.uvs[0].y);
                    text_uvs.emplace_back(chr.uvs[1].x, chr.uvs[1].y);
                    text_uvs.emplace_back(chr.uvs[2].x, chr.uvs[2].y);
                    text_uvs.emplace_back(chr.uvs[3].x, chr.uvs[3].y);
                }
            }
        }
        void _renderTextRight(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
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
                        const FontGlyph& chr   = font.getGlyphData(character);
                        const float startingY = -int(chr.height + chr.yoffset) - y;

                        text_ind.emplace_back(accum + 0);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 2);
                        text_ind.emplace_back(accum + 3);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 0);

                        if (k == 0) {
                            x -= chr.width;
                        }
                        const float startingX = x + chr.xoffset;
                        x -= chr.xadvance;

                        text_pts.emplace_back(startingX + chr.pts[0].x, startingY + chr.pts[0].y, z);
                        text_pts.emplace_back(startingX + chr.pts[1].x, startingY + chr.pts[1].y, z);
                        text_pts.emplace_back(startingX + chr.pts[2].x, startingY + chr.pts[2].y, z);
                        text_pts.emplace_back(startingX + chr.pts[3].x, startingY + chr.pts[3].y, z);

                        text_uvs.emplace_back(chr.uvs[0].x, chr.uvs[0].y);
                        text_uvs.emplace_back(chr.uvs[1].x, chr.uvs[1].y);
                        text_uvs.emplace_back(chr.uvs[2].x, chr.uvs[2].y);
                        text_uvs.emplace_back(chr.uvs[3].x, chr.uvs[3].y);
                        ++k;
                    }
                }
                y += newLineGlyphHeight;
                x = 0.0f;
            }
        }
        void _renderTextCenter(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
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
            if (lines.size() == 0) {
                lines.push_back(line_accumulator);
                lines_sizes.push_back(static_cast<unsigned short>(x));
            }

            x = 0.0f;
            unsigned int i = 0;
            for (uint l = 0; l < lines.size(); ++l) {
                const auto& line      = lines[l];
                const auto& line_size = lines_sizes[l] / 2;
                for (auto& character : line) {
                    if (character != '\0') {
                        const unsigned int accum = i * 4;
                        ++i;
                        const FontGlyph& chr   = font.getGlyphData(character);
                        const float startingY = -int(chr.height + chr.yoffset) - y;

                        text_ind.emplace_back(accum + 0);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 2);
                        text_ind.emplace_back(accum + 3);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 0);

                        const float startingX = x + chr.xoffset;
                        x += chr.xadvance;

                        text_pts.emplace_back(startingX + chr.pts[0].x - line_size, startingY + chr.pts[0].y, z);
                        text_pts.emplace_back(startingX + chr.pts[1].x - line_size, startingY + chr.pts[1].y, z);
                        text_pts.emplace_back(startingX + chr.pts[2].x - line_size, startingY + chr.pts[2].y, z);
                        text_pts.emplace_back(startingX + chr.pts[3].x - line_size, startingY + chr.pts[3].y, z);

                        text_uvs.emplace_back(chr.uvs[0].x, chr.uvs[0].y);
                        text_uvs.emplace_back(chr.uvs[1].x, chr.uvs[1].y);
                        text_uvs.emplace_back(chr.uvs[2].x, chr.uvs[2].y);
                        text_uvs.emplace_back(chr.uvs[3].x, chr.uvs[3].y);
                    }
                }
                y += newLineGlyphHeight;
                x = 0.0f;
            }
        }
        void _renderSunLight(Camera& c, SunLight& s) {
            if (!s.isActive())
                return;
            auto& body = *s.getComponent<ComponentBody>();
            const auto pos = body.position_render();
            Engine::Renderer::sendUniform4("light.DataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, 0.0f);
            Engine::Renderer::sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
            Engine::Renderer::sendUniform4("light.DataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, static_cast<float>(s.m_Type));
            Engine::Renderer::sendUniform1Safe("Type", 0.0f);

            Engine::Renderer::renderFullscreenTriangle();
        }
        void _renderPointLight(Camera& c, PointLight& p) {
            if (!p.isActive()) 
                return;
            auto& body = *p.getComponent<ComponentBody>();
            const auto pos = body.position_render();
            const auto factor = 1100.0f * p.m_CullingRadius;
            const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
            if ((!c.sphereIntersectTest(pos, p.m_CullingRadius)) || (distSq > factor * factor))
                return;
            Engine::Renderer::sendUniform4("light.DataA", p.m_AmbientIntensity, p.m_DiffuseIntensity, p.m_SpecularIntensity, 0.0f);
            Engine::Renderer::sendUniform4("light.DataB", 0.0f, 0.0f, p.m_C, p.m_L);
            Engine::Renderer::sendUniform4("light.DataC", p.m_E, pos.x, pos.y, pos.z);
            Engine::Renderer::sendUniform4("light.DataD", p.m_Color.x, p.m_Color.y, p.m_Color.z, static_cast<float>(p.m_Type));
            Engine::Renderer::sendUniform4Safe("light.DataE", 0.0f, 0.0f, static_cast<float>(p.m_AttenuationModel), 0.0f);
            Engine::Renderer::sendUniform1Safe("Type", 1.0f);

            const auto model = body.modelMatrixRendering();

            Engine::Renderer::sendUniformMatrix4("Model", model);
            Engine::Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (distSq <= (p.m_CullingRadius * p.m_CullingRadius)) { //inside the light volume
                Engine::Renderer::cullFace(GL_FRONT);
            }else{
                Engine::Renderer::cullFace(GL_BACK);
            }
            auto& pointLightMesh = const_cast<Mesh&>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPointLightBounds());

            pointLightMesh.bind();
            pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            pointLightMesh.unbind();
            Engine::Renderer::cullFace(GL_BACK);
        }
        void _renderDirectionalLight(Camera& c, DirectionalLight& d) {
            if (!d.isActive()) 
                return;
            auto& body = *d.getComponent<ComponentBody>();
            const auto _forward = glm::vec3(body.forward());
            Engine::Renderer::sendUniform4("light.DataA", d.m_AmbientIntensity, d.m_DiffuseIntensity, d.m_SpecularIntensity, _forward.x);
            Engine::Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, 0.0f, 0.0f);
            Engine::Renderer::sendUniform4("light.DataD", d.m_Color.x, d.m_Color.y, d.m_Color.z, static_cast<float>(d.m_Type));
            Engine::Renderer::sendUniform1Safe("Type", 0.0f);
            Engine::Renderer::renderFullscreenTriangle();
        }
        void _renderSpotLight(Camera& c, SpotLight& s) {
            if (!s.isActive()) 
                return;
            auto& body = *s.m_Entity.getComponent<ComponentBody>();
            auto pos = body.position_render();
            auto _forward = glm::vec3(body.forward());
            const auto factor = 1100.0f * s.m_CullingRadius;
            const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
            if (!c.sphereIntersectTest(pos, s.m_CullingRadius) || (distSq > factor * factor))
                return;
            Engine::Renderer::sendUniform4("light.DataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, _forward.x);
            Engine::Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, s.m_C, s.m_L);
            Engine::Renderer::sendUniform4("light.DataC", s.m_E, pos.x, pos.y, pos.z);
            Engine::Renderer::sendUniform4("light.DataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, static_cast<float>(s.m_Type));
            Engine::Renderer::sendUniform4Safe("light.DataE", s.m_Cutoff, s.m_OuterCutoff, static_cast<float>(s.m_AttenuationModel), 0.0f);
            Engine::Renderer::sendUniform2Safe("VertexShaderData", s.m_OuterCutoff, s.m_CullingRadius);
            Engine::Renderer::sendUniform1Safe("Type", 2.0f);

            const auto model = body.modelMatrixRendering();

            Engine::Renderer::sendUniformMatrix4("Model", model);
            Engine::Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (distSq <= (s.m_CullingRadius * s.m_CullingRadius)) { //inside the light volume                                                 
                Engine::Renderer::cullFace(GL_FRONT);
            }else{
                Engine::Renderer::cullFace(GL_BACK);
            }
            auto& spotLightMesh = const_cast<Mesh&>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getSpotLightBounds());

            spotLightMesh.bind();
            spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            spotLightMesh.unbind();
            Engine::Renderer::cullFace(GL_BACK);

            Engine::Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
        }
        void _renderRodLight(Camera& c, RodLight& r) {
            if (!r.isActive()) 
                return;
            auto& body = *r.m_Entity.getComponent<ComponentBody>();
            const auto pos = body.position_render();
            auto cullingDistance = r.m_RodLength + (r.m_CullingRadius * 2.0f);
            const auto factor = 1100.0f * cullingDistance;
            const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
            if (!c.sphereIntersectTest(pos, cullingDistance) || (distSq > factor * factor))
                return;
            const float half = r.m_RodLength / 2.0f;
            const auto firstEndPt = pos + (glm::vec3(body.forward()) * half);
            const auto secndEndPt = pos - (glm::vec3(body.forward()) * half);
            Engine::Renderer::sendUniform4("light.DataA", r.m_AmbientIntensity, r.m_DiffuseIntensity, r.m_SpecularIntensity, firstEndPt.x);
            Engine::Renderer::sendUniform4("light.DataB", firstEndPt.y, firstEndPt.z, r.m_C, r.m_L);
            Engine::Renderer::sendUniform4("light.DataC", r.m_E, secndEndPt.x, secndEndPt.y, secndEndPt.z);
            Engine::Renderer::sendUniform4("light.DataD", r.m_Color.x, r.m_Color.y, r.m_Color.z, static_cast<float>(r.m_Type));
            Engine::Renderer::sendUniform4Safe("light.DataE", r.m_RodLength, 0.0f, static_cast<float>(r.m_AttenuationModel), 0.0f);
            Engine::Renderer::sendUniform1Safe("Type", 1.0f);

            const auto model = body.modelMatrixRendering();

            Engine::Renderer::sendUniformMatrix4("Model", model);
            Engine::Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (distSq <= (cullingDistance * cullingDistance)) {
                Engine::Renderer::cullFace(GL_FRONT);
            }else{
                Engine::Renderer::cullFace(GL_BACK);
            }
            auto& rodLightMesh = const_cast<Mesh&>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getRodLightBounds());

            rodLightMesh.bind();
            rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            rodLightMesh.unbind();
            Engine::Renderer::cullFace(GL_BACK);

            Engine::Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
        }
        
        void _renderParticle(Particle& p, Camera& c, Scene& s) {
            p.getMaterial()->bind();

            auto maxTextures = priv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;

            Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer->getTexture(GBufferType::Depth), maxTextures);
            Engine::Renderer::sendUniform4Safe("Object_Color", p.color());
            Engine::Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, p.position());
            modelMatrix *= glm::mat4_cast(c.getOrientation());
            modelMatrix = glm::rotate(modelMatrix, p.angle(), glm::vec3(0, 0, 1));
            auto& scale = p.getScale();
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));

            Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

            auto& plane = const_cast<Mesh&>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh());

            plane.render();
        }

        void _passGeometry(const double& dt, GBuffer& gbuffer, Viewport& viewport, Camera& camera){
            Scene& scene = viewport.m_Scene;
            const glm::vec4& clear = viewport.m_BackgroundColor;
            const float colors[4] = { clear.r, clear.g, clear.b, clear.a };
    
            gbuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

            Engine::Renderer::Settings::clear(true,true,true); // (0,0,0,0)
            
            Engine::Renderer::setDepthFunc(GL_LEQUAL);

            glClearBufferfv(GL_COLOR, 0, colors);
            auto& godRays = GodRays::godRays;
            if(godRays.godRays_active){
                const float godraysclearcolor[4] = { godRays.clearColor.r, godRays.clearColor.g, godRays.clearColor.b, godRays.clearColor.a };
                glClearBufferfv(GL_COLOR, 2, godraysclearcolor);
            }
            Engine::Renderer::GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure
            InternalScenePublicInterface::RenderGeometryOpaque(scene, viewport, camera);
            if ((viewport.getRenderFlags() & ViewportRenderingFlag::Skybox)) {
                _renderSkybox(scene.skybox(), scene, viewport, camera);
            }
            InternalScenePublicInterface::RenderGeometryTransparent(scene, viewport, camera);
            InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(scene, viewport, camera, true);
        }
        void _passForwardRendering(const double& dt, GBuffer& gbuffer, Viewport& viewport, Camera& camera){
            Scene& scene = viewport.m_Scene;
            gbuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, GBufferType::Lighting, "RGBA");
            InternalScenePublicInterface::RenderForwardOpaque(scene, viewport, camera, dt);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            Engine::Renderer::GLEnablei(GL_BLEND, 0); //this might need to be all buffers not just 0
 
            Engine::Renderer::GLEnablei(GL_BLEND, 1); //yes this is important
            Engine::Renderer::GLEnablei(GL_BLEND, 2); //yes this is important
            Engine::Renderer::GLEnablei(GL_BLEND, 3); //yes this is important

            glDepthMask(GL_TRUE);
            InternalScenePublicInterface::RenderForwardTransparent(scene, viewport, camera);
            InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(scene, viewport, camera);
            glDepthMask(GL_FALSE);
            InternalScenePublicInterface::RenderDecals(scene, viewport, camera);
            InternalScenePublicInterface::RenderForwardParticles(scene, viewport, camera);
            InternalScenePublicInterface::RenderParticles(scene, viewport, camera, *m_InternalShaderPrograms[EngineInternalShaderPrograms::Particle] ,gbuffer);

            Engine::Renderer::GLDisablei(GL_BLEND, 0); //this is needed for smaa at least
            Engine::Renderer::GLDisablei(GL_BLEND, 1);
            Engine::Renderer::GLDisablei(GL_BLEND, 2);
            Engine::Renderer::GLDisablei(GL_BLEND, 3);
        }
        void _passCopyDepth(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight){
            Engine::Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth]->bind();

            Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 0);

            _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
            Engine::Renderer::colorMask(true, true, true, true);
        }
        void _passLighting(GBuffer& gbuffer, Viewport& viewport, Camera& camera, const uint& fboWidth, const uint& fboHeight,bool mainRenderFunc){
            Scene& scene = viewport.m_Scene;

            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting]->bind();

            if(Renderer::GLSL_VERSION < 140){
                Engine::Renderer::sendUniformMatrix4Safe("CameraView", camera.getView());
                Engine::Renderer::sendUniformMatrix4Safe("CameraProj", camera.getProjection());
                //sendUniformMatrix4Safe("CameraViewProj",camera.getViewProjection()); //moved to shader binding function
                Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
                Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
                Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
                Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
                Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
            }
            const auto& fbo_width = static_cast<float>(fboWidth);
            const auto& fbo_height = static_cast<float>(fboHeight);

            Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, static_cast<uint>(Material::m_MaterialProperities.size()));
            Engine::Renderer::sendUniform4("ScreenData", 0.0f, gamma, fbo_width, fbo_height);

            Engine::Renderer::sendTexture("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 0);
            Engine::Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 1);
            Engine::Renderer::sendTexture("gMiscMap", gbuffer.getTexture(GBufferType::Misc), 2);
            Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 3);
            Engine::Renderer::sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 4);

            Engine::Renderer::setDepthFunc(GL_GEQUAL);
            Engine::Renderer::GLEnable(GL_DEPTH_TEST);
            for (const auto& light : InternalScenePublicInterface::GetPointLights(scene)) {
                _renderPointLight(camera , *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetSpotLights(scene)) {
                _renderSpotLight(camera , *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetRodLights(scene)) {
                _renderRodLight(camera , *light);
            }
            Engine::Renderer::setDepthFunc(GL_LEQUAL);
            Engine::Renderer::GLDisable(GL_DEPTH_TEST);
            for (const auto& light : InternalScenePublicInterface::GetSunLights(scene)) {
                _renderSunLight(camera, *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetDirectionalLights(scene)) {
                _renderDirectionalLight(camera, *light);
            }

            if(mainRenderFunc){
                //do GI here. (only doing GI during the main render pass, not during light probes
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI]->bind();
                if(Renderer::GLSL_VERSION < 140){
                    Engine::Renderer::sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
                    Engine::Renderer::sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
                    Engine::Renderer::sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
                    Engine::Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
                    Engine::Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
                }
                
                Engine::Renderer::sendUniform4v("materials[0]", Material::m_MaterialProperities, static_cast<uint>(Material::m_MaterialProperities.size()));
                Engine::Renderer::sendUniform4("ScreenData", lighting_gi_pack, gamma, fbo_width, fbo_height);
                Engine::Renderer::sendTexture("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 0);
                Engine::Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 1);
                Engine::Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);
                Engine::Renderer::sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 3);
                Engine::Renderer::sendTexture("gMiscMap", gbuffer.getTexture(GBufferType::Misc), 4);
                Skybox* skybox = scene.skybox();
                if(skybox && skybox->texture()->numAddresses() >= 3){
                    Engine::Renderer::sendTextureSafe("irradianceMap", skybox->texture()->address(1), 5, GL_TEXTURE_CUBE_MAP);
                    Engine::Renderer::sendTextureSafe("prefilterMap", skybox->texture()->address(2), 6, GL_TEXTURE_CUBE_MAP);
                    Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, 7);
                }else{
                    Engine::Renderer::sendTextureSafe("irradianceMap", Texture::Black->address(0), 5, GL_TEXTURE_2D);
                    Engine::Renderer::sendTextureSafe("prefilterMap", Texture::Black->address(0), 6, GL_TEXTURE_2D);
                    Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, 7);
                }
                _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
            }
        }
        void _passStencil(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight){
            Engine::Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass]->bind();

            gbuffer.getMainFBO()->bind();

            Engine::Renderer::GLEnable(GL_STENCIL_TEST);
            Engine::Renderer::Settings::clear(false,false,true); //stencil is completely filled with 0's
            Engine::Renderer::stencilMask(0xFFFFFFFF);

            Engine::Renderer::stencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
            //exclude shadeless normals
            Engine::Renderer::stencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);


            Engine::Renderer::sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),0);
            Engine::Renderer::sendUniform1("Type",0.0f);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);

            Engine::Renderer::stencilMask(0xFFFFFFFF);

            Engine::Renderer::stencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);

            Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil

            Engine::Renderer::colorMask(true, true, true, true);
        }
        void _passBlur(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight,string type, GLuint texture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur]->bind();

            const float& divisor = gbuffer.getSmallFBO()->divisor();
            glm::vec2 hv(0.0f);
            if(type == "H"){ hv = glm::vec2(1.0f,0.0f); }
            else{            hv = glm::vec2(0.0f,1.0f); }

            const glm::ivec2 Res(fboWidth, fboHeight);
            auto& bloom = Bloom::bloom;
            Engine::Renderer::sendUniform4("strengthModifier",
                bloom.blur_strength,
                bloom.blur_strength,
                bloom.blur_strength,
                SSAO::ssao.m_ssao_blur_strength
            );
            Engine::Renderer::sendUniform2("Resolution", Res);
            Engine::Renderer::sendUniform4("DataA", bloom.blur_radius,0.0f,hv.x,hv.y);
            Engine::Renderer::sendTexture("image",gbuffer.getTexture(texture),0);

            const uint screen_width = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
            const uint screen_height = static_cast<uint>(static_cast<float>(fboHeight) * divisor);
            _renderFullscreenTriangle(screen_width, screen_height, 0, 0);
        }
        void _passFinal(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight, GBufferType::Type sceneTexture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal]->bind();
            Engine::Renderer::sendUniform1Safe("HasBloom", static_cast<int>(Bloom::bloom.bloom_active));
            Engine::Renderer::sendUniform1Safe("HasFog", static_cast<int>(Fog::fog.fog_active));

            if(Fog::fog.fog_active){
                Engine::Renderer::sendUniform1Safe("FogDistNull", Fog::fog.distNull);
                Engine::Renderer::sendUniform1Safe("FogDistBlend", Fog::fog.distBlend);
                Engine::Renderer::sendUniform4Safe("FogColor", Fog::fog.color);
                Engine::Renderer::sendTextureSafe("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);
            }
            Engine::Renderer::sendTextureSafe("SceneTexture", gbuffer.getTexture(sceneTexture), 0);
            Engine::Renderer::sendTextureSafe("gBloomMap", gbuffer.getTexture(GBufferType::Bloom), 1);
            Engine::Renderer::sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 2);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _passDepthAndTransparency(GBuffer& gbuffer , const uint& fboWidth, const uint& fboHeight, Viewport& viewport, Camera& camera, GBufferType::Type sceneTexture) {
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DepthAndTransparency]->bind();

            Engine::Renderer::sendTextureSafe("SceneTexture", gbuffer.getTexture(sceneTexture), 0);
            Engine::Renderer::sendTextureSafe("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 1);

            Engine::Renderer::GLEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            //sendUniform4Safe("TransparencyMaskColor", viewport.getTransparencyMaskColor());
            //sendUniform1Safe("TransparencyMaskActive", static_cast<int>(viewport.isTransparencyMaskActive()));
            Engine::Renderer::sendUniform1Safe("DepthMaskValue", viewport.getDepthMaskValue());
            Engine::Renderer::sendUniform1Safe("DepthMaskActive", static_cast<int>(viewport.isDepthMaskActive()));

            _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);

            Engine::Renderer::GLDisable(GL_BLEND);
        }
        void _renderFullscreenQuad(const uint& width, const uint& height,uint startX,uint startY){
            const float w2 = static_cast<float>(width) * 0.5f;
            const float h2 = static_cast<float>(height) * 0.5f;
            const glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            Engine::Renderer::sendUniformMatrix4("Model", m_IdentityMat4);
            Engine::Renderer::sendUniformMatrix4("VP", p);
            Engine::Renderer::sendUniform2("screenSizeDivideBy2", w2, h2);
            m_FullscreenQuad->render();
        }
        void _renderFullscreenTriangle(const uint& width, const uint& height,uint startX,uint startY){
            const float w2 = static_cast<float>(width) * 0.5f;
            const float h2 = static_cast<float>(height) * 0.5f;
            const glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            Engine::Renderer::sendUniformMatrix4("Model", m_IdentityMat4);
            Engine::Renderer::sendUniformMatrix4("VP", p);
            Engine::Renderer::sendUniform2("screenSizeDivideBy2", w2, h2);
            m_FullscreenTriangle->render();
        }
        
        void _startupRenderFrame(GBuffer& gbuffer, Viewport& viewport, Camera& camera, const glm::uvec4& dimensions) {
            const auto& winSize = Resources::getWindowSize();
            if (viewport.isAspectRatioSynced()) {
                camera.setAspect(dimensions.z / static_cast<float>(dimensions.w));
            }
            Engine::Renderer::setViewport(static_cast<float>(dimensions.x), static_cast<float>(dimensions.y), static_cast<float>(dimensions.z), static_cast<float>(dimensions.w));
            gbuffer.resize(dimensions.z, dimensions.w);

            //scissor disabling
            glScissor(0, 0, winSize.x, winSize.y);

            m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(winSize.x), 0.0f, static_cast<float>(winSize.y), 0.005f, 3000.0f);
            //this is god awful and ugly, but its needed. definately find a way to refactor this properly
            for (uint i = 0; i < 9; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
        }
        
        void _render(const double& dt, GBuffer& gbuffer, Viewport& viewport,const bool& mainRenderFunc, const GLuint& fbo, const GLuint& rbo){
            const Scene& scene           = viewport.m_Scene;
            Camera& camera               = const_cast<Camera&>(viewport.getCamera());
            const glm::uvec4& dimensions = viewport.getViewportDimensions();

            _startupRenderFrame(gbuffer, viewport, camera, dimensions);

            if(mainRenderFunc){
                #pragma region Camera UBO
                if(Renderer::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA){  
                    //TODO: change the manual camera uniform sending (for when glsl version < 140) to give a choice between the two render spaces
                    /*
                    //same simulation and render space
                    m_UBOCameraData.View        = camera.getView();
                    m_UBOCameraData.Proj        = camera.getProjection();
                    m_UBOCameraData.ViewProj    = camera.getViewProjection();
                    m_UBOCameraData.InvProj     = camera.getProjectionInverse();
                    m_UBOCameraData.InvView     = camera.getViewInverse();
                    m_UBOCameraData.InvViewProj = camera.getViewProjectionInverse();
                    m_UBOCameraData.Info1       = glm::vec4(camera.getPosition(),camera.getNear());
                    m_UBOCameraData.Info2       = glm::vec4(camera.getViewVector(),camera.getFar());
                    m_UBOCameraData.Info3       = glm::vec4(0.0f,0.0f,0.0f, 0.0f);
                    */
                    
                    //this render space places the camera at the origin and offsets submitted model matrices to the vertex shaders
                    //by the camera's real simulation position
                    //this helps to deal with shading inaccuracies for when the camera is very far away from the origin
                    m_UBOCameraData.View        = ComponentCamera_Functions::GetViewNoTranslation(camera);
                    m_UBOCameraData.Proj        = camera.getProjection();
                    m_UBOCameraData.ViewProj    = ComponentCamera_Functions::GetViewProjectionNoTranslation(camera);
                    m_UBOCameraData.InvProj     = camera.getProjectionInverse();
                    m_UBOCameraData.InvView     = ComponentCamera_Functions::GetViewInverseNoTranslation(camera);
                    m_UBOCameraData.InvViewProj = ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(camera);
                    m_UBOCameraData.Info1       = glm::vec4(0.0001f,0.0001f,0.0001f, camera.getNear());
                    m_UBOCameraData.Info2       = glm::vec4(ComponentCamera_Functions::GetViewVectorNoTranslation(camera), camera.getFar());
                    m_UBOCameraData.Info3       = glm::vec4(camera.getPosition(), 0.0f);
                    
                    UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);           
                }
                #pragma endregion
            }
            _passGeometry(dt, gbuffer, viewport, camera);
            Engine::Renderer::GLDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);


            Engine::Renderer::GLDisablei(GL_BLEND, 0);

            #pragma region SSAO
            //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes
            gbuffer.bindFramebuffers(GBufferType::Bloom, GBufferType::GodRays, "A", false);
            Engine::Renderer::Settings::clear(true, false, false); //bloom and god rays alpha channels cleared to black 
            if (SSAO::ssao.m_ssao && (viewport.getRenderFlags() & ViewportRenderingFlag::SSAO)) {
                Engine::Renderer::GLEnablei(GL_BLEND, 0);//i dont think this is needed anymore
                gbuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
                auto& ssaoShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO];
                SSAO::ssao.passSSAO(ssaoShader, gbuffer, dimensions.z, dimensions.w, camera);
                if (SSAO::ssao.m_ssao_do_blur) {
                    Engine::Renderer::GLDisablei(GL_BLEND, 0); //yes this is absolutely needed
                    auto& ssaoBlurShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlurSSAO];
                    for (uint i = 0; i < SSAO::ssao.m_ssao_blur_num_passes; ++i) {
                        gbuffer.bindFramebuffers(GBufferType::GodRays, "A", false);
                        SSAO::ssao.passBlur(ssaoBlurShader, gbuffer, dimensions.z, dimensions.w, "H", GBufferType::Bloom);
                        gbuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
                        SSAO::ssao.passBlur(ssaoBlurShader, gbuffer, dimensions.z, dimensions.w, "V", GBufferType::GodRays);
                    }
                }  
            }
            
            #pragma endregion

            Engine::Renderer::GLDisablei(GL_BLEND, 0);

            _passStencil(gbuffer, dimensions.z, dimensions.w); //confirmed, stencil rejection does help
            
            Engine::Renderer::GLEnablei(GL_BLEND, 0);
            glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            
            //this needs to be cleaned up
            gbuffer.bindFramebuffers(GBufferType::Lighting, "RGB");
            Engine::Renderer::Settings::clear(true, false, false);//lighting rgb channels cleared to black
            if(lighting){
                _passLighting(gbuffer,viewport, camera, dimensions.z, dimensions.w, mainRenderFunc);
            }
            
            Engine::Renderer::GLDisablei(GL_BLEND, 0);
            Engine::Renderer::GLDisable(GL_STENCIL_TEST);


            Engine::Renderer::GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            _passForwardRendering(dt, gbuffer, viewport, camera);
            Engine::Renderer::GLDisable(GL_DEPTH_TEST);


            #pragma region GodRays
            gbuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
            Engine::Renderer::Settings::clear(true, false, false); //godrays rgb channels cleared to black
            auto& godRaysPlatform = GodRays::godRays;
            auto* sun = Engine::Renderer::godRays::getSun();
            if ((viewport.getRenderFlags() & ViewportRenderingFlag::GodRays) && godRaysPlatform.godRays_active && sun) {
                auto& body = *sun->getComponent<ComponentBody>();
                const glm::vec3& oPos = body.position();
                const glm::vec3& camPos = camera.getPosition();
                const glm::vec3& camVec = camera.getViewVector();
                const bool infront = Math::isPointWithinCone(camPos, -camVec, oPos, Math::toRadians(godRaysPlatform.fovDegrees));
                if (infront) {
                    const glm::vec3& sp = Math::getScreenCoordinates(oPos, camera, false);
                    float alpha = Math::getAngleBetweenTwoVectors(camVec, camPos - oPos, true) / godRaysPlatform.fovDegrees;

                    alpha = glm::pow(alpha, godRaysPlatform.alphaFalloff);
                    alpha = glm::clamp(alpha, 0.01f, 0.99f);
                    if (boost::math::isnan(alpha) || boost::math::isinf(alpha)) { //yes this is needed...
                        alpha = 0.01f;
                    }
                    alpha = 1.0f - alpha;
                    auto& godRaysShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredGodRays];
                    godRaysPlatform.pass(godRaysShader, gbuffer, dimensions.z, dimensions.w, glm::vec2(sp.x, sp.y), alpha);
                }
            }
            #pragma endregion




            #pragma region HDR and GodRays addition
            gbuffer.bindFramebuffers(GBufferType::Misc);
            auto& hdrShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHDR];
            HDR::hdr.pass(hdrShader, gbuffer, dimensions.z, dimensions.w, godRaysPlatform.godRays_active, lighting, godRaysPlatform.factor);
            #pragma endregion
            
            #pragma region Bloom
            //TODO: possible optimization: use stencil buffer to reject completely black pixels during blur passes
            if (Bloom::bloom.bloom_active && (viewport.getRenderFlags() & ViewportRenderingFlag::Bloom)) {
                gbuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
                auto& bloomShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBloom];
                Bloom::bloom.pass(bloomShader, gbuffer, dimensions.z, dimensions.w, GBufferType::Lighting);
                for (uint i = 0; i < Bloom::bloom.num_passes; ++i) {
                    gbuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
                    _passBlur(gbuffer, dimensions.z, dimensions.w, "H", GBufferType::Bloom);
                    gbuffer.bindFramebuffers(GBufferType::Bloom, "RGB", false);
                    _passBlur(gbuffer, dimensions.z, dimensions.w, "V", GBufferType::GodRays);
                }
            }
            #pragma endregion

            GBufferType::Type sceneTexture = GBufferType::Misc;
            GBufferType::Type outTexture = GBufferType::Lighting;
            #pragma region DOF
            if (DepthOfField::DOF.dof && (viewport.getRenderFlags() & ViewportRenderingFlag::DepthOfField)) {
                auto& dofShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF];
                gbuffer.bindFramebuffers(outTexture);
                DepthOfField::DOF.pass(dofShader,gbuffer, dimensions.z, dimensions.w, sceneTexture);
                sceneTexture = GBufferType::Lighting;
                outTexture = GBufferType::Misc;
            }
            #pragma endregion
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            #pragma region Finalization and AA
            if (!mainRenderFunc || aa_algorithm == AntiAliasingAlgorithm::None || !(viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)){
                gbuffer.bindFramebuffers(outTexture);
                _passFinal(gbuffer, dimensions.z, dimensions.w, sceneTexture);
                gbuffer.bindBackbuffer(viewport, fbo, rbo);
                _passDepthAndTransparency(gbuffer, dimensions.z, dimensions.w, viewport,camera,outTexture);
            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::FXAA && (viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)){
                gbuffer.bindFramebuffers(outTexture);
                _passFinal(gbuffer, dimensions.z, dimensions.w, sceneTexture);
                gbuffer.bindFramebuffers(sceneTexture);
                auto& fxaaShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA];
                FXAA::fxaa.pass(fxaaShader, gbuffer, dimensions.z, dimensions.w, outTexture);

                gbuffer.bindBackbuffer(viewport, fbo, rbo);
                _passDepthAndTransparency(gbuffer, dimensions.z, dimensions.w, viewport, camera, sceneTexture);

            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::SMAA && (viewport.getRenderFlags() & ViewportRenderingFlag::AntiAliasing)){
                gbuffer.bindFramebuffers(outTexture);
                _passFinal(gbuffer, dimensions.z, dimensions.w, sceneTexture);

                std::swap(sceneTexture, outTexture);


                const float& _fboWidth  = static_cast<float>(dimensions.z);
                const float& _fboHeight = static_cast<float>(dimensions.w);
                const glm::vec4& SMAA_PIXEL_SIZE = glm::vec4(1.0f / _fboWidth, 1.0f / _fboHeight, _fboWidth, _fboHeight);

                auto& edgeProgram     = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA1];
                auto& blendProgram    = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA2];
                auto& neighborProgram = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA3];
                auto& finalProgram    = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA4];

                SMAA::smaa.passEdge(edgeProgram, gbuffer, SMAA_PIXEL_SIZE, dimensions.z, dimensions.w, sceneTexture, outTexture);

                SMAA::smaa.passBlend(blendProgram, gbuffer, SMAA_PIXEL_SIZE, dimensions.z, dimensions.w, outTexture);

                gbuffer.bindFramebuffers(outTexture);
                SMAA::smaa.passNeighbor(neighborProgram, gbuffer, SMAA_PIXEL_SIZE, dimensions.z, dimensions.w, sceneTexture);
                //gbuffer.bindFramebuffers(sceneTexture);

                //SMAA::smaa.passFinal(finalProgram, gbuffer, dimensions.z, dimensions.w);//unused

                gbuffer.bindBackbuffer(viewport, fbo, rbo);
                _passDepthAndTransparency(gbuffer, dimensions.z, dimensions.w, viewport, camera, outTexture);
            }
            _passCopyDepth(gbuffer, dimensions.z, dimensions.w);
            #pragma endregion
            
            
            #pragma region RenderPhysics
            Engine::Renderer::GLEnablei(GL_BLEND, 0);
            if(mainRenderFunc && (viewport.getRenderFlags() & ViewportRenderingFlag::PhysicsDebug)){
                if(draw_physics_debug  &&  &camera == scene.getActiveCamera()){
                    Engine::Renderer::GLDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_FALSE);
                    m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics]->bind();
                    Core::m_Engine->m_PhysicsManager._render(camera);
                }
            }
            #pragma endregion
            
         
            #pragma region 2DAPI
            Engine::Renderer::GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            if (mainRenderFunc) {
                if ((viewport.getRenderFlags() & ViewportRenderingFlag::API2D)) {
                    Engine::Renderer::Settings::clear(false, true, false); //clear depth only
                    m_InternalShaderPrograms[EngineInternalShaderPrograms::Deferred2DAPI]->bind();
                    Engine::Renderer::sendUniformMatrix4("VP", m_2DProjectionMatrix);
                    Engine::Renderer::sendUniform1Safe("ScreenGamma", gamma);
                    Engine::Renderer::GLEnable(GL_SCISSOR_TEST);
                    for (auto& command : m_2DAPICommands) {
                        command.func();
                    }
                    Engine::Renderer::GLDisable(GL_SCISSOR_TEST);
                }
            }
            #pragma endregion

            
        }
        void _render2DApi(const double& dt, GBuffer& gbuffer, Viewport& viewport, const bool& mainRenderFunc, const GLuint& fbo, const GLuint& rbo) {
            Camera& camera = const_cast<Camera&>(viewport.getCamera());
            const glm::uvec4& dimensions = viewport.getViewportDimensions();

            const auto& winSize = Resources::getWindowSize();
            if (viewport.isAspectRatioSynced()) {
                camera.setAspect(dimensions.z / static_cast<float>(dimensions.w));
            }
            Engine::Renderer::setViewport(static_cast<float>(dimensions.x), static_cast<float>(dimensions.y), static_cast<float>(dimensions.z), static_cast<float>(dimensions.w));

            glScissor(0, 0, winSize.x, winSize.y);

            m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(winSize.x), 0.0f, static_cast<float>(winSize.y), 0.005f, 3000.0f);
            //this is god awful and ugly, but its needed. definately find a way to refactor this properly
            for (uint i = 0; i < 9; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
            if (Renderer::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA) {
                m_UBOCameraData.View = ComponentCamera_Functions::GetViewNoTranslation(camera);
                m_UBOCameraData.Proj = camera.getProjection();
                m_UBOCameraData.ViewProj = ComponentCamera_Functions::GetViewProjectionNoTranslation(camera);
                m_UBOCameraData.InvProj = camera.getProjectionInverse();
                m_UBOCameraData.InvView = ComponentCamera_Functions::GetViewInverseNoTranslation(camera);
                m_UBOCameraData.InvViewProj = ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(camera);
                m_UBOCameraData.Info1 = glm::vec4(0.0001f, 0.0001f, 0.0001f, camera.getNear());
                m_UBOCameraData.Info2 = glm::vec4(ComponentCamera_Functions::GetViewVectorNoTranslation(camera), camera.getFar());
                m_UBOCameraData.Info3 = glm::vec4(camera.getPosition(), 0.0f);

                UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
            }
            Engine::Renderer::setDepthFunc(GL_LEQUAL);
            Engine::Renderer::Settings::clear(true, true, true);
            Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure

            Engine::Renderer::GLDisable(GL_STENCIL_TEST);
            Engine::Renderer::GLEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_TRUE);
            if ((viewport.getRenderFlags() & ViewportRenderingFlag::API2D)) {
                Engine::Renderer::Settings::clear(false, true, false); //clear depth only
                m_InternalShaderPrograms[EngineInternalShaderPrograms::Deferred2DAPI]->bind();
                Engine::Renderer::sendUniformMatrix4("VP", m_2DProjectionMatrix);
                Engine::Renderer::sendUniform1Safe("ScreenGamma", gamma);
                Engine::Renderer::GLEnable(GL_SCISSOR_TEST);
                for (auto& command : m_2DAPICommands) {
                    command.func();
                }
                Engine::Renderer::GLDisable(GL_SCISSOR_TEST);
            }
        }
};


priv::Renderer::Renderer(const EngineOptions& options) : m_i(NEW impl){
    m_i->_init(options);
    renderManagerImpl = m_i.get();
    renderManager = this;

    OpenGLStateMachine = OpenGLState(options.width, options.height);
}
priv::Renderer::~Renderer(){
    cleanup();
}
void priv::Renderer::cleanup() {
    m_i->_destruct();
}
void priv::Renderer::_init(){
    m_i->_postInit();
}
void priv::Renderer::_render(const double& dt, Viewport& viewport,const bool mainFunc, const GLuint display_fbo, const GLuint display_rbo){
    m_i->_render(dt, *m_i->m_GBuffer, viewport, mainFunc, display_fbo, display_rbo);
}
void priv::Renderer::_render2DApi(const double& dt, Viewport& viewport, const bool mainFunc, const GLuint display_fbo, const GLuint display_rbo) {
    m_i->_render2DApi(dt, *m_i->m_GBuffer, viewport, mainFunc, display_fbo, display_rbo);
}
void priv::Renderer::_resize(uint w,uint h){
    m_i->_resize(w, h);
}
void priv::Renderer::_onFullscreen(const unsigned int& width, const unsigned int& height){
    SAFE_DELETE(m_i->m_GBuffer);
    //oh yea the opengl context is lost, gotta restore the state machine
    renderManager->OpenGLStateMachine.GL_RESTORE_CURRENT_STATE_MACHINE();

    Engine::Renderer::GLEnable(GL_CULL_FACE);
    Engine::Renderer::GLEnable(GL_DEPTH_CLAMP);
    m_i->m_GBuffer = NEW GBuffer(width, height);
}
void priv::Renderer::_onOpenGLContextCreation(uint windowWidth,uint windowHeight,uint _glslVersion,uint _openglVersion){
    OpenGLStateMachine.GL_INIT_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
    m_i->_onOpenGLContextCreation(windowWidth, windowHeight, _glslVersion, _openglVersion);
}
void priv::Renderer::_clear2DAPICommands() {
    m_i->m_2DAPICommands.clear();
}
void priv::Renderer::_sort2DAPICommands() {
    auto& commands = m_i->m_2DAPICommands;
    const auto& lambda_sorter = [&](API2DCommand& lhs, API2DCommand& rhs) {
        return lhs.depth > rhs.depth;
    };
    std::sort( std::execution::par_unseq, commands.begin(), commands.end(), lambda_sorter);
}
priv::GBuffer& priv::Renderer::getGbuffer() {
    return *m_i->m_GBuffer;
}
const float priv::Renderer::_getGIPackedData() {
    return m_i->lighting_gi_pack;
}
const bool priv::Renderer::_bindShaderProgram(ShaderProgram* program){
    auto& currentShaderPgrm = RendererState.current_bound_shader_program;
    if(currentShaderPgrm != program){
        OpenGLStateMachine.GL_glUseProgram(program->program());
        currentShaderPgrm = program;
        currentShaderPgrm->BindableResource::bind();
        return true;
    }
    return false;
}
const bool priv::Renderer::_unbindShaderProgram() {
    auto& currentShaderPgrm = RendererState.current_bound_shader_program;
    if (currentShaderPgrm) {
        currentShaderPgrm->BindableResource::unbind();
        currentShaderPgrm = nullptr;
        OpenGLStateMachine.GL_glUseProgram(0);
        return true;
    }
    return false;
}
const bool priv::Renderer::_bindMaterial(Material* material){
    auto& currentMaterial = RendererState.current_bound_material;
    if(currentMaterial != material){
        currentMaterial = material;
        currentMaterial->BindableResource::bind();
        return true;
    }
    return false;
}
const bool priv::Renderer::_unbindMaterial(){
    auto& currentMaterial = RendererState.current_bound_material;
    if(currentMaterial){
        currentMaterial->BindableResource::unbind();
        currentMaterial = nullptr;
        return true;
    }
    return false;
}
void priv::Renderer::_genPBREnvMapData(Texture& texture, uint size1, uint size2){
    m_i->_generatePBREnvMapData(texture,size1,size2);
}

void Renderer::restoreDefaultOpenGLState() {
    const auto winWidth = Resources::getWindowSize();
    renderManager->OpenGLStateMachine.GL_RESTORE_DEFAULT_STATE_MACHINE(winWidth.x, winWidth.y);
}
void Renderer::restoreCurrentOpenGLState() {
    renderManager->OpenGLStateMachine.GL_RESTORE_CURRENT_STATE_MACHINE();
}


void Renderer::Settings::Lighting::enable(const bool b){
    renderManagerImpl->lighting = b; 
}
void Renderer::Settings::Lighting::disable(){ 
    renderManagerImpl->lighting = false; 
}
const float Renderer::Settings::Lighting::getGIContributionGlobal(){
    return renderManagerImpl->lighting_gi_contribution_global; 
}
void Renderer::Settings::Lighting::setGIContributionGlobal(const float gi){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_global = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
const float Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return renderManagerImpl->lighting_gi_contribution_diffuse;
}
void Renderer::Settings::Lighting::setGIContributionDiffuse(const float gi){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_diffuse = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
const float Renderer::Settings::Lighting::getGIContributionSpecular(){
    return renderManagerImpl->lighting_gi_contribution_specular; 
}
void Renderer::Settings::Lighting::setGIContributionSpecular(const float gi){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_specular = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
void Renderer::Settings::Lighting::setGIContribution(const float g, const float d, const float s){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_global = glm::clamp(g,0.001f,0.999f);
    mgr.lighting_gi_contribution_diffuse = glm::clamp(d,0.001f,0.999f);
    mgr.lighting_gi_contribution_specular = glm::clamp(s,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}

const bool Renderer::Settings::setAntiAliasingAlgorithm(const AntiAliasingAlgorithm::Algorithm& algorithm){
    auto& i = *renderManagerImpl;
    if(i.aa_algorithm != algorithm){ 
        i.aa_algorithm = algorithm; 
        return true;
    }
    return false;
}
const bool Renderer::stencilOp(const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glStencilOp(sfail, dpfail, dppass);
}
const bool Renderer::stencilMask(const GLuint& mask) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glStencilMask(mask);
}
const bool Renderer::cullFace(const GLenum& state){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glCullFace(state);
}
void Renderer::Settings::clear(const bool color, const bool depth, const bool stencil){
    if(!color && !depth && !stencil) return;
    GLuint clearBit = 0x00000000;
    if(color)   clearBit |= GL_COLOR_BUFFER_BIT;
    if(depth)   clearBit |= GL_DEPTH_BUFFER_BIT;
    if(stencil) clearBit |= GL_STENCIL_BUFFER_BIT;
    glClear(clearBit);
}
void Renderer::Settings::enableDrawPhysicsInfo(const bool b){
    renderManagerImpl->draw_physics_debug = b; 
}
void Renderer::Settings::disableDrawPhysicsInfo(){ 
    renderManagerImpl->draw_physics_debug = false; 
}
void Renderer::Settings::setGamma(const float g){
    renderManagerImpl->gamma = g; 
}
const float Renderer::Settings::getGamma(){
    return renderManagerImpl->gamma; 
}
const bool Renderer::setDepthFunc(const GLenum& func){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glDepthFunc(func);
}
const bool Renderer::setViewport(const float& x, const float& y, const float& w, const float& h){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}
const bool Renderer::stencilFunc(const GLenum& func, const GLint& ref, const GLuint& mask) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glStencilFunc(func, ref, mask);
}
const bool Renderer::colorMask(const bool& r, const bool& g, const bool& b, const bool& a) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glColorMask(r, g, b, a);
}
const bool Renderer::clearColor(const float& r, const float& g, const float& b, const float& a) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glClearColor(r, g, b, a);
}

const bool Renderer::bindTextureForModification(const GLuint _textureType, const GLuint _textureObject) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glBindTextureForModification(_textureType, _textureObject);
}

const bool Renderer::bindVAO(const GLuint vaoObject){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glBindVertexArray(vaoObject);
}
const bool Renderer::deleteVAO(GLuint& vaoObject) {
    if (vaoObject) {
        glDeleteVertexArrays(1, &vaoObject);
        vaoObject = 0;
        return true;
    }
    return false;
}
void Renderer::genAndBindTexture(const GLuint _textureType, GLuint& _textureObject){
    auto& i = *renderManager;
    glGenTextures(1, &_textureObject);
    i.OpenGLStateMachine.GL_glBindTextureForModification(_textureType, _textureObject);
}
void Renderer::genAndBindVAO(GLuint& _vaoObject){
    glGenVertexArrays(1, &_vaoObject);
    bindVAO(_vaoObject);
}
const bool Renderer::GLEnable(const GLenum& capability) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glEnable(capability);
}
const bool Renderer::GLDisable(const GLenum& capability) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glDisable(capability);
}
const bool Renderer::GLEnablei(const GLenum& capability, const GLuint& index) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glEnablei(capability, index);
}
const bool Renderer::GLDisablei(const GLenum& capability, const GLuint& index) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glDisablei(capability, index);
}

void Renderer::sendTexture(const char* location, const Texture& texture,const int& slot){
    auto& i = *renderManager;
    i.OpenGLStateMachine.GL_glActiveTexture(slot);
    i.OpenGLStateMachine.GL_glBindTextureForRendering(texture.type(), texture.address());
    sendUniform1(location, slot);
}
void Renderer::sendTexture(const char* location,const GLuint textureObject,const int& slot,const GLuint& textureTarget){
    auto& i = *renderManager;
    i.OpenGLStateMachine.GL_glActiveTexture(slot);
    i.OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    sendUniform1(location, slot);
}
void Renderer::sendTextureSafe(const char* location, const Texture& texture,const int& slot){
    auto& i = *renderManager;
    i.OpenGLStateMachine.GL_glActiveTexture(slot);
    i.OpenGLStateMachine.GL_glBindTextureForRendering(texture.type(), texture.address());
    sendUniform1Safe(location, slot);
}
void Renderer::sendTextureSafe(const char* location,const GLuint textureObject,const int& slot,const GLuint& textureTarget){
    auto& i = *renderManager;
    i.OpenGLStateMachine.GL_glActiveTexture(slot);
    i.OpenGLStateMachine.GL_glBindTextureForRendering(textureTarget, textureObject);
    sendUniform1Safe(location, slot);
}
const bool Renderer::bindReadFBO(const GLuint& fbo){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
}
const bool Renderer::bindDrawFBO(const GLuint& fbo) {
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}
void Renderer::bindFBO(priv::FramebufferObject& fbo){ 
    Renderer::bindFBO(fbo.address()); 
}
const bool Renderer::bindRBO(priv::RenderbufferObject& rbo){
    return Renderer::bindRBO(rbo.address()); 
}
void Renderer::bindFBO(const GLuint& fbo){
    Renderer::bindReadFBO(fbo);
    Renderer::bindDrawFBO(fbo);
}
const bool Renderer::bindRBO(const GLuint& rbo){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glBindRenderbuffer(rbo);
}
void Renderer::unbindFBO(){ 
    Renderer::bindFBO(GLuint(0)); 
}
void Renderer::unbindRBO(){ 
    Renderer::bindRBO(GLuint(0)); 
}
void Renderer::unbindReadFBO(){ 
    Renderer::bindReadFBO(0); 
}
void Renderer::unbindDrawFBO(){ 
    Renderer::bindDrawFBO(0); 
}
inline const GLint Renderer::getUniformLoc(const char* location) {
    const auto& m = renderManager->RendererState.current_bound_shader_program->uniforms();
    if (!m.count(location)) 
        return -1; 
    return m.at(location);
}
inline const GLint& Renderer::getUniformLocUnsafe(const char* location) {
    return renderManager->RendererState.current_bound_shader_program->uniforms().at(location);
}



struct RenderingAPI2D final {
    //TODO: optimize scissoring
    static inline void GLScissorDisable() {
        const auto winSize = Resources::getWindowSize();
        glScissor(0, 0, winSize.x, winSize.y);
    }
    static void GLScissor(const glm::vec4& s) {
        if (s == glm::vec4(-1.0f)) {
            GLScissorDisable();
        }else{
            glScissor(static_cast<GLint>(s.x), static_cast<GLint>(s.y), static_cast<GLsizei>(s.z), static_cast<GLsizei>(s.w));
        }
    }
    static void Render2DText(const string& text, const Font& font, const glm::vec2& pos, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& alignType, const glm::vec4& scissor) {
        GLScissor(scissor);
        
        auto& impl = *renderManagerImpl;
        impl.text_pts.clear();
        impl.text_uvs.clear();
        impl.text_ind.clear();

        auto& mesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();
        mesh.bind();
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);

        const auto& newLineGlyphHeight = font.getMaxHeight();
        const auto& texture = font.getGlyphTexture();
        Engine::Renderer::sendTexture("DiffuseTexture", texture, 0);
        Engine::Renderer::sendUniform4("Object_Color", color);
        float y = 0.0f;
        float x = 0.0f;
        float z = -0.001f - depth;

        glm::mat4 m = impl.m_IdentityMat4;
        m = glm::translate(m, glm::vec3(pos.x, pos.y, 0));
        m = glm::rotate(m, angle, impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(scale.x, scale.y, 1));
        Engine::Renderer::sendUniformMatrix4("Model", m);

        if (alignType == TextAlignment::Left) {
            impl._renderTextLeft(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
        }else if (alignType == TextAlignment::Right) {
            impl._renderTextRight(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
        }else if (alignType == TextAlignment::Center) {
            impl._renderTextCenter(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
        }
        mesh.modifyVertices(0, impl.text_pts, MeshModifyFlags::Default); //prevent gpu upload until after all the data is collected
        mesh.modifyVertices(1, impl.text_uvs);
        mesh.modifyIndices(impl.text_ind);
        mesh.render(false);
    }
    static void Render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
        GLScissor(scissor);
        
        auto& impl = *renderManagerImpl;
        auto& plane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
        plane.bind();
        glm::mat4 m = impl.m_IdentityMat4;
        Engine::Renderer::sendUniform4("Object_Color", color);

        float translationX = position.x;
        float translationY = position.y;
        float totalSizeX = scale.x;
        float totalSizeY = scale.y;
        if (texture) {
            totalSizeX *= texture->width();
            totalSizeY *= texture->height();

            Engine::Renderer::sendTexture("DiffuseTexture", *texture, 0);
            Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
        }else{
            Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
            Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
        }
        Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

        m = glm::translate(m, glm::vec3(translationX, translationY, -0.001f - depth));
        m = glm::rotate(m, Math::toRadians(angle), impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(totalSizeX, totalSizeY, 1.0f));
        Engine::Renderer::sendUniformMatrix4("Model", m);
        plane.render(false);
    }
    static void RenderTriangle(const glm::vec2& pos, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
        GLScissor(scissor);
        
        auto& impl = *renderManagerImpl;

        auto& triangle = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh();
        triangle.bind();

        Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
        Engine::Renderer::sendUniform4("Object_Color", color);

        float translationX = pos.x;
        float translationY = pos.y;

        Renderer::alignmentOffset(align, translationX, translationY, width, height);

        glm::mat4 m = impl.m_IdentityMat4;
        m = glm::translate(m, glm::vec3(translationX, translationY, -0.001f - depth));
        m = glm::rotate(m, Math::toRadians(angle), impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(width, height, 1));
        Engine::Renderer::sendUniformMatrix4("Model", m);

        triangle.render(false);
    }
};

void Renderer::alignmentOffset(const Alignment::Type& align, float& x, float& y, const float& width, const float& height) {
    switch (align) {
        case Alignment::TopLeft: {
            x += width / 2;
            y -= height / 2;
            break;
        }case Alignment::TopCenter: {
            y -= height / 2;
            break;
        }case Alignment::TopRight: {
            x -= width / 2;
            y -= height / 2;
            break;
        }case Alignment::Left: {
            x += width / 2;
            break;
        }case Alignment::Center: {
            break;
        }case Alignment::Right: {
            x -= width / 2;
            break;
        }case Alignment::BottomLeft: {
            x += width / 2;
            y += height / 2;
            break;
        }case Alignment::BottomCenter: {
            y += height / 2;
            break;
        }case Alignment::BottomRight: {
            x -= width / 2;
            y += height / 2;
            break;
        }default: {
            break;
        }
    }
}
void Renderer::renderTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    priv::API2DCommand command;
    command.func = std::bind<void>(&RenderingAPI2D::RenderTriangle, position, color, angle, width, height, depth, align, scissor);
    command.depth = depth;
    renderManagerImpl->m_2DAPICommands.push_back(std::move(command));
}
void Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, const float width, const float height, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor){
    priv::API2DCommand command;
    command.func = std::bind<void>(&RenderingAPI2D::Render2DTexture, nullptr, pos, col, angle, glm::vec2(width, height), depth, align, scissor);
    command.depth = depth;
    renderManagerImpl->m_2DAPICommands.push_back(std::move(command));
}
void Renderer::renderTexture(const Texture& tex, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const Alignment::Type& align, const glm::vec4& scissor){
    priv::API2DCommand command;
    command.func = std::bind<void>(&RenderingAPI2D::Render2DTexture, &tex, p, c, a, s, d, align, scissor);
    command.depth = d;
    renderManagerImpl->m_2DAPICommands.push_back(std::move(command));
}
void Renderer::renderText(const string& t, const Font& fnt, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type& align, const glm::vec4& scissor) {
    priv::API2DCommand command;
    command.func = std::bind<void>(&RenderingAPI2D::Render2DText, t, std::ref(fnt), p, c, a, s, d, align, scissor);
    command.depth = d;
    renderManagerImpl->m_2DAPICommands.push_back(std::move(command));
}
void Renderer::renderBorder(const float borderSize, const glm::vec2& pos, const glm::vec4& col, const float w, const float h, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    const float doubleBorder = borderSize * 2.0f;
    const float halfWidth    = w / 2.0f;
    const float halfHeight   = h / 2.0f;

    float translationX = pos.x;
    float translationY = pos.y;
    Renderer::alignmentOffset(align, translationX, translationY, w, h);
    glm::vec2 newPos(translationX, translationY);

    Renderer::renderRectangle(newPos - glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth,Alignment::Right, scissor);
    Renderer::renderRectangle(newPos + glm::vec2(halfWidth, 0.0f), col, borderSize, h + doubleBorder, angle, depth,Alignment::Left, scissor);
    Renderer::renderRectangle(newPos - glm::vec2(0.0f, halfHeight), col, w, borderSize, angle, depth,Alignment::TopCenter, scissor);
    Renderer::renderRectangle(newPos + glm::vec2(0.0f, halfHeight + borderSize), col, w, borderSize, angle, depth,Alignment::BottomCenter, scissor);
}

void Renderer::renderFullscreenQuad(const uint& w, const uint& h, const uint& startX, const uint& startY){
    renderManagerImpl->_renderFullscreenQuad(w,h,startX,startY); 
}
void Renderer::renderFullscreenTriangle(const uint& w, const uint& h, const uint& startX, const uint& startY){
    renderManagerImpl->_renderFullscreenTriangle(w,h,startX,startY); 
}
void Renderer::renderFullscreenQuad() {
    const auto& size = Resources::getWindowSize();
    renderManagerImpl->_renderFullscreenQuad(size.x, size.y, 0, 0);
}
void Renderer::renderFullscreenTriangle() {
    const auto& size = Resources::getWindowSize();
    renderManagerImpl->_renderFullscreenTriangle(size.x, size.y, 0, 0);
}
