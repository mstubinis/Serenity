#include <core/engine/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/renderer/FullscreenItems.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
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
#include <core/ModelInstance.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/materials/Material.h>
#include <ecs/ComponentBody.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include <queue>

#include <iostream>

#include <boost/function.hpp>

typedef boost::function<void()> boost_func;


using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

Mesh* epriv::InternalMeshes::PointLightBounds    = nullptr;
Mesh* epriv::InternalMeshes::RodLightBounds      = nullptr;
Mesh* epriv::InternalMeshes::SpotLightBounds     = nullptr;

epriv::RenderManager* renderManager;
epriv::RenderManager::impl* renderManagerImpl;

uint epriv::RenderManager::GLSL_VERSION;
uint epriv::RenderManager::OPENGL_VERSION;

namespace Engine{
    namespace epriv{

        struct srtKeyShaderProgram final{inline bool operator() ( ShaderProgram* _1,  ShaderProgram* _2){return (_1->name() < _2->name());}};

        struct EngineInternalShaders final{enum Shader{
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
            DeferredFragSkyboxFake,
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
            LightingFragOptimized,
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

        _TOTAL};};
        struct EngineInternalShaderPrograms final{enum Program{
            //Deferred, //using the internal resource static one instead
            //Forward, //using the internal resource static one instead
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
            DeferredSkyboxFake,
            CopyDepth,
            DeferredLighting,
            DeferredLightingOptimized,
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


class epriv::RenderManager::impl final{
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

        vector<boost_func> m_2DAPICommands;

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
        vector<Shader> m_InternalShaders;
        vector<ShaderProgram*> m_InternalShaderPrograms;

        epriv::UBOCamera m_UBOCameraData;
        #pragma endregion

        void _init(const char* name, const uint& w, const uint& h){
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
            m_2DProjectionMatrix = glm::ortho(0.0f,float(w),0.0f,float(h),0.005f,1000.0f);
            m_IdentityMat4 = glm::mat4(1.0f);
            m_IdentityMat3 = glm::mat3(1.0f);
            m_RotationAxis2D = glm::vec3(0, 0, 1);
            #pragma endregion
        }      
        void _postInit(const char* name, const uint& width, const uint& height) {
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &UniformBufferObject::MAX_UBO_BINDINGS);
#pragma region OpenGLExtensions

            renderManager->OpenGLExtensionsManager.INIT();

#pragma endregion

            epriv::EShaders::init();

#pragma region EngineInternalShaderUBOs
            UniformBufferObject::UBO_CAMERA = new UniformBufferObject("Camera", sizeof(epriv::UBOCamera));
            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
#pragma endregion

#pragma region EngineInternalShadersAndPrograms
            m_InternalShaderPrograms.resize(EngineInternalShaderPrograms::_TOTAL, nullptr);

            m_InternalShaders.emplace_back(EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::fxaa_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::bullet_physics_vert, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::bullet_physcis_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::vertex_basic, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::vertex_2DAPI, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::vertex_skybox, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::lighting_vert, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::forward_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::zprepass_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_hud, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_skybox, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_skybox_fake, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::copy_depth_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::ssao_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::bloom_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::hdr_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::blur_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::depth_of_field, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::ssao_blur_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::godRays_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::final_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::depth_and_transparency_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag_optimized, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag_gi, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::cubemap_convolude_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::cubemap_prefilter_envmap_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::brdf_precompute, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::greyscale_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::stencil_passover, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_1, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_2, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_3, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_4, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_1, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_2, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_3, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_4, ShaderType::Fragment, false);

            ShaderProgram::Deferred = new ShaderProgram("Deferred", m_InternalShaders[EngineInternalShaders::VertexBasic], m_InternalShaders[EngineInternalShaders::DeferredFrag]);
            ShaderProgram::Forward = new ShaderProgram("Forward", m_InternalShaders[EngineInternalShaders::VertexBasic], m_InternalShaders[EngineInternalShaders::ForwardFrag]);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics] = new ShaderProgram("Bullet_Physics", m_InternalShaders[EngineInternalShaders::BulletPhysicsVertex], m_InternalShaders[EngineInternalShaders::BulletPhysicsFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::ZPrepass] = new ShaderProgram("ZPrepass", m_InternalShaders[EngineInternalShaders::VertexBasic], m_InternalShaders[EngineInternalShaders::ZPrepassFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Deferred2DAPI] = new ShaderProgram("Deferred_2DAPI", m_InternalShaders[EngineInternalShaders::Vertex2DAPI], m_InternalShaders[EngineInternalShaders::DeferredFrag2DAPI]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredGodRays] = new ShaderProgram("Deferred_GodsRays", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::GodRaysFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur] = new ShaderProgram("Deferred_Blur", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::BlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlurSSAO] = new ShaderProgram("Deferred_Blur_SSAO", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::SSAOBlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHDR] = new ShaderProgram("Deferred_HDR", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::HDRFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO] = new ShaderProgram("Deferred_SSAO", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::SSAOFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF] = new ShaderProgram("Deferred_DOF", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::DOFFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBloom] = new ShaderProgram("Deferred_Bloom", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::BloomFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal] = new ShaderProgram("Deferred_Final", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::FinalFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DepthAndTransparency] = new ShaderProgram("DepthAndTransparency", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::DepthAndTransparencyFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA] = new ShaderProgram("Deferred_FXAA", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::FXAAFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox] = new ShaderProgram("Deferred_Skybox", m_InternalShaders[EngineInternalShaders::VertexSkybox], m_InternalShaders[EngineInternalShaders::DeferredFragSkybox]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkyboxFake] = new ShaderProgram("Deferred_Skybox_Fake", m_InternalShaders[EngineInternalShaders::VertexSkybox], m_InternalShaders[EngineInternalShaders::DeferredFragSkyboxFake]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth] = new ShaderProgram("Copy_Depth", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::CopyDepthFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting] = new ShaderProgram("Deferred_Light", m_InternalShaders[EngineInternalShaders::LightingVertex], m_InternalShaders[EngineInternalShaders::LightingFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingOptimized] = new ShaderProgram("Deferred_Light_Optimized", m_InternalShaders[EngineInternalShaders::LightingVertex], m_InternalShaders[EngineInternalShaders::LightingFragOptimized]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI] = new ShaderProgram("Deferred_Light_GI", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::LightingGIFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude] = new ShaderProgram("Cubemap_Convolude", m_InternalShaders[EngineInternalShaders::VertexSkybox], m_InternalShaders[EngineInternalShaders::CubemapConvoludeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv] = new ShaderProgram("Cubemap_Prefilter_Env", m_InternalShaders[EngineInternalShaders::VertexSkybox], m_InternalShaders[EngineInternalShaders::CubemapPrefilterEnvFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance] = new ShaderProgram("BRDF_Precompute_CookTorrance", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::BRDFPrecomputeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Grayscale] = new ShaderProgram("Greyscale_Frag", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::GrayscaleFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass] = new ShaderProgram("Stencil_Pass", m_InternalShaders[EngineInternalShaders::LightingVertex], m_InternalShaders[EngineInternalShaders::StencilPassFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA1] = new ShaderProgram("Deferred_SMAA_1", m_InternalShaders[EngineInternalShaders::SMAAVertex1], m_InternalShaders[EngineInternalShaders::SMAAFrag1]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA2] = new ShaderProgram("Deferred_SMAA_2", m_InternalShaders[EngineInternalShaders::SMAAVertex2], m_InternalShaders[EngineInternalShaders::SMAAFrag2]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA3] = new ShaderProgram("Deferred_SMAA_3", m_InternalShaders[EngineInternalShaders::SMAAVertex3], m_InternalShaders[EngineInternalShaders::SMAAFrag3]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA4] = new ShaderProgram("Deferred_SMAA_4", m_InternalShaders[EngineInternalShaders::SMAAVertex4], m_InternalShaders[EngineInternalShaders::SMAAFrag4]);

#pragma endregion

#pragma region MeshData

            string triangleMesh = 
                "v 0.0 -0.948008 0.0\n"
                "v -1.0 0.689086 0.0\n"
                "v 1.0 0.689086 0.0\n"
                "vt 0.5 0.0\n"
                "vt 1.0 1.0\n"
                "vt 0.0 1.0\n"
                "vn 0.0 0.0 1.0\n"
                "f 1/1/1 3/2/1 2/3/1";

            string cubeMesh =
                "v 1.0 -1.0 -1.0\n"
                "v 1.0 -1.0 1.0\n"
                "v -1.0 -1.0 1.0\n"
                "v -1.0 -1.0 -1.0\n"
                "v 1.0 1.0 -1.0\n"
                "v 1.0 1.0 1.0\n"
                "v -1.0 1.0 1.0\n"
                "v -1.0 1.0 -1.0\n"
                "vt 1.0 0.0\n"
                "vt 0.0 1.0\n"
                "vt 0.0 0.0\n"
                "vt 1.0 0.0\n"
                "vt 0.0 1.0\n"
                "vt 0.0 0.0\n"
                "vt 1.0 0.0\n"
                "vt 0.0 1.0\n"
                "vt 1.0 0.0\n"
                "vt 0.0 1.0\n"
                "vt 0.0 0.0\n"
                "vt 0.0 0.0\n"
                "vt 1.0 1.0\n"
                "vt 1.0 0.0\n"
                "vt 0.0 1.0\n"
                "vt 1.0 1.0\n"
                "vt 1.0 1.0\n"
                "vt 1.0 1.0\n"
                "vt 1.0 0.0\n"
                "vt 1.0 1.0\n"
                "vn 0.0 -1.0 0.0\n"
                "vn 0.0 1.0 0.0\n"
                "vn 1.0 -0.0 0.0\n"
                "vn 0.0 -0.0 1.0\n"
                "vn -1.0 -0.0 -0.0\n"
                "vn 0.0 0.0 -1.0\n"
                "f 2/1/1 4/2/1 1/3/1\n"
                "f 8/4/2 6/5/2 5/6/2\n"
                "f 5/7/3 2/8/3 1/3/3\n"
                "f 6/9/4 3/10/4 2/11/4\n"
                "f 3/12/5 8/13/5 4/2/5\n"
                "f 1/14/6 8/15/6 5/6/6\n"
                "f 2/1/1 3/16/1 4/2/1\n"
                "f 8/4/2 7/17/2 6/5/2\n"
                "f 5/7/3 6/18/3 2/8/3\n"
                "f 6/9/4 7/17/4 3/10/4\n"
                "f 3/12/5 7/19/5 8/13/5\n"
                "f 1/14/6 4/20/6 8/15/6";

            string pointLightMesh =
                "v 0.000000 -1.000000 0.000000\n"
                "v 0.723607 -0.447220 0.525725\n"
                "v -0.276388 -0.447220 0.850649\n"
                "v -0.894426 -0.447216 0.000000\n"
                "v -0.276388 -0.447220 -0.850649\n"
                "v 0.723607 -0.447220 -0.525725\n"
                "v 0.276388 0.447220 0.850649\n"
                "v -0.723607 0.447220 0.525725\n"
                "v -0.723607 0.447220 -0.525725\n"
                "v 0.276388 0.447220 -0.850649\n"
                "v 0.894426 0.447216 0.000000\n"
                "v 0.000000 1.000000 0.000000\n"
                "v -0.232822 -0.657519 0.716563\n"
                "v -0.162456 -0.850654 0.499995\n"
                "v -0.077607 -0.967950 0.238853\n"
                "v 0.203181 -0.967950 0.147618\n"
                "v 0.425323 -0.850654 0.309011\n"
                "v 0.609547 -0.657519 0.442856\n"
                "v 0.531941 -0.502302 0.681712\n"
                "v 0.262869 -0.525738 0.809012\n"
                "v -0.029639 -0.502302 0.864184\n"
                "v 0.812729 -0.502301 -0.295238\n"
                "v 0.850648 -0.525736 0.000000\n"
                "v 0.812729 -0.502301 0.295238\n"
                "v 0.203181 -0.967950 -0.147618\n"
                "v 0.425323 -0.850654 -0.309011\n"
                "v 0.609547 -0.657519 -0.442856\n"
                "v -0.753442 -0.657515 0.000000\n"
                "v -0.525730 -0.850652 0.000000\n"
                "v -0.251147 -0.967949 0.000000\n"
                "v -0.483971 -0.502302 0.716565\n"
                "v -0.688189 -0.525736 0.499997\n"
                "v -0.831051 -0.502299 0.238853\n"
                "v -0.232822 -0.657519 -0.716563\n"
                "v -0.162456 -0.850654 -0.499995\n"
                "v -0.077607 -0.967950 -0.238853\n"
                "v -0.831051 -0.502299 -0.238853\n"
                "v -0.688189 -0.525736 -0.499997\n"
                "v -0.483971 -0.502302 -0.716565\n"
                "v -0.029639 -0.502302 -0.864184\n"
                "v 0.262869 -0.525738 -0.809012\n"
                "v 0.531941 -0.502302 -0.681712\n"
                "v 0.956626 0.251149 0.147618\n"
                "v 0.951058 -0.000000 0.309013\n"
                "v 0.860698 -0.251151 0.442858\n"
                "v 0.860698 -0.251151 -0.442858\n"
                "v 0.951058 0.000000 -0.309013\n"
                "v 0.956626 0.251149 -0.147618\n"
                "v 0.155215 0.251152 0.955422\n"
                "v 0.000000 -0.000000 1.000000\n"
                "v -0.155215 -0.251152 0.955422\n"
                "v 0.687159 -0.251152 0.681715\n"
                "v 0.587786 0.000000 0.809017\n"
                "v 0.436007 0.251152 0.864188\n"
                "v -0.860698 0.251151 0.442858\n"
                "v -0.951058 -0.000000 0.309013\n"
                "v -0.956626 -0.251149 0.147618\n"
                "v -0.436007 -0.251152 0.864188\n"
                "v -0.587786 0.000000 0.809017\n"
                "v -0.687159 0.251152 0.681715\n"
                "v -0.687159 0.251152 -0.681715\n"
                "v -0.587786 -0.000000 -0.809017\n"
                "v -0.436007 -0.251152 -0.864188\n"
                "v -0.956626 -0.251149 -0.147618\n"
                "v -0.951058 0.000000 -0.309013\n"
                "v -0.860698 0.251151 -0.442858\n"
                "v 0.436007 0.251152 -0.864188\n"
                "v 0.587786 -0.000000 -0.809017\n"
                "v 0.687159 -0.251152 -0.681715\n"
                "v -0.155215 -0.251152 -0.955422\n"
                "v 0.000000 0.000000 -1.000000\n"
                "v 0.155215 0.251152 -0.955422\n"
                "v 0.831051 0.502299 0.238853\n"
                "v 0.688189 0.525736 0.499997\n"
                "v 0.483971 0.502302 0.716565\n"
                "v 0.029639 0.502302 0.864184\n"
                "v -0.262869 0.525738 0.809012\n"
                "v -0.531941 0.502302 0.681712\n"
                "v -0.812729 0.502301 0.295238\n"
                "v -0.850648 0.525736 0.000000\n"
                "v -0.812729 0.502301 -0.295238\n"
                "v -0.531941 0.502302 -0.681712\n"
                "v -0.262869 0.525738 -0.809012\n"
                "v 0.029639 0.502302 -0.864184\n"
                "v 0.483971 0.502302 -0.716565\n"
                "v 0.688189 0.525736 -0.499997\n"
                "v 0.831051 0.502299 -0.238853\n"
                "v 0.077607 0.967950 0.238853\n"
                "v 0.162456 0.850654 0.499995\n"
                "v 0.232822 0.657519 0.716563\n"
                "v 0.753442 0.657515 0.000000\n"
                "v 0.525730 0.850652 0.000000\n"
                "v 0.251147 0.967949 0.000000\n"
                "v -0.203181 0.967950 0.147618\n"
                "v -0.425323 0.850654 0.309011\n"
                "v -0.609547 0.657519 0.442856\n"
                "v -0.203181 0.967950 -0.147618\n"
                "v -0.425323 0.850654 -0.309011\n"
                "v -0.609547 0.657519 -0.442856\n"
                "v 0.077607 0.967950 -0.238853\n"
                "v 0.162456 0.850654 -0.499995\n"
                "v 0.232822 0.657519 -0.716563\n"
                "v 0.361800 0.894429 -0.262863\n"
                "v 0.638194 0.723610 -0.262864\n"
                "v 0.447209 0.723612 -0.525728\n"
                "v -0.138197 0.894430 -0.425319\n"
                "v -0.052790 0.723612 -0.688185\n"
                "v -0.361804 0.723612 -0.587778\n"
                "v -0.447210 0.894429 0.000000\n"
                "v -0.670817 0.723611 -0.162457\n"
                "v -0.670817 0.723611 0.162457\n"
                "v -0.138197 0.894430 0.425319\n"
                "v -0.361804 0.723612 0.587778\n"
                "v -0.052790 0.723612 0.688185\n"
                "v 0.361800 0.894429 0.262863\n"
                "v 0.447209 0.723612 0.525728\n"
                "v 0.638194 0.723610 0.262864\n"
                "v 0.861804 0.276396 -0.425322\n"
                "v 0.809019 0.000000 -0.587782\n"
                "v 0.670821 0.276397 -0.688189\n"
                "v -0.138199 0.276397 -0.951055\n"
                "v -0.309016 -0.000000 -0.951057\n"
                "v -0.447215 0.276397 -0.850649\n"
                "v -0.947213 0.276396 -0.162458\n"
                "v -1.000000 0.000001 0.000000\n"
                "v -0.947213 0.276397 0.162458\n"
                "v -0.447216 0.276397 0.850648\n"
                "v -0.309017 -0.000001 0.951056\n"
                "v -0.138199 0.276397 0.951055\n"
                "v 0.670820 0.276396 0.688190\n"
                "v 0.809019 -0.000002 0.587783\n"
                "v 0.861804 0.276394 0.425323\n"
                "v 0.309017 -0.000000 -0.951056\n"
                "v 0.447216 -0.276398 -0.850648\n"
                "v 0.138199 -0.276398 -0.951055\n"
                "v -0.809018 -0.000000 -0.587783\n"
                "v -0.670819 -0.276397 -0.688191\n"
                "v -0.861803 -0.276396 -0.425324\n"
                "v -0.809018 0.000000 0.587783\n"
                "v -0.861803 -0.276396 0.425324\n"
                "v -0.670819 -0.276397 0.688191\n"
                "v 0.309017 0.000000 0.951056\n"
                "v 0.138199 -0.276398 0.951055\n"
                "v 0.447216 -0.276398 0.850648\n"
                "v 1.000000 0.000000 0.000000\n"
                "v 0.947213 -0.276396 0.162458\n"
                "v 0.947213 -0.276396 -0.162458\n"
                "v 0.361803 -0.723612 -0.587779\n"
                "v 0.138197 -0.894429 -0.425321\n"
                "v 0.052789 -0.723611 -0.688186\n"
                "v -0.447211 -0.723612 -0.525727\n"
                "v -0.361801 -0.894429 -0.262863\n"
                "v -0.638195 -0.723609 -0.262863\n"
                "v -0.638195 -0.723609 0.262864\n"
                "v -0.361801 -0.894428 0.262864\n"
                "v -0.447211 -0.723610 0.525729\n"
                "v 0.670817 -0.723611 -0.162457\n"
                "v 0.670818 -0.723610 0.162458\n"
                "v 0.447211 -0.894428 0.000001\n"
                "v 0.052790 -0.723612 0.688185\n"
                "v 0.138199 -0.894429 0.425321\n"
                "v 0.361805 -0.723611 0.587779\n"
                "f 1 16 15\n"
                "f 2 18 24\n"
                "f 1 15 30\n"
                "f 1 30 36\n"
                "f 1 36 25\n"
                "f 2 24 45\n"
                "f 3 21 51\n"
                "f 4 33 57\n"
                "f 5 39 63\n"
                "f 6 42 69\n"
                "f 2 45 52\n"
                "f 3 51 58\n"
                "f 4 57 64\n"
                "f 5 63 70\n"
                "f 6 69 46\n"
                "f 7 75 90\n"
                "f 8 78 96\n"
                "f 9 81 99\n"
                "f 10 84 102\n"
                "f 11 87 91\n"
                "f 93 100 12\n"
                "f 92 103 93\n"
                "f 91 104 92\n"
                "f 93 103 100\n"
                "f 103 101 100\n"
                "f 92 104 103\n"
                "f 104 105 103\n"
                "f 103 105 101\n"
                "f 105 102 101\n"
                "f 91 87 104\n"
                "f 87 86 104\n"
                "f 104 86 105\n"
                "f 86 85 105\n"
                "f 105 85 102\n"
                "f 85 10 102\n"
                "f 100 97 12\n"
                "f 101 106 100\n"
                "f 102 107 101\n"
                "f 100 106 97\n"
                "f 106 98 97\n"
                "f 101 107 106\n"
                "f 107 108 106\n"
                "f 106 108 98\n"
                "f 108 99 98\n"
                "f 102 84 107\n"
                "f 84 83 107\n"
                "f 107 83 108\n"
                "f 83 82 108\n"
                "f 108 82 99\n"
                "f 82 9 99\n"
                "f 97 94 12\n"
                "f 98 109 97\n"
                "f 99 110 98\n"
                "f 97 109 94\n"
                "f 109 95 94\n"
                "f 98 110 109\n"
                "f 110 111 109\n"
                "f 109 111 95\n"
                "f 111 96 95\n"
                "f 99 81 110\n"
                "f 81 80 110\n"
                "f 110 80 111\n"
                "f 80 79 111\n"
                "f 111 79 96\n"
                "f 79 8 96\n"
                "f 94 88 12\n"
                "f 95 112 94\n"
                "f 96 113 95\n"
                "f 94 112 88\n"
                "f 112 89 88\n"
                "f 95 113 112\n"
                "f 113 114 112\n"
                "f 112 114 89\n"
                "f 114 90 89\n"
                "f 96 78 113\n"
                "f 78 77 113\n"
                "f 113 77 114\n"
                "f 77 76 114\n"
                "f 114 76 90\n"
                "f 76 7 90\n"
                "f 88 93 12\n"
                "f 89 115 88\n"
                "f 90 116 89\n"
                "f 88 115 93\n"
                "f 115 92 93\n"
                "f 89 116 115\n"
                "f 116 117 115\n"
                "f 115 117 92\n"
                "f 117 91 92\n"
                "f 90 75 116\n"
                "f 75 74 116\n"
                "f 116 74 117\n"
                "f 74 73 117\n"
                "f 117 73 91\n"
                "f 73 11 91\n"
                "f 48 87 11\n"
                "f 47 118 48\n"
                "f 46 119 47\n"
                "f 48 118 87\n"
                "f 118 86 87\n"
                "f 47 119 118\n"
                "f 119 120 118\n"
                "f 118 120 86\n"
                "f 120 85 86\n"
                "f 46 69 119\n"
                "f 69 68 119\n"
                "f 119 68 120\n"
                "f 68 67 120\n"
                "f 120 67 85\n"
                "f 67 10 85\n"
                "f 72 84 10\n"
                "f 71 121 72\n"
                "f 70 122 71\n"
                "f 72 121 84\n"
                "f 121 83 84\n"
                "f 71 122 121\n"
                "f 122 123 121\n"
                "f 121 123 83\n"
                "f 123 82 83\n"
                "f 70 63 122\n"
                "f 63 62 122\n"
                "f 122 62 123\n"
                "f 62 61 123\n"
                "f 123 61 82\n"
                "f 61 9 82\n"
                "f 66 81 9\n"
                "f 65 124 66\n"
                "f 64 125 65\n"
                "f 66 124 81\n"
                "f 124 80 81\n"
                "f 65 125 124\n"
                "f 125 126 124\n"
                "f 124 126 80\n"
                "f 126 79 80\n"
                "f 64 57 125\n"
                "f 57 56 125\n"
                "f 125 56 126\n"
                "f 56 55 126\n"
                "f 126 55 79\n"
                "f 55 8 79\n"
                "f 60 78 8\n"
                "f 59 127 60\n"
                "f 58 128 59\n"
                "f 60 127 78\n"
                "f 127 77 78\n"
                "f 59 128 127\n"
                "f 128 129 127\n"
                "f 127 129 77\n"
                "f 129 76 77\n"
                "f 58 51 128\n"
                "f 51 50 128\n"
                "f 128 50 129\n"
                "f 50 49 129\n"
                "f 129 49 76\n"
                "f 49 7 76\n"
                "f 54 75 7\n"
                "f 53 130 54\n"
                "f 52 131 53\n"
                "f 54 130 75\n"
                "f 130 74 75\n"
                "f 53 131 130\n"
                "f 131 132 130\n"
                "f 130 132 74\n"
                "f 132 73 74\n"
                "f 52 45 131\n"
                "f 45 44 131\n"
                "f 131 44 132\n"
                "f 44 43 132\n"
                "f 132 43 73\n"
                "f 43 11 73\n"
                "f 67 72 10\n"
                "f 68 133 67\n"
                "f 69 134 68\n"
                "f 67 133 72\n"
                "f 133 71 72\n"
                "f 68 134 133\n"
                "f 134 135 133\n"
                "f 133 135 71\n"
                "f 135 70 71\n"
                "f 69 42 134\n"
                "f 42 41 134\n"
                "f 134 41 135\n"
                "f 41 40 135\n"
                "f 135 40 70\n"
                "f 40 5 70\n"
                "f 61 66 9\n"
                "f 62 136 61\n"
                "f 63 137 62\n"
                "f 61 136 66\n"
                "f 136 65 66\n"
                "f 62 137 136\n"
                "f 137 138 136\n"
                "f 136 138 65\n"
                "f 138 64 65\n"
                "f 63 39 137\n"
                "f 39 38 137\n"
                "f 137 38 138\n"
                "f 38 37 138\n"
                "f 138 37 64\n"
                "f 37 4 64\n"
                "f 55 60 8\n"
                "f 56 139 55\n"
                "f 57 140 56\n"
                "f 55 139 60\n"
                "f 139 59 60\n"
                "f 56 140 139\n"
                "f 140 141 139\n"
                "f 139 141 59\n"
                "f 141 58 59\n"
                "f 57 33 140\n"
                "f 33 32 140\n"
                "f 140 32 141\n"
                "f 32 31 141\n"
                "f 141 31 58\n"
                "f 31 3 58\n"
                "f 49 54 7\n"
                "f 50 142 49\n"
                "f 51 143 50\n"
                "f 49 142 54\n"
                "f 142 53 54\n"
                "f 50 143 142\n"
                "f 143 144 142\n"
                "f 142 144 53\n"
                "f 144 52 53\n"
                "f 51 21 143\n"
                "f 21 20 143\n"
                "f 143 20 144\n"
                "f 20 19 144\n"
                "f 144 19 52\n"
                "f 19 2 52\n"
                "f 43 48 11\n"
                "f 44 145 43\n"
                "f 45 146 44\n"
                "f 43 145 48\n"
                "f 145 47 48\n"
                "f 44 146 145\n"
                "f 146 147 145\n"
                "f 145 147 47\n"
                "f 147 46 47\n"
                "f 45 24 146\n"
                "f 24 23 146\n"
                "f 146 23 147\n"
                "f 23 22 147\n"
                "f 147 22 46\n"
                "f 22 6 46\n"
                "f 27 42 6\n"
                "f 26 148 27\n"
                "f 25 149 26\n"
                "f 27 148 42\n"
                "f 148 41 42\n"
                "f 26 149 148\n"
                "f 149 150 148\n"
                "f 148 150 41\n"
                "f 150 40 41\n"
                "f 25 36 149\n"
                "f 36 35 149\n"
                "f 149 35 150\n"
                "f 35 34 150\n"
                "f 150 34 40\n"
                "f 34 5 40\n"
                "f 34 39 5\n"
                "f 35 151 34\n"
                "f 36 152 35\n"
                "f 34 151 39\n"
                "f 151 38 39\n"
                "f 35 152 151\n"
                "f 152 153 151\n"
                "f 151 153 38\n"
                "f 153 37 38\n"
                "f 36 30 152\n"
                "f 30 29 152\n"
                "f 152 29 153\n"
                "f 29 28 153\n"
                "f 153 28 37\n"
                "f 28 4 37\n"
                "f 28 33 4\n"
                "f 29 154 28\n"
                "f 30 155 29\n"
                "f 28 154 33\n"
                "f 154 32 33\n"
                "f 29 155 154\n"
                "f 155 156 154\n"
                "f 154 156 32\n"
                "f 156 31 32\n"
                "f 30 15 155\n"
                "f 15 14 155\n"
                "f 155 14 156\n"
                "f 14 13 156\n"
                "f 156 13 31\n"
                "f 13 3 31\n"
                "f 22 27 6\n"
                "f 23 157 22\n"
                "f 24 158 23\n"
                "f 22 157 27\n"
                "f 157 26 27\n"
                "f 23 158 157\n"
                "f 158 159 157\n"
                "f 157 159 26\n"
                "f 159 25 26\n"
                "f 24 18 158\n"
                "f 18 17 158\n"
                "f 158 17 159\n"
                "f 17 16 159\n"
                "f 159 16 25\n"
                "f 16 1 25\n"
                "f 13 21 3\n"
                "f 14 160 13\n"
                "f 15 161 14\n"
                "f 13 160 21\n"
                "f 160 20 21\n"
                "f 14 161 160\n"
                "f 161 162 160\n"
                "f 160 162 20\n"
                "f 162 19 20\n"
                "f 15 16 161\n"
                "f 16 17 161\n"
                "f 161 17 162\n"
                "f 17 18 162\n"
                "f 162 18 19\n"
                "f 18 2 19";

            string spotLightData =
                "v 0.000000 1.000000 -1.000000\n"
                "v 0.222521 0.974928 -1.000000\n"
                "v 0.433884 0.900969 -1.000000\n"
                "v 0.623490 0.781831 -1.000000\n"
                "v 0.781831 0.623490 -1.000000\n"
                "v 0.900969 0.433884 -1.000000\n"
                "v 0.000000 -0.000000 0.000000\n"
                "v 0.974928 0.222521 -1.000000\n"
                "v 1.000000 -0.000000 -1.000000\n"
                "v 0.974928 -0.222521 -1.000000\n"
                "v 0.900969 -0.433884 -1.000000\n"
                "v 0.781831 -0.623490 -1.000000\n"
                "v 0.623490 -0.781832 -1.000000\n"
                "v 0.433884 -0.900969 -1.000000\n"
                "v 0.222521 -0.974928 -1.000000\n"
                "v -0.000000 -1.000000 -1.000000\n"
                "v -0.222521 -0.974928 -1.000000\n"
                "v -0.433884 -0.900969 -1.000000\n"
                "v -0.623490 -0.781831 -1.000000\n"
                "v -0.781832 -0.623489 -1.000000\n"
                "v -0.900969 -0.433883 -1.000000\n"
                "v -0.974928 -0.222520 -1.000000\n"
                "v -1.000000 0.000001 -1.000000\n"
                "v -0.974928 0.222522 -1.000000\n"
                "v -0.900968 0.433885 -1.000000\n"
                "v -0.781831 0.623491 -1.000000\n"
                "v -0.623489 0.781832 -1.000000\n"
                "v -0.433882 0.900969 -1.000000\n"
                "v -0.222519 0.974928 -1.000000\n"
                "f 1 7 2\n"
                "f 2 7 3\n"
                "f 3 7 4\n"
                "f 4 7 5\n"
                "f 5 7 6\n"
                "f 6 7 8\n"
                "f 8 7 9\n"
                "f 9 7 10\n"
                "f 10 7 11\n"
                "f 11 7 12\n"
                "f 12 7 13\n"
                "f 13 7 14\n"
                "f 14 7 15\n"
                "f 15 7 16\n"
                "f 16 7 17\n"
                "f 17 7 18\n"
                "f 18 7 19\n"
                "f 19 7 20\n"
                "f 20 7 21\n"
                "f 21 7 22\n"
                "f 22 7 23\n"
                "f 23 7 24\n"
                "f 24 7 25\n"
                "f 25 7 26\n"
                "f 26 7 27\n"
                "f 27 7 28\n"
                "f 28 7 29\n"
                "f 29 7 1\n"
                "f 17 1 16\n"
                "f 1 15 16\n"
                "f 3 15 2\n"
                "f 4 14 3\n"
                "f 18 29 17\n"
                "f 19 28 18\n"
                "f 19 26 27\n"
                "f 20 25 26\n"
                "f 22 25 21\n"
                "f 23 24 22\n"
                "f 5 13 4\n"
                "f 6 12 5\n"
                "f 8 11 6\n"
                "f 9 10 8\n"
                "f 17 29 1\n"
                "f 1 2 15\n"
                "f 3 14 15\n"
                "f 4 13 14\n"
                "f 18 28 29\n"
                "f 19 27 28\n"
                "f 19 20 26\n"
                "f 20 21 25\n"
                "f 22 24 25\n"
                "f 5 12 13\n"
                "f 6 11 12\n"
                "f 8 10 11";

            string rodLightData =
                "v -0.000000 1.000000 -1.000000\n"
                "v -0.000000 1.000000 1.000000\n"
                "v 0.284630 0.959493 -1.000000\n"
                "v 0.284630 0.959493 1.000000\n"
                "v 0.546200 0.841254 -1.000000\n"
                "v 0.546200 0.841254 1.000000\n"
                "v 0.763521 0.654861 -1.000000\n"
                "v 0.763521 0.654861 1.000000\n"
                "v 0.918986 0.415415 -1.000000\n"
                "v 0.918986 0.415415 1.000000\n"
                "v 1.000000 0.142315 -1.000000\n"
                "v 1.000000 0.142315 1.000000\n"
                "v 1.000000 -0.142315 -1.000000\n"
                "v 1.000000 -0.142315 1.000000\n"
                "v 0.918986 -0.415415 -1.000000\n"
                "v 0.918986 -0.415415 1.000000\n"
                "v 0.763521 -0.654860 -1.000000\n"
                "v 0.763521 -0.654861 1.000000\n"
                "v 0.546200 -0.841253 -1.000000\n"
                "v 0.546200 -0.841253 1.000000\n"
                "v 0.284630 -0.959493 -1.000000\n"
                "v 0.284630 -0.959493 1.000000\n"
                "v 0.000000 -1.000000 -1.000000\n"
                "v 0.000000 -1.000000 1.000000\n"
                "v -0.284629 -0.959493 -1.000000\n"
                "v -0.284629 -0.959493 1.000000\n"
                "v -0.546200 -0.841253 -1.000000\n"
                "v -0.546200 -0.841254 1.000000\n"
                "v -0.763521 -0.654861 -1.000000\n"
                "v -0.763521 -0.654861 1.000000\n"
                "v -0.918986 -0.415415 -1.000000\n"
                "v -0.918986 -0.415415 1.000000\n"
                "v -1.000000 -0.142315 -1.000000\n"
                "v -1.000000 -0.142315 1.000000\n"
                "v -1.000000 0.142314 -1.000000\n"
                "v -1.000000 0.142314 1.000000\n"
                "v -0.918986 0.415415 -1.000000\n"
                "v -0.918986 0.415414 1.000000\n"
                "v -0.763522 0.654860 -1.000000\n"
                "v -0.763522 0.654860 1.000000\n"
                "v -0.546201 0.841253 -1.000000\n"
                "v -0.546201 0.841253 1.000000\n"
                "v -0.284631 0.959493 -1.000000\n"
                "v -0.284631 0.959493 1.000000\n"
                "f 2 3 1\n"
                "f 4 5 3\n"
                "f 6 7 5\n"
                "f 8 9 7\n"
                "f 10 11 9\n"
                "f 12 13 11\n"
                "f 14 15 13\n"
                "f 16 17 15\n"
                "f 18 19 17\n"
                "f 20 21 19\n"
                "f 21 24 23\n"
                "f 24 25 23\n"
                "f 26 27 25\n"
                "f 28 29 27\n"
                "f 29 32 31\n"
                "f 32 33 31\n"
                "f 34 35 33\n"
                "f 35 38 37\n"
                "f 38 39 37\n"
                "f 39 42 41\n"
                "f 25 1 23\n"
                "f 42 43 41\n"
                "f 44 1 43\n"
                "f 3 23 1\n"
                "f 27 43 25\n"
                "f 29 41 27\n"
                "f 31 39 29\n"
                "f 33 37 31\n"
                "f 5 21 3\n"
                "f 7 19 5\n"
                "f 9 17 7\n"
                "f 11 15 9\n"
                "f 2 22 4\n"
                "f 24 44 26\n"
                "f 26 42 28\n"
                "f 28 40 30\n"
                "f 30 38 32\n"
                "f 32 36 34\n"
                "f 6 22 20\n"
                "f 8 20 18\n"
                "f 8 16 10\n"
                "f 12 16 14\n"
                "f 2 4 3\n"
                "f 4 6 5\n"
                "f 6 8 7\n"
                "f 8 10 9\n"
                "f 10 12 11\n"
                "f 12 14 13\n"
                "f 14 16 15\n"
                "f 16 18 17\n"
                "f 18 20 19\n"
                "f 20 22 21\n"
                "f 21 22 24\n"
                "f 24 26 25\n"
                "f 26 28 27\n"
                "f 28 30 29\n"
                "f 29 30 32\n"
                "f 32 34 33\n"
                "f 34 36 35\n"
                "f 35 36 38\n"
                "f 38 40 39\n"
                "f 39 40 42\n"
                "f 25 43 1\n"
                "f 42 44 43\n"
                "f 44 2 1\n"
                "f 3 21 23\n"
                "f 27 41 43\n"
                "f 29 39 41\n"
                "f 31 37 39\n"
                "f 33 35 37\n"
                "f 5 19 21\n"
                "f 7 17 19\n"
                "f 9 15 17\n"
                "f 11 13 15\n"
                "f 2 24 22\n"
                "f 24 2 44\n"
                "f 26 44 42\n"
                "f 28 42 40\n"
                "f 30 40 38\n"
                "f 32 38 36\n"
                "f 6 4 22\n"
                "f 8 6 20\n"
                "f 8 18 16\n"
                "f 12 10 16";
#pragma endregion

            epriv::InternalMeshes::PointLightBounds = new Mesh(pointLightMesh, 0.0005f);
            epriv::InternalMeshes::RodLightBounds = new Mesh(rodLightData, 0.0005f);
            epriv::InternalMeshes::SpotLightBounds = new Mesh(spotLightData, 0.0005f);

            Mesh::Plane = new Mesh("Plane", 1.0f, 1.0f, 0.0005f);
            Mesh::Cube = new Mesh(cubeMesh, 0.0005f);
            Mesh::Triangle = new Mesh(triangleMesh, 0.0005f);

            Mesh::FontPlane = new Mesh("FontPlane", 1.0f, 1.0f, 0.0005f);
            auto& fontPlane = *Mesh::FontPlane;

            text_pts.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 points per char, 4096 chars
            text_uvs.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4);//4 uvs per char
            text_ind.reserve(Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6);//6 ind per char

            for (unsigned int i = 0; i < text_pts.capacity(); ++i)
                text_pts.emplace_back(0.0f);
            for (unsigned int i = 0; i < text_uvs.capacity(); ++i)
                text_uvs.emplace_back(0.0f);
            for (unsigned int i = 0; i < text_ind.capacity(); ++i)
                text_ind.emplace_back(0);

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
            Texture::White    = new Texture(sfImageWhite,    "WhiteTexturePlaceholder",    false, ImageInternalFormat::RGBA8);
            Texture::Black    = new Texture(sfImageBlack,    "BlackTexturePlaceholder",    false, ImageInternalFormat::RGBA8);
            Texture::Checkers = new Texture(sfImageCheckers, "CheckersTexturePlaceholder", false, ImageInternalFormat::RGBA8);
            Texture::Checkers->setFilter(TextureFilter::Nearest);
            Material::Checkers = new Material("MaterialDefaultCheckers", Texture::Checkers);
            Material::Checkers->setSpecularModel(SpecularModel::None);
            Material::Checkers->setSmoothness(0.0f);

            Texture::BRDF = new Texture(512,512,ImagePixelType::FLOAT,ImagePixelFormat::RG,ImageInternalFormat::RG16F);
            Texture::BRDF->setWrapping(TextureWrap::ClampToEdge);

            m_FullscreenQuad = new FullscreenQuad();
            m_FullscreenTriangle = new FullscreenTriangle();

            SSAO::ssao.init();

            renderManager->OpenGLStateMachine.GL_glEnable(GL_DEPTH_TEST);
            Renderer::setDepthFunc(GL_LEQUAL);
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

            SAFE_DELETE(epriv::InternalMeshes::PointLightBounds);
            SAFE_DELETE(epriv::InternalMeshes::RodLightBounds);
            SAFE_DELETE(epriv::InternalMeshes::SpotLightBounds);
            SAFE_DELETE(Mesh::FontPlane);
            SAFE_DELETE(Mesh::Plane);
            SAFE_DELETE(Mesh::Cube);
            SAFE_DELETE(Mesh::Triangle);

            SAFE_DELETE(Texture::White);
            SAFE_DELETE(Texture::Black);
            SAFE_DELETE(Texture::Checkers);
            SAFE_DELETE(Texture::BRDF);
            SAFE_DELETE(Material::Checkers);


            SAFE_DELETE(ShaderProgram::Deferred);
            SAFE_DELETE(ShaderProgram::Forward);

            SAFE_DELETE_VECTOR(m_InternalShaderPrograms);

            //TODO: add cleanup() from ssao / smaa here?
        }
        void _renderSkybox(Skybox* skybox, Scene& scene, Viewport& viewport, Camera& camera){
            glm::mat4 view = camera.getView();
            Math::removeMatrixPosition(view);
            if(skybox){
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox]->bind();
                sendUniformMatrix4("VP", camera.getProjection() * view);
                sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP);
                Skybox::bindMesh();
            }else{//render a fake skybox.
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkyboxFake]->bind();
                auto& bgColor = scene.getBackgroundColor();
                sendUniformMatrix4("VP", camera.getProjection() * view);
                sendUniform4("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                Skybox::bindMesh();
            }
            sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
            //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...
        }
        void _resize(const uint& w, const uint& h){
            m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h), 0.005f, 3000.0f);
            m_GBuffer->resize(w, h);
        }
        void _onFullscreen(sf::Window* sfWindow, const sf::VideoMode& videoMode,const char* winName, const uint& style, const sf::ContextSettings& settings){
            SAFE_DELETE(m_GBuffer);

            sfWindow->create(videoMode, winName, style, settings);

            //oh yea the opengl context is lost, gotta restore the state machine
            renderManager->OpenGLStateMachine.GL_RESTORE_CURRENT_STATE_MACHINE();

            GLEnable(GL_CULL_FACE);
            GLEnable(GL_DEPTH_CLAMP);

            const auto& winSize = Resources::getWindowSize();
            m_GBuffer = new GBuffer(winSize.x, winSize.y);
        }
        void _onOpenGLContextCreation(const uint& width, const uint& height, const uint& _glslVersion, const uint& _openglVersion){
            epriv::RenderManager::GLSL_VERSION = _glslVersion;
            epriv::RenderManager::OPENGL_VERSION = _openglVersion;
            GLEnable(GL_CULL_FACE);
            SAFE_DELETE(m_GBuffer);
            m_GBuffer = new GBuffer(width,height);
        }
        void _generatePBREnvMapData(Texture& texture, const uint& convoludeTextureSize, const uint& preEnvFilterSize){
            uint texType = texture.type();
            if(texType != GL_TEXTURE_CUBE_MAP){
                cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
            }
            uint size = convoludeTextureSize;
            Renderer::bindTextureForModification(texType, texture.address(1));
            Renderer::unbindFBO();
            epriv::FramebufferObject* fbo = new epriv::FramebufferObject(texture.name() + "_fbo_envData",size,size); //try without a depth format
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

            Renderer::sendTexture("cubemap",texture.address(),0,texType);
            Renderer::setViewport(0,0,size,size);
            for (uint i = 0; i < 6; ++i){
                const glm::mat4 vp = captureProjection * captureViews[i];
                Renderer::sendUniformMatrix4("VP",vp);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(1),0);
                Renderer::Settings::clear(true,true,false);
                Skybox::bindMesh();
            }
            Resources::getWindow().display(); //prevent opengl & windows timeout


            //now gen EnvPrefilterMap for specular IBL
            size = preEnvFilterSize;
            Renderer::bindTextureForModification(texType, texture.address(2));
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv]->bind();
            Renderer::sendTexture("cubemap",texture.address(),0,texType);
            Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix",12.56637f / float((texture.width() * texture.width())*6));
            Renderer::sendUniform1("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
                fbo->resize(mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                Renderer::sendUniform1("roughness",roughness);
                float a = roughness * roughness;
                Renderer::sendUniform1("a2",a*a);
                for (uint i = 0; i < 6; ++i){
                    glm::mat4 vp = captureProjection * captureViews[i];
                    Renderer::sendUniformMatrix4("VP", vp);
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(2),m);
                    Renderer::Settings::clear(true,true,false);
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

            FramebufferObject* fbo = new FramebufferObject("BRDFLUT_Gen_CookTorr_FBO", brdfSize, brdfSize); //try without a depth format
            fbo->bind();

            Renderer::bindTextureForModification(GL_TEXTURE_2D, Texture::BRDF->address());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
            Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToEdge);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture::BRDF->address(), 0);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance]->bind();
            Renderer::sendUniform1("NUM_SAMPLES", 256);
            Renderer::Settings::clear(true, true, false);
            Renderer::colorMask(true, true, false, false);
            Renderer::renderFullscreenTriangle(brdfSize, brdfSize);
            Renderer::colorMask(true, true, true, true);

            SAFE_DELETE(fbo);
            //Renderer::bindReadFBO(prevReadBuffer);
            //Renderer::bindDrawFBO(prevDrawBuffer);
        }
        void _renderTextLeft(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
            uint i = 0;
            for (auto& character : text) {
                //if ((text_ind.size() * 6) >= text_ind.capacity())
                    //return;
                if (character == '\n') {
                    y += newLineGlyphHeight + 7;
                    x = 0.0f;
                }else if (character != '\0') {
                    const uint& accum = i * 4;
                    ++i;
                    const FontGlyph& chr   = font.getGlyphData(character);
                    const float& startingY = -int(chr.height + chr.yoffset) - y;

                    text_ind.emplace_back(accum + 0);
                    text_ind.emplace_back(accum + 1);
                    text_ind.emplace_back(accum + 2);
                    text_ind.emplace_back(accum + 3);
                    text_ind.emplace_back(accum + 1);
                    text_ind.emplace_back(accum + 0);

                    const float& startingX = x + chr.xoffset;
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

            uint i = 0;
            for (auto& line : lines) {
                for (int j = line.size(); j >= 0; --j) {
                    const auto& character = line[j];
                    //if ((text_ind.size() * 6) >= text_ind.capacity())
                        //return;
                    if (character != '\0') {   
                        const uint& accum = i * 4;
                        ++i;
                        const FontGlyph& chr   = font.getGlyphData(character);
                        const float& startingY = -int(chr.height + chr.yoffset) - y;

                        text_ind.emplace_back(accum + 0);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 2);
                        text_ind.emplace_back(accum + 3);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 0);

                        const float& startingX = x + chr.xoffset;
                        x -= chr.xadvance;

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
                y += newLineGlyphHeight + 7;
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
            uint i = 0;
            for (uint l = 0; l < lines.size(); ++l) {
                const auto& line      = lines[l];
                const auto& line_size = lines_sizes[l] / 2;
                for (auto& character : line) {
                    //if ((text_ind.size() * 6) >= text_ind.capacity())
                        //return;
                    if (character != '\0') {
                        const uint& accum = i * 4;
                        ++i;
                        const FontGlyph& chr   = font.getGlyphData(character);
                        const float& startingY = -int(chr.height + chr.yoffset) - y;

                        text_ind.emplace_back(accum + 0);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 2);
                        text_ind.emplace_back(accum + 3);
                        text_ind.emplace_back(accum + 1);
                        text_ind.emplace_back(accum + 0);

                        const float& startingX = x + chr.xoffset;
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
                y += newLineGlyphHeight + 7;
                x = 0.0f;
            }
        }
        void _renderSunLight(Camera& c, SunLight& s) {
            if (!s.isActive()) return;
            auto& body = *s.getComponent<ComponentBody>();
            const glm::vec3& pos = body.position();
            Renderer::sendUniform4("light.DataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, 0.0f);
            Renderer::sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("light.DataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, static_cast<float>(s.m_Type));
            Renderer::sendUniform1Safe("Type", 0.0f);

            Renderer::renderFullscreenTriangle();
        }
        void _renderPointLight(Camera& c, PointLight& p) {
            if (!p.isActive()) return;
            auto& body = *p.getComponent<ComponentBody>();
            const auto& pos = body.position();
            const auto factor = 1100.0f * p.m_CullingRadius;
            if ((!c.sphereIntersectTest(pos, p.m_CullingRadius)) || (c.getDistanceSquared(pos) > factor * factor))
                return;
            Renderer::sendUniform4("light.DataA", p.m_AmbientIntensity, p.m_DiffuseIntensity, p.m_SpecularIntensity, 0.0f);
            Renderer::sendUniform4("light.DataB", 0.0f, 0.0f, p.m_C, p.m_L);
            Renderer::sendUniform4("light.DataC", p.m_E, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("light.DataD", p.m_Color.x, p.m_Color.y, p.m_Color.z, static_cast<float>(p.m_Type));
            Renderer::sendUniform4Safe("light.DataE", 0.0f, 0.0f, static_cast<float>(p.m_AttenuationModel), 0.0f);
            Renderer::sendUniform1Safe("Type", 1.0f);

            const glm::mat4& model = body.modelMatrix();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (c.getDistanceSquared(pos) <= (p.m_CullingRadius * p.m_CullingRadius)) { //inside the light volume
                Renderer::cullFace(GL_FRONT);
            }else{
                Renderer::cullFace(GL_BACK);
            }
            auto& pointLightMesh = *epriv::InternalMeshes::PointLightBounds;

            pointLightMesh.bind();
            pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            pointLightMesh.unbind();
            Renderer::cullFace(GL_BACK);
        }
        void _renderDirectionalLight(Camera& c, DirectionalLight& d) {
            if (!d.isActive()) return;
            auto& body = *d.getComponent<ComponentBody>();
            const glm::vec3& _forward = body.forward();
            Renderer::sendUniform4("light.DataA", d.m_AmbientIntensity, d.m_DiffuseIntensity, d.m_SpecularIntensity, _forward.x);
            Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, 0.0f, 0.0f);
            Renderer::sendUniform4("light.DataD", d.m_Color.x, d.m_Color.y, d.m_Color.z, static_cast<float>(d.m_Type));
            Renderer::sendUniform1Safe("Type", 0.0f);
            Renderer::renderFullscreenTriangle();
        }
        void _renderSpotLight(Camera& c, SpotLight& s) {
            if (!s.isActive()) return;
            auto& body = *s.m_Entity.getComponent<ComponentBody>();
            glm::vec3 pos = body.position();
            glm::vec3 _forward = body.forward();
            const auto factor = 1100.0f * s.m_CullingRadius;
            if (!c.sphereIntersectTest(pos, s.m_CullingRadius) || (c.getDistanceSquared(pos) > factor * factor))
                return;
            Renderer::sendUniform4("light.DataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, _forward.x);
            Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, s.m_C, s.m_L);
            Renderer::sendUniform4("light.DataC", s.m_E, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("light.DataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, static_cast<float>(s.m_Type));
            Renderer::sendUniform4Safe("light.DataE", s.m_Cutoff, s.m_OuterCutoff, static_cast<float>(s.m_AttenuationModel), 0.0f);
            Renderer::sendUniform2Safe("VertexShaderData", s.m_OuterCutoff, s.m_CullingRadius);
            Renderer::sendUniform1Safe("Type", 2.0f);

            const glm::mat4& model = body.modelMatrix();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (c.getDistanceSquared(pos) <= (s.m_CullingRadius * s.m_CullingRadius)) { //inside the light volume                                                 
                Renderer::cullFace(GL_FRONT);
            }else{
                Renderer::cullFace(GL_BACK);
            }
            auto& spotLightMesh = *epriv::InternalMeshes::SpotLightBounds;

            spotLightMesh.bind();
            spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            spotLightMesh.unbind();
            Renderer::cullFace(GL_BACK);

            Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
        }
        void _renderRodLight(Camera& c, RodLight& r) {
            if (!r.isActive()) return;
            auto& body = *r.m_Entity.getComponent<ComponentBody>();
            const glm::vec3& pos = body.position();
            float cullingDistance = r.m_RodLength + (r.m_CullingRadius * 2.0f);
            const auto factor = 1100.0f * cullingDistance;
            if (!c.sphereIntersectTest(pos, cullingDistance) || (c.getDistanceSquared(pos) > factor * factor))
                return;
            const float half = r.m_RodLength / 2.0f;
            const glm::vec3& firstEndPt = pos + (body.forward() * half);
            const glm::vec3& secndEndPt = pos - (body.forward() * half);
            Renderer::sendUniform4("light.DataA", r.m_AmbientIntensity, r.m_DiffuseIntensity, r.m_SpecularIntensity, firstEndPt.x);
            Renderer::sendUniform4("light.DataB", firstEndPt.y, firstEndPt.z, r.m_C, r.m_L);
            Renderer::sendUniform4("light.DataC", r.m_E, secndEndPt.x, secndEndPt.y, secndEndPt.z);
            Renderer::sendUniform4("light.DataD", r.m_Color.x, r.m_Color.y, r.m_Color.z, static_cast<float>(r.m_Type));
            Renderer::sendUniform4Safe("light.DataE", r.m_RodLength, 0.0f, static_cast<float>(r.m_AttenuationModel), 0.0f);
            Renderer::sendUniform1Safe("Type", 1.0f);

            const glm::mat4& model = body.modelMatrix();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

            if (c.getDistanceSquared(pos) <= (cullingDistance * cullingDistance)) {
                Renderer::cullFace(GL_FRONT);
            }else{
                Renderer::cullFace(GL_BACK);
            }
            auto& rodLightMesh = *epriv::InternalMeshes::RodLightBounds;

            rodLightMesh.bind();
            rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            rodLightMesh.unbind();
            Renderer::cullFace(GL_BACK);

            Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
        }
        

        void _passGeometry(const double& dt, GBuffer& gbuffer, Viewport& viewport, Camera& camera){
            Scene& scene = viewport.m_Scene;
            const glm::vec4& clear = viewport.m_BackgroundColor;
            const float colors[4] = { clear.r, clear.g, clear.b, clear.a };
    
            gbuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

            Settings::clear(true,true,true); // (0,0,0,0)
            
            Renderer::setDepthFunc(GL_LEQUAL);

            glClearBufferfv(GL_COLOR, 0, colors);
            auto& godRays = GodRays::godRays;
            if(godRays.godRays_active){
                const float godraysclearcolor[4] = { godRays.clearColor.r, godRays.clearColor.g, godRays.clearColor.b, godRays.clearColor.a };
                glClearBufferfv(GL_COLOR, 2, godraysclearcolor);
            }
            GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            GLEnablei(GL_BLEND, 0); //this is needed for sure
            InternalScenePublicInterface::RenderGeometryOpaque(scene, camera, dt);
            if (viewport.isSkyboxVisible()) {
                _renderSkybox(scene.skybox(), scene, viewport, camera);
            }
            InternalScenePublicInterface::RenderGeometryTransparent(scene, camera, dt);
            InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(scene, camera, dt, true);
        }
        void _passForwardRendering(const double& dt, GBuffer& gbuffer, Viewport& viewport, Camera& camera){
            Scene& scene = viewport.m_Scene;
            gbuffer.bindFramebuffers(GBufferType::Diffuse, GBufferType::Misc, GBufferType::Lighting, "RGBA");
            InternalScenePublicInterface::RenderForwardOpaque(scene, camera, dt);

            GLEnablei(GL_BLEND, 0); //this might need to be all buffers not just 0
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFuncSeparatei(0,GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); //this works too
            //glBlendFuncSeparatei(2, GL_ONE, GL_ONE, GL_ONE, GL_ONE); //this works too
            glDepthMask(GL_TRUE);

            GLEnablei(GL_BLEND, 1); //yes this is important
            GLEnablei(GL_BLEND, 2); //yes this is important
            InternalScenePublicInterface::RenderForwardTransparent(scene, camera, dt);
            InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(scene, camera, dt);

            glDepthMask(GL_FALSE);
            InternalScenePublicInterface::RenderForwardParticles(scene, camera, dt);

            GLDisablei(GL_BLEND, 0); //this is needed for smaa at least
            GLDisablei(GL_BLEND, 1);
            GLDisablei(GL_BLEND, 2);
        }
        void _passCopyDepth(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth]->bind();

            sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 0);

            _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
            Renderer::colorMask(true, true, true, true);
        }
        void _passLighting(GBuffer& gbuffer, Viewport& viewport, Camera& camera, const uint& fboWidth, const uint& fboHeight,bool mainRenderFunc){
            Scene& scene = viewport.m_Scene;

            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting]->bind();

            if(RenderManager::GLSL_VERSION < 140){
                sendUniformMatrix4Safe("CameraView", camera.getView());
                sendUniformMatrix4Safe("CameraProj", camera.getProjection());
                //sendUniformMatrix4Safe("CameraViewProj",camera.getViewProjection()); //moved to shader binding function
                sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
                sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
                sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
                sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
                sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
            }
            const auto& fbo_width = static_cast<float>(fboWidth);
            const auto& fbo_height = static_cast<float>(fboHeight);

            sendUniform4v("materials[0]", Material::m_MaterialProperities, Material::m_MaterialProperities.size());
            sendUniform4("ScreenData", 0.0f, gamma, fbo_width, fbo_height);

            sendTexture("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 0);
            sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 1);
            sendTexture("gMiscMap", gbuffer.getTexture(GBufferType::Misc), 2);
            sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 3);
            sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 4);

            Renderer::setDepthFunc(GL_GEQUAL);
            Renderer::GLEnable(GL_DEPTH_TEST);
            for (const auto& light : InternalScenePublicInterface::GetPointLights(scene)) {
                _renderPointLight(camera , *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetSpotLights(scene)) {
                _renderSpotLight(camera , *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetRodLights(scene)) {
                _renderRodLight(camera , *light);
            }
            Renderer::setDepthFunc(GL_LEQUAL);
            Renderer::GLDisable(GL_DEPTH_TEST);
            for (const auto& light : InternalScenePublicInterface::GetSunLights(scene)) {
                _renderSunLight(camera, *light);
            }
            for (const auto& light : InternalScenePublicInterface::GetDirectionalLights(scene)) {
                _renderDirectionalLight(camera, *light);
            }

            if(mainRenderFunc){
                //do GI here. (only doing GI during the main render pass, not during light probes
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI]->bind();
                if(RenderManager::GLSL_VERSION < 140){
                    sendUniformMatrix4Safe("CameraInvView", camera.getViewInverse());
                    sendUniformMatrix4Safe("CameraInvProj", camera.getProjectionInverse());
                    sendUniformMatrix4Safe("CameraInvViewProj", camera.getViewProjectionInverse());
                    sendUniform4Safe("CameraInfo1", glm::vec4(camera.getPosition(), camera.getNear()));
                    sendUniform4Safe("CameraInfo2", glm::vec4(camera.getViewVector(), camera.getFar()));
                }
                
                sendUniform4v("materials[0]", Material::m_MaterialProperities, Material::m_MaterialProperities.size());
                sendUniform4("ScreenData", lighting_gi_pack, gamma, fbo_width, fbo_height);
                sendTexture("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 0);
                sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 1);
                sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);
                sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 3);

                Skybox* skybox = scene.skybox();
                if(skybox && skybox->texture()->numAddresses() >= 3){
                    sendTextureSafe("irradianceMap", skybox->texture()->address(1), 4, GL_TEXTURE_CUBE_MAP);
                    sendTextureSafe("prefilterMap", skybox->texture()->address(2), 5, GL_TEXTURE_CUBE_MAP);
                    sendTextureSafe("brdfLUT", *Texture::BRDF, 6);
                }else{
                    sendTextureSafe("irradianceMap", Texture::Black->address(0), 4, GL_TEXTURE_2D);
                    sendTextureSafe("prefilterMap", Texture::Black->address(0), 5, GL_TEXTURE_2D);
                    sendTextureSafe("brdfLUT", *Texture::BRDF, 6);
                }
                _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
            }
        }
        void _passStencil(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass]->bind();

            gbuffer.getMainFBO()->bind();

            GLEnable(GL_STENCIL_TEST);
            Settings::clear(false,false,true); //stencil is completely filled with 0's
            Renderer::stencilMask(0xFFFFFFFF);

            Renderer::stencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
            //exclude shadeless normals
            Renderer::stencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);


            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),0);
            sendUniform1("Type",0.0f);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);

            Renderer::stencilMask(0xFFFFFFFF);

            Renderer::stencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);

            Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil

            Renderer::colorMask(true, true, true, true);
        }
        void _passBlur(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight,string type, GLuint texture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur]->bind();

            const float& divisor = gbuffer.getSmallFBO()->divisor();
            glm::vec2 hv(0.0f);
            if(type == "H"){ hv = glm::vec2(1.0f,0.0f); }
            else{            hv = glm::vec2(0.0f,1.0f); }

            const glm::ivec2 Res(fboWidth, fboHeight);
            auto& bloom = Bloom::bloom;
            sendUniform4("strengthModifier", 
                bloom.blur_strength,
                bloom.blur_strength,
                bloom.blur_strength,
                SSAO::ssao.m_ssao_blur_strength
            );
            sendUniform2("Resolution", Res);
            sendUniform4("DataA", bloom.blur_radius,0.0f,hv.x,hv.y);
            sendTexture("image",gbuffer.getTexture(texture),0);

            const uint screen_width = static_cast<uint>(static_cast<float>(fboWidth) * divisor);
            const uint screen_height = static_cast<uint>(static_cast<float>(fboHeight) * divisor);
            _renderFullscreenTriangle(screen_width, screen_height, 0, 0);
        }
        void _passFinal(GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight, GBufferType::Type sceneTexture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal]->bind();
            sendUniform1Safe("HasBloom", static_cast<int>(Bloom::bloom.bloom_active));
            sendUniform1Safe("HasFog", static_cast<int>(Fog::fog.fog_active));

            if(Fog::fog.fog_active){
                sendUniform1Safe("FogDistNull", Fog::fog.distNull);
                sendUniform1Safe("FogDistBlend", Fog::fog.distBlend);
                sendUniform4Safe("FogColor", Fog::fog.color);
                sendTextureSafe("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);
            }
            sendTextureSafe("SceneTexture", gbuffer.getTexture(sceneTexture), 0);
            sendTextureSafe("gBloomMap", gbuffer.getTexture(GBufferType::Bloom), 1);
            sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 2);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _passDepthAndTransparency(GBuffer& gbuffer , const uint& fboWidth, const uint& fboHeight, Viewport& viewport, Camera& camera, GBufferType::Type sceneTexture) {
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DepthAndTransparency]->bind();

            sendTextureSafe("SceneTexture", gbuffer.getTexture(sceneTexture), 0);
            sendTextureSafe("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 1);

            GLEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            //sendUniform4Safe("TransparencyMaskColor", viewport.getTransparencyMaskColor());
            //sendUniform1Safe("TransparencyMaskActive", static_cast<int>(viewport.isTransparencyMaskActive()));
            sendUniform1Safe("DepthMaskValue", viewport.getDepthMaskValue());
            sendUniform1Safe("DepthMaskActive", static_cast<int>(viewport.isDepthMaskActive()));

            _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);

            GLDisable(GL_BLEND);
        }
        void _renderFullscreenQuad(const uint& width, const uint& height,uint startX,uint startY){
            const float w2 = static_cast<float>(width) * 0.5f;
            const float h2 = static_cast<float>(height) * 0.5f;
            const glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            sendUniformMatrix4("Model", m_IdentityMat4);
            sendUniformMatrix4("VP", p);
            sendUniform2("screenSizeDivideBy2", w2, h2);
            m_FullscreenQuad->render();
        }
        void _renderFullscreenTriangle(const uint& width, const uint& height,uint startX,uint startY){
            const float w2 = static_cast<float>(width) * 0.5f;
            const float h2 = static_cast<float>(height) * 0.5f;
            const glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            sendUniformMatrix4("Model", m_IdentityMat4);
            sendUniformMatrix4("VP", p);
            sendUniform2("screenSizeDivideBy2", w2, h2);
            m_FullscreenTriangle->render();
        }
        
        void _startupRenderFrame(GBuffer& gbuffer, Viewport& viewport, Camera& camera, const glm::uvec4& dimensions) {
            const auto& winSize = Resources::getWindowSize();
            if (viewport.isAspectRatioSynced()) {
                camera.setAspect(dimensions.z / static_cast<float>(dimensions.w));
            }
            Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
            gbuffer.resize(dimensions.z, dimensions.w);
            Renderer::scissorDisable();
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
                if(RenderManager::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA){  
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
            GLDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);


            Renderer::GLDisablei(GL_BLEND, 0);

            #pragma region SSAO
            //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes
            gbuffer.bindFramebuffers(GBufferType::Bloom, GBufferType::GodRays, "A", false);
            Settings::clear(true, false, false); //bloom and god rays alpha channels cleared to black 
            if (SSAO::ssao.m_ssao) {
                GLEnablei(GL_BLEND, 0);//i dont think this is needed anymore
                gbuffer.bindFramebuffers(GBufferType::Bloom, "A", false);
                auto& ssaoShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO];
                SSAO::ssao.passSSAO(ssaoShader, gbuffer, dimensions.z, dimensions.w, camera);
                if (SSAO::ssao.m_ssao_do_blur) {
                    GLDisablei(GL_BLEND, 0); //yes this is absolutely needed
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

            GLDisablei(GL_BLEND, 0);

            _passStencil(gbuffer, dimensions.z, dimensions.w); //confirmed, stencil rejection does help
            
            GLEnablei(GL_BLEND, 0);
            glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            
            //this needs to be cleaned up
            gbuffer.bindFramebuffers(GBufferType::Lighting, "RGB");
            Settings::clear(true, false, false);//lighting rgb channels cleared to black
            if(lighting){
                _passLighting(gbuffer,viewport, camera, dimensions.z, dimensions.w, mainRenderFunc);
            }
            
            GLDisablei(GL_BLEND, 0);
            GLDisable(GL_STENCIL_TEST);


            GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            _passForwardRendering(dt, gbuffer, viewport, camera);
            GLDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            


            #pragma region GodRays
            gbuffer.bindFramebuffers(GBufferType::GodRays, "RGB", false);
            Settings::clear(true, false, false); //godrays rgb channels cleared to black
            auto& godRaysPlatform = GodRays::godRays;

            if (godRaysPlatform.godRays_active && godRaysPlatform.sun) {
                auto& body = *godRaysPlatform.sun->getComponent<ComponentBody>();
                const glm::vec3& oPos = body.position();
                const glm::vec3& camPos = camera.getPosition();
                const glm::vec3& camVec = camera.getViewVector();
                const bool infront = Math::isPointWithinCone(camPos, -camVec, oPos, Math::toRadians(godRaysPlatform.fovDegrees));
                if (infront) {
                    const glm::vec3& sp = Math::getScreenCoordinates(oPos, false);
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
            if (Bloom::bloom.bloom_active) {
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
            if (DepthOfField::DOF.dof) {
                auto& dofShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF];
                gbuffer.bindFramebuffers(outTexture);
                DepthOfField::DOF.pass(dofShader,gbuffer, dimensions.z, dimensions.w, sceneTexture);
                sceneTexture = GBufferType::Lighting;
                outTexture = GBufferType::Misc;
            }
            #pragma endregion
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            #pragma region Finalization and AA
            if (!mainRenderFunc || aa_algorithm == AntiAliasingAlgorithm::None){
                gbuffer.bindFramebuffers(outTexture);
                _passFinal(gbuffer, dimensions.z, dimensions.w, sceneTexture);
                gbuffer.bindBackbuffer(viewport, fbo, rbo);
                _passDepthAndTransparency(gbuffer, dimensions.z, dimensions.w, viewport,camera,outTexture);
            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::FXAA){
                gbuffer.bindFramebuffers(outTexture);
                _passFinal(gbuffer, dimensions.z, dimensions.w, sceneTexture);
                gbuffer.bindFramebuffers(sceneTexture);
                auto& fxaaShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA];
                FXAA::fxaa.pass(fxaaShader, gbuffer, dimensions.z, dimensions.w, outTexture);

                gbuffer.bindBackbuffer(viewport, fbo, rbo);
                _passDepthAndTransparency(gbuffer, dimensions.z, dimensions.w, viewport, camera, sceneTexture);

            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::SMAA){
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
            GLEnablei(GL_BLEND, 0);
            if(mainRenderFunc){
                if(draw_physics_debug  &&  &camera == scene.getActiveCamera()){
                    GLDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_FALSE);
                    m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics]->bind();
                    Core::m_Engine->m_PhysicsManager._render(camera);
                }
            }
            #pragma endregion
            
         
            #pragma region 2DAPI
            GLEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            if(mainRenderFunc){
                if(viewport.isUsing2DAPI()){
                    Settings::clear(false,true,false); //clear depth only
                    m_InternalShaderPrograms[EngineInternalShaderPrograms::Deferred2DAPI]->bind();
                    sendUniformMatrix4("VP", m_2DProjectionMatrix);
                    GLEnable(GL_SCISSOR_TEST);

                    for (auto& command : m_2DAPICommands) {
                        command();
                    }
                    GLDisable(GL_SCISSOR_TEST);
                }
            }
            #pragma endregion

            
        }
};


epriv::RenderManager::RenderManager(const char* name, uint windowWidth, uint windowHeight):m_i(new impl){ 
    m_i->_init(name, windowWidth, windowHeight);
    renderManagerImpl = m_i.get();
    renderManager = this;

    OpenGLStateMachine = OpenGLState(windowWidth, windowHeight);
}
epriv::RenderManager::~RenderManager(){ 
    m_i->_destruct(); 
}
void epriv::RenderManager::_init(const char* name,uint w,uint h){ 
    m_i->_postInit(name, w, h);
}
void epriv::RenderManager::_render(const double& dt, Viewport& viewport,const bool mainFunc, const GLuint display_fbo, const GLuint display_rbo){
    m_i->_render(dt, *m_i->m_GBuffer, viewport, mainFunc, display_fbo, display_rbo);
}
void epriv::RenderManager::_resize(uint w,uint h){ 
    m_i->_resize(w, h);
}
void epriv::RenderManager::_onFullscreen(sf::Window* w,const sf::VideoMode& m,const char* n, const uint& s, const sf::ContextSettings& set){
    m_i->_onFullscreen(w, m, n, s, set);
}
void epriv::RenderManager::_onOpenGLContextCreation(uint windowWidth,uint windowHeight,uint _glslVersion,uint _openglVersion){ 
    OpenGLStateMachine.GL_INIT_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
    m_i->_onOpenGLContextCreation(windowWidth, windowHeight, _glslVersion, _openglVersion);
}
void epriv::RenderManager::_clear2DAPICommands() {
    vector_clear(m_i->m_2DAPICommands);
}
const float epriv::RenderManager::_getGIPackedData() {
    return m_i->lighting_gi_pack;
}
const bool epriv::RenderManager::_bindShaderProgram(ShaderProgram* program){
    auto& currentShaderPgrm = RendererState.current_bound_shader_program;
    if(currentShaderPgrm != program){
        OpenGLStateMachine.GL_glUseProgram(program->program());
        currentShaderPgrm = program;
        currentShaderPgrm->BindableResource::bind();
        return true;
    }
    return false;
}
const bool epriv::RenderManager::_unbindShaderProgram() {
    auto& currentShaderPgrm = RendererState.current_bound_shader_program;
    if (currentShaderPgrm) {
        currentShaderPgrm->BindableResource::unbind();
        currentShaderPgrm = nullptr;
        OpenGLStateMachine.GL_glUseProgram(0);
        return true;
    }
    return false;
}
const bool epriv::RenderManager::_bindMaterial(Material* material){
    auto& currentMaterial = RendererState.current_bound_material;
    if(currentMaterial != material){
        currentMaterial = material;
        currentMaterial->BindableResource::bind();
        return true;
    }
    return false;
}
const bool epriv::RenderManager::_unbindMaterial(){
    auto& currentMaterial = RendererState.current_bound_material;
    if(currentMaterial){
        currentMaterial->BindableResource::unbind();
        currentMaterial = nullptr;
        return true;
    }
    return false;
}
void epriv::RenderManager::_genPBREnvMapData(Texture& texture, uint size1, uint size2){
    m_i->_generatePBREnvMapData(texture,size1,size2);
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
const bool Renderer::setViewport(const uint& x, const uint& y, const uint& w, const uint& h){
    auto& i = *renderManager;
    return i.OpenGLStateMachine.GL_glViewport(x, y, w, h);
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
void Renderer::bindFBO(epriv::FramebufferObject& fbo){ 
    Renderer::bindFBO(fbo.address()); 
}
const bool Renderer::bindRBO(epriv::RenderbufferObject& rbo){
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





//a collection of 2d rendering api functors
void Renderer::alignmentOffset(const Alignment::Type& align, uint& x, uint& y, const uint& width, const uint& height) {
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


struct RenderingAPI2D final {
    static void Render2DText(const string& text, const Font& font, const glm::uvec2& position, const glm::vec4& color, const float& angle, const glm::vec2& scale, const float& depth, const TextAlignment::Type& alignType) { 
        auto& impl = *renderManagerImpl;
        impl.text_pts.clear();
        impl.text_uvs.clear();
        impl.text_ind.clear();

        auto& mesh = *Mesh::FontPlane;
        mesh.bind();
        sendUniform1("DiffuseTextureEnabled", 1);

        const auto& newLineGlyphHeight = font.getGlyphData('X').height;
        const auto& texture = font.getGlyphTexture();
        sendTexture("DiffuseTexture", texture, 0);
        sendUniform4("Object_Color", color);
        float y = 0.0f;
        float x = 0.0f;
        float z = -0.001f - depth;

        glm::mat4 m = impl.m_IdentityMat4;
        m = glm::translate(m, glm::vec3(position.x, position.y, 0));
        m = glm::rotate(m, angle, impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(scale.x, scale.y, 1));
        sendUniformMatrix4("Model", m);

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
    static void Render2DTexture(const Texture* texture, const glm::uvec2& position, const glm::vec4& color, const float& angle, const glm::vec2& scale, const float& depth, const Alignment::Type& align) {
        auto& impl = *renderManagerImpl;
        auto& mesh = *Mesh::Plane;
        mesh.bind();
        glm::mat4 m = impl.m_IdentityMat4;
        sendUniform4("Object_Color", color);

        uint translationX = position.x;
        uint translationY = position.y;
        float totalSizeX   = scale.x;
        float totalSizeY   = scale.y;
        if (texture) {
            totalSizeX *= texture->width();
            totalSizeY *= texture->height();

            sendTexture("DiffuseTexture", *texture, 0);
            sendUniform1("DiffuseTextureEnabled", 1);
        }else{
            sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
            sendUniform1("DiffuseTextureEnabled", 0);
        }
        Renderer::alignmentOffset(align, translationX, translationY, static_cast<uint>(totalSizeX), static_cast<uint>(totalSizeY));

        m = glm::translate(m, glm::vec3(translationX, translationY, -0.001f - depth));
        m = glm::rotate(m, Math::toRadians(angle), impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(totalSizeX, totalSizeY, 1.0f));
        sendUniformMatrix4("Model", m);
        mesh.render(false);
    }
    static void RenderTriangle(const glm::uvec2& position, const glm::vec4& color, const float& angle, const uint& width, const uint& height, const float& depth, const Alignment::Type& align) {
        auto& impl = *renderManagerImpl;

        auto& mesh = *Mesh::Triangle;
        mesh.bind();

        sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
        sendUniform1("DiffuseTextureEnabled", 0);
        sendUniform4("Object_Color", color);

        uint translationX = position.x;
        uint translationY = position.y;

        Renderer::alignmentOffset(align, translationX, translationY, width, height);

        glm::mat4 m = impl.m_IdentityMat4;
        m = glm::translate(m, glm::vec3(translationX, translationY, -0.001f - depth));
        m = glm::rotate(m, Math::toRadians(angle), impl.m_RotationAxis2D);
        m = glm::scale(m, glm::vec3(width, height, 1));
        sendUniformMatrix4("Model", m);

        mesh.render(false);
    }
    static void GLScissor(const int& x, const int& y, const GLsizei& width, const GLsizei& height) {
        glScissor(x, y, width, height);
    }
    static void GLScissorDisable() {
        const auto& winSize = Resources::getWindowSize();
        glScissor(0, 0, winSize.x, winSize.y);
    }
};
void Renderer::renderTriangle(const glm::uvec2& position, const glm::vec4& color, const float& angle, const uint& width, const uint& height, const float& depth, const Alignment::Type& align) {
    boost_func f = boost::bind<void>(&RenderingAPI2D::RenderTriangle, position, color, angle, width, height, depth, align);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
}
void Renderer::renderRectangle(const glm::uvec2& pos, const glm::vec4& col, const uint& width, const uint& height, const float& angle, const float& depth, const Alignment::Type& align){
    boost_func f = boost::bind<void>(&RenderingAPI2D::Render2DTexture, nullptr, pos, col, angle, glm::vec2(width, height), depth, align);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
}
void Renderer::renderTexture(const Texture& tex, const glm::uvec2& p, const glm::vec4& c, const float& a, const glm::vec2& s, const float& d, const Alignment::Type& align){
    boost_func f = boost::bind<void>(&RenderingAPI2D::Render2DTexture, &tex, p, c, a, s, d, align);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
}
void Renderer::renderText(const string& t, const Font& fnt, const glm::uvec2& p, const glm::vec4& c, const float& a, const glm::vec2& s, const float& d, const TextAlignment::Type& align) {
    boost_func f = boost::bind<void>(&RenderingAPI2D::Render2DText, t, boost::ref(fnt), p, c, a, s, d, align);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
}
void Renderer::renderBorder(const uint& borderSize, const glm::uvec2& pos, const glm::vec4& col, const uint& w, const uint& h, const float& angle, const float& depth, const Alignment::Type& align) {
    const uint& doubleBorder = borderSize * 2;
    const uint& halfWidth = w / 2;
    const uint& halfHeight = h / 2;

    uint translationX = pos.x;
    uint translationY = pos.y;
    Renderer::alignmentOffset(align, translationX, translationY, w, h);
    glm::uvec2 newPos(translationX, translationY);

    Renderer::renderRectangle(newPos - glm::uvec2(halfWidth, 0), col, borderSize, h + doubleBorder, angle, depth,Alignment::Right);
    Renderer::renderRectangle(newPos + glm::uvec2(halfWidth, 0), col, borderSize, h + doubleBorder, angle, depth,Alignment::Left);
    Renderer::renderRectangle(newPos - glm::uvec2(0, halfHeight), col, w, borderSize, angle, depth,Alignment::TopCenter);
    Renderer::renderRectangle(newPos + glm::uvec2(0, halfHeight + borderSize), col, w, borderSize, angle, depth,Alignment::BottomCenter);
}
void Renderer::scissor(const int& x, const int& y, const uint& width, const uint& height) {
    boost_func f = boost::bind<void>(&RenderingAPI2D::GLScissor, x, y, width, height);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
}
void Renderer::scissorDisable() {
    boost_func f = boost::bind<void>(&RenderingAPI2D::GLScissorDisable);
    renderManagerImpl->m_2DAPICommands.push_back(std::move(f));
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
