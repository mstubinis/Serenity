#include <core/engine/Engine.h>
#include <core/engine/Engine_Debugging.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/Engine_FullscreenItems.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/Engine_Math.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/Camera.h>
#include <core/engine/lights/Light.h>
#include <core/Font.h>
#include <core/Scene.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/mesh/Mesh.h>
#include <core/MeshInstance.h>
#include <core/Skybox.h>
#include <core/Material.h>
#include <ecs/ComponentBody.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <SFML/Graphics.hpp>
#include <random>

#include <iostream>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

Mesh* epriv::InternalMeshes::PointLightBounds = nullptr;
Mesh* epriv::InternalMeshes::RodLightBounds = nullptr;
Mesh* epriv::InternalMeshes::SpotLightBounds = nullptr;

ShaderP* epriv::InternalShaderPrograms::Deferred = nullptr;
ShaderP* epriv::InternalShaderPrograms::Forward = nullptr;

epriv::RenderManager* renderManager;
epriv::RenderManager::impl* renderManagerImpl;

uint epriv::RenderManager::GLSL_VERSION;
uint epriv::RenderManager::OPENGL_VERSION;
//extensions
vector<bool> epriv::RenderManager::OPENGL_EXTENSIONS = [](){
    vector<bool> v; v.resize(epriv::OpenGLExtensionEnum::_TOTAL,false);

    return v;
}();

namespace Engine{
    namespace epriv{

        struct srtKeyShaderP final{inline bool operator() ( ShaderP* _1,  ShaderP* _2){return (_1->name() < _2->name());}};

        struct EngineInternalShaders final{enum Shader{
            FullscreenVertex,
            FXAAFrag,
            BulletPhysicsVertex,
            BulletPhysicsFrag,
            VertexBasic,
            VertexHUD,
            VertexSkybox,
            LightingVertex,
            ForwardFrag,
            DeferredFrag,
            DeferredFragHUD,
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
            DeferredHUD,
            DeferredGodRays,
            DeferredBlur,
            DeferredBlurSSAO,
            DeferredHDR,
            DeferredSSAO,
            DeferredDOF,
            DeferredBloom,
            DeferredFinal,
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

        struct TextureRenderInfo final{
            Texture* texture;
            glm::vec2 pos, scl;
            glm::vec4 col;
            float rot, depth;
            TextureRenderInfo(Texture* _texture, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth){
                texture = _texture; pos = _pos; col = _col; scl = _scl; rot = _rot; depth = _depth;
            }
        };
        struct FontRenderInfo final{
            Font* font;
            glm::vec2 pos, scl;
            glm::vec4 col;
            float rot, depth;
            std::string text;
            FontRenderInfo(Font* _font, string _text, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth){
                font = _font; pos = _pos; col = _col; scl = _scl; rot = _rot; depth = _depth;
                text = _text;
            }
        };
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

        bool enabled1;

        float gamma;
        Texture* brdfCook;
        Texture* blackCubemap;
        unsigned char cull_face_status;
        GLboolean color_mask_r;
        GLboolean color_mask_g;
        GLboolean color_mask_b;
        GLboolean color_mask_a;
        glm::vec4 clear_color;
        AntiAliasingAlgorithm::Algorithm aa_algorithm;
        DepthFunc::Func depth_func;
        glm::uvec4 gl_viewport_data;
        bool draw_physics_debug;

        GBuffer* m_GBuffer;
        glm::mat4 m_2DProjectionMatrix;
        vector<FontRenderInfo> m_FontsToBeRendered;
        vector<TextureRenderInfo> m_TexturesToBeRendered;
        vector<glm::vec3> text_pts;
        vector<glm::vec2> text_uvs;
        vector<ushort>    text_ind;


        glm::mat4 m_IdentityMat4;
        glm::mat3 m_IdentityMat3;
        FullscreenQuad* m_FullscreenQuad;
        FullscreenTriangle* m_FullscreenTriangle;
        #pragma endregion

        #pragma region EngineInternalShadersAndPrograms
        vector<Shader> m_InternalShaders;
        vector<ShaderP*> m_InternalShaderPrograms;

        epriv::UBOCamera m_UBOCameraData;
        #pragma endregion

        void _init(const char* name,uint& w,uint& h){
            #pragma region LightingInfo
            lighting = true;
            lighting_gi_contribution_diffuse = 1.0f;
            lighting_gi_contribution_specular = 1.0f;
            lighting_gi_contribution_global = 1.0f;
            lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(lighting_gi_contribution_diffuse,lighting_gi_contribution_specular,lighting_gi_contribution_global);
            #pragma endregion

            #pragma region GeneralInfo

            enabled1 = true;

            text_pts.reserve(4096 * 4);//4 points per char
            text_uvs.reserve(4096 * 4);//4 uvs per char
            text_ind.reserve(4096 * 6);//6 ind per char


            gamma = 2.2f;
            brdfCook = nullptr;
            blackCubemap = nullptr;
            cull_face_status = 0; /* 0 = back | 1 = front | 2 = front and back */
            color_mask_r = GL_TRUE;
            color_mask_g = GL_TRUE;
            color_mask_b = GL_TRUE;
            color_mask_a = GL_TRUE;
            clear_color = glm::vec4(0.0f,0.0f,0.0f,0.0f);
            aa_algorithm = AntiAliasingAlgorithm::FXAA;
            depth_func = DepthFunc::Less;
            gl_viewport_data = glm::uvec4(0,0,0,0);
            #ifdef _DEBUG
                draw_physics_debug = true;
            #else
                draw_physics_debug = false;
            #endif

            m_GBuffer = nullptr;
            m_2DProjectionMatrix = glm::ortho(0.0f,float(w),0.0f,float(h),0.005f,1000.0f);
            m_IdentityMat4 = glm::mat4(1.0f);
            m_IdentityMat3 = glm::mat3(1.0f);
            #pragma endregion
        }      
        void _postInit(const char* name,uint& width,uint& height){
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,&UniformBufferObject::MAX_UBO_BINDINGS);
            #pragma region OpenGLExtensions

            //prints all the extensions the gpu supports
            /*
            GLint n=0; glGetIntegerv(GL_NUM_EXTENSIONS, &n); 
            for (GLint i=0; i<n; i++) { 
            const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
            printf("Ext %d: %s\n", i, extension); 
            }
            */

            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_Ansiotropic_Filtering]    = _checkOpenGLExtension("GL_EXT_texture_filter_anisotropic");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_Ansiotropic_Filtering]    = _checkOpenGLExtension("GL_ARB_texture_filter_anisotropic");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_draw_instanced]           = _checkOpenGLExtension("GL_EXT_draw_instanced");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_draw_instanced]           = _checkOpenGLExtension("GL_ARB_draw_instanced");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_separate_shader_objects]  = _checkOpenGLExtension("GL_EXT_separate_shader_objects");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_separate_shader_objects]  = _checkOpenGLExtension("GL_ARB_separate_shader_objects");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_explicit_attrib_location] = _checkOpenGLExtension("GL_EXT_explicit_attrib_location");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_explicit_attrib_location] = _checkOpenGLExtension("GL_ARB_explicit_attrib_location");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_geometry_shader_4]        = _checkOpenGLExtension("GL_EXT_geometry_shader4");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_geometry_shader_4]        = _checkOpenGLExtension("GL_ARB_geometry_shader4");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_compute_shader]           = _checkOpenGLExtension("GL_EXT_compute_shader");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_compute_shader]           = _checkOpenGLExtension("GL_ARB_compute_shader");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::EXT_tessellation_shader]      = _checkOpenGLExtension("GL_EXT_tessellation_shader");
            OPENGL_EXTENSIONS[OpenGLExtensionEnum::ARB_tessellation_shader]      = _checkOpenGLExtension("GL_ARB_tessellation_shader");
            #pragma endregion

            //dummy vao
            GLuint dummyVAO;
            Engine::Renderer::genAndBindVAO(dummyVAO);

            epriv::EShaders::init();

            #pragma region EngineInternalShaderUBOs
            UniformBufferObject::UBO_CAMERA = new UniformBufferObject("Camera",sizeof(epriv::UBOCamera));
            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
            #pragma endregion

            #pragma region EngineInternalShadersAndPrograms
            m_InternalShaderPrograms.resize(EngineInternalShaderPrograms::_TOTAL, nullptr);

            m_InternalShaders.emplace_back(EShaders::fullscreen_quad_vertex,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::fxaa_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::bullet_physics_vert, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::bullet_physcis_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::vertex_basic,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::vertex_hud,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::vertex_skybox,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::lighting_vert, ShaderType::Vertex, false);
            m_InternalShaders.emplace_back(EShaders::forward_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_hud,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_skybox,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::deferred_frag_skybox_fake,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::copy_depth_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::ssao_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::bloom_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::hdr_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::blur_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::depth_of_field, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::ssao_blur_frag, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::godRays_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::final_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag_optimized, ShaderType::Fragment, false);
            m_InternalShaders.emplace_back(EShaders::lighting_frag_gi,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::cubemap_convolude_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::cubemap_prefilter_envmap_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::brdf_precompute,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::greyscale_frag,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::stencil_passover,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_1,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_2,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_3,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::smaa_vertex_4,ShaderType::Vertex,false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_1,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_2,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_3,ShaderType::Fragment,false);
            m_InternalShaders.emplace_back(EShaders::smaa_frag_4,ShaderType::Fragment,false);

            InternalShaderPrograms::Deferred = new ShaderP("Deferred",m_InternalShaders[EngineInternalShaders::VertexBasic],m_InternalShaders[EngineInternalShaders::DeferredFrag]);
            InternalShaderPrograms::Forward = new ShaderP("Forward",m_InternalShaders[EngineInternalShaders::VertexBasic],m_InternalShaders[EngineInternalShaders::ForwardFrag]);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics] = new ShaderP("Bullet_Physics",m_InternalShaders[EngineInternalShaders::BulletPhysicsVertex],m_InternalShaders[EngineInternalShaders::BulletPhysicsFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHUD] = new ShaderP("Deferred_HUD",m_InternalShaders[EngineInternalShaders::VertexHUD],m_InternalShaders[EngineInternalShaders::DeferredFragHUD]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredGodRays] = new ShaderP("Deferred_GodsRays",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::GodRaysFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur] = new ShaderP("Deferred_Blur",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::BlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlurSSAO] = new ShaderP("Deferred_Blur_SSAO", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::SSAOBlurFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHDR] = new ShaderP("Deferred_HDR",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::HDRFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO] = new ShaderP("Deferred_SSAO",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::SSAOFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF] = new ShaderP("Deferred_DOF", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::DOFFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBloom] = new ShaderP("Deferred_Bloom", m_InternalShaders[EngineInternalShaders::FullscreenVertex], m_InternalShaders[EngineInternalShaders::BloomFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal] = new ShaderP("Deferred_Final",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::FinalFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA] = new ShaderP("Deferred_FXAA",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::FXAAFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox] = new ShaderP("Deferred_Skybox",m_InternalShaders[EngineInternalShaders::VertexSkybox],m_InternalShaders[EngineInternalShaders::DeferredFragSkybox]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkyboxFake] = new ShaderP("Deferred_Skybox_Fake",m_InternalShaders[EngineInternalShaders::VertexSkybox],m_InternalShaders[EngineInternalShaders::DeferredFragSkyboxFake]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth] = new ShaderP("Copy_Depth",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::CopyDepthFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting] = new ShaderP("Deferred_Light",m_InternalShaders[EngineInternalShaders::LightingVertex],m_InternalShaders[EngineInternalShaders::LightingFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingOptimized] = new ShaderP("Deferred_Light_Optimized", m_InternalShaders[EngineInternalShaders::LightingVertex], m_InternalShaders[EngineInternalShaders::LightingFragOptimized]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI] = new ShaderP("Deferred_Light_GI",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::LightingGIFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude] = new ShaderP("Cubemap_Convolude",m_InternalShaders[EngineInternalShaders::VertexSkybox],m_InternalShaders[EngineInternalShaders::CubemapConvoludeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv] = new ShaderP("Cubemap_Prefilter_Env",m_InternalShaders[EngineInternalShaders::VertexSkybox],m_InternalShaders[EngineInternalShaders::CubemapPrefilterEnvFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance] = new ShaderP("BRDF_Precompute_CookTorrance",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::BRDFPrecomputeFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::Grayscale] = new ShaderP("Greyscale_Frag",m_InternalShaders[EngineInternalShaders::FullscreenVertex],m_InternalShaders[EngineInternalShaders::GrayscaleFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass] = new ShaderP("Stencil_Pass",m_InternalShaders[EngineInternalShaders::LightingVertex],m_InternalShaders[EngineInternalShaders::StencilPassFrag]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA1] = new ShaderP("Deferred_SMAA_1",m_InternalShaders[EngineInternalShaders::SMAAVertex1],m_InternalShaders[EngineInternalShaders::SMAAFrag1]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA2] = new ShaderP("Deferred_SMAA_2",m_InternalShaders[EngineInternalShaders::SMAAVertex2],m_InternalShaders[EngineInternalShaders::SMAAFrag2]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA3] = new ShaderP("Deferred_SMAA_3",m_InternalShaders[EngineInternalShaders::SMAAVertex3],m_InternalShaders[EngineInternalShaders::SMAAFrag3]);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA4] = new ShaderP("Deferred_SMAA_4",m_InternalShaders[EngineInternalShaders::SMAAVertex4],m_InternalShaders[EngineInternalShaders::SMAAFrag4]);
            
            #pragma endregion

            #pragma region MeshData
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

            epriv::InternalMeshes::PointLightBounds = new Mesh(pointLightMesh,0.0005f);
            epriv::InternalMeshes::RodLightBounds = new Mesh(rodLightData,0.0005f);
            epriv::InternalMeshes::SpotLightBounds = new Mesh(spotLightData,0.0005f);

            Mesh::FontPlane = new Mesh("FontPlane",1.0f,1.0f,0.0005f);
            Mesh::Plane = new Mesh("Plane",1.0f,1.0f,0.0005f);
            Mesh::Cube = new Mesh(cubeMesh,0.0005f);

            sf::Image sfImageWhite; sfImageWhite.create(2, 2, sf::Color::White);
            sf::Image sfImageBlack; sfImageBlack.create(2, 2, sf::Color::Black);
            Texture::White = new Texture(sfImageWhite, "WhiteTexturePlaceholder", false, ImageInternalFormat::RGB8);
            Texture::Black = new Texture(sfImageBlack, "BlackTexturePlaceholder", false, ImageInternalFormat::RGB8);

            brdfCook = new Texture(512,512,ImagePixelType::FLOAT,ImagePixelFormat::RG,ImageInternalFormat::RG16F);
            brdfCook->setWrapping(TextureWrap::ClampToEdge);	
            epriv::Core::m_Engine->m_ResourceManager._addTexture(brdfCook);

            m_FullscreenQuad = new FullscreenQuad();
            m_FullscreenTriangle = new FullscreenTriangle();

            epriv::Postprocess_SSAO::SSAO.init();

            GLEnable(GLState::DEPTH_TEST);
            Renderer::setDepthFunc(DepthFunc::LEqual);
            glClearDepth(1.0f);
            glClearStencil(0);
            GLDisable(GLState::STENCIL_TEST);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1); //for non Power of Two textures
    
            //GLEnable(GLState::TEXTURE_CUBE_MAP_SEAMLESS); //very odd, supported on my gpu and opengl version but it runs REAL slowly, dropping fps to 1
            GLEnable(GLState::DEPTH_CLAMP);



            epriv::Postprocess_SMAA::SMAA.init();

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

            SAFE_DELETE(Texture::White);
            SAFE_DELETE(Texture::Black);

            SAFE_DELETE(epriv::InternalShaderPrograms::Deferred);
            SAFE_DELETE(epriv::InternalShaderPrograms::Forward);

            SAFE_DELETE_VECTOR(m_InternalShaderPrograms);

            //TODO: add cleanup() from ssao / smaa here?
        }
        bool _checkOpenGLExtension(const char* e){ if(glewIsExtensionSupported(e)!=0) return true;return 0!=glewIsSupported(e); }
        void _renderSkybox(SkyboxEmpty* skybox){
            Scene& scene = *Resources::getCurrentScene();
            Camera* c = scene.getActiveCamera();
            glm::mat4 view = c->getView();
            Math::removeMatrixPosition(view);
            if(skybox){
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox]->bind();
                sendUniformMatrix4("VP",c->getProjection() * view);
                sendTexture("Texture",skybox->texture()->address(0),0,GL_TEXTURE_CUBE_MAP);
                Skybox::bindMesh();
                skybox->draw();
            }else{//render a fake skybox.
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkyboxFake]->bind();
                glm::vec3 bgColor = scene.getBackgroundColor();
                sendUniformMatrix4("VP",c->getProjection() * view);
                sendUniform4("Color",bgColor.r,bgColor.g,bgColor.b,1.0f);
                Skybox::bindMesh();
            }
        }
        void _resize(uint& w, uint& h){
            m_2DProjectionMatrix = glm::ortho(0.0f,(float)w,0.0f,(float)h,0.005f,3000.0f);
            m_GBuffer->resize(w,h);
        }
        void _onFullscreen(sf::Window* sfWindow,sf::VideoMode& videoMode,const char* winName,uint& style,sf::ContextSettings& settings){
            SAFE_DELETE(m_GBuffer);

            sfWindow->create(videoMode,winName,style,settings);

            //oh yea the opengl context is lost, gotta restore the state machine
            Renderer::RestoreGLState();
  
            glDepthFunc(GL_LEQUAL);
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_CLAMP);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClearDepth(1.0f);
            glClearStencil(0);

            const auto& winSize = Resources::getWindowSize();
            m_GBuffer = new GBuffer(winSize.x, winSize.y);
        }
        void _onOpenGLContextCreation(uint& width,uint& height,uint& _glslVersion,uint _openglVersion){
            epriv::RenderManager::GLSL_VERSION = _glslVersion;
            epriv::RenderManager::OPENGL_VERSION = _openglVersion;
            glewExperimental = GL_TRUE;
            glewInit(); glGetError();//stupid glew always inits an error. nothing we can do about it.
            GLEnable(GLState::CULL_FACE);
            Settings::cullFace(GL_BACK);
            SAFE_DELETE(m_GBuffer);
            m_GBuffer = new GBuffer(width,height);
        }
        void _generatePBREnvMapData(Texture& texture,uint& convoludeTextureSize, uint& preEnvFilterSize){
            uint texType = texture.type();
            if(texType != GL_TEXTURE_CUBE_MAP){
                cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
            }
            uint size = convoludeTextureSize;
            bindTexture(texType, texture.address(1));
            Renderer::unbindFBO();
            epriv::FramebufferObject* fbo = new epriv::FramebufferObject(texture.name() + "_fbo_envData",size,size); //try without a depth format
            fbo->bind();
    
            //make these 2 variables global in the renderer class?
            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f),1.0f,0.1f,3000000.0f);
            glm::mat4 captureViews[] = {
                glm::lookAt(glm::vec3(0.0f),glm::vec3( 1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
                glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
            };
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude]->bind();

            sendTexture("cubemap",texture.address(),0,texType);
            setViewport(0,0,size,size);
            for (uint i = 0; i < 6; ++i){
                glm::mat4 vp = captureProjection * captureViews[i];
                sendUniformMatrix4("VP",vp);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(1),0);
                Settings::clear(true,true,false);
                Skybox::bindMesh();
            }
            Resources::getWindow().display(); //prevent opengl & windows timeout


            //now gen EnvPrefilterMap for specular IBL
            size = preEnvFilterSize;
            bindTexture(texType, texture.address(2));
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv]->bind();
            sendTexture("cubemap",texture.address(),0,texType);
            sendUniform1("PiFourDividedByResSquaredTimesSix",12.56637f / float((texture.width() * texture.width())*6));
            sendUniform1("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
                fbo->resize(mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                sendUniform1("roughness",roughness);
                float a = roughness * roughness;
                sendUniform1("a2",a*a);
                for (uint i = 0; i < 6; ++i){
                    glm::mat4 vp = captureProjection * captureViews[i];
                    sendUniformMatrix4("VP", vp);
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture.address(2),m);
                    Settings::clear(true,true,false);
                    Skybox::bindMesh();
                }
            }
            Resources::getWindow().display(); //prevent opengl & windows timeout
            fbo->unbind();
            SAFE_DELETE(fbo);
        }
        void _generateBRDFLUTCookTorrance(uint brdfSize){
            uint& prevReadBuffer = renderManager->glSM.current_bound_read_fbo;
            uint& prevDrawBuffer = renderManager->glSM.current_bound_draw_fbo;

            FramebufferObject* fbo = new FramebufferObject("BRDFLUT_Gen_CookTorr_FBO",brdfSize,brdfSize); //try without a depth format
            fbo->bind();

            bindTexture(GL_TEXTURE_2D, brdfCook->address());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
            Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToEdge);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,brdfCook->address(), 0);

            m_InternalShaderPrograms[EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance]->bind();
            sendUniform1("NUM_SAMPLES",256);
            Settings::clear(true,true,false);
            Renderer::colorMask(true,true,false,false);
            _renderFullscreenTriangle(brdfSize,brdfSize,0,0);
            Renderer::colorMask(true, true, true, true);

            SAFE_DELETE(fbo);
            bindReadFBO(prevReadBuffer);
            bindDrawFBO(prevDrawBuffer);
        }
        void _setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm& algorithm){
            if(aa_algorithm != algorithm){ 
                aa_algorithm = algorithm; 
            }
        }
        void _setDepthFunc(DepthFunc::Func func){
            if(depth_func != func){
                glDepthFunc(func);
                depth_func = func;
            }
        }
        void _clearColor(float& r, float& g, float& b, float& a) {
            if (r == clear_color.r && g == clear_color.g && b == clear_color.b && a == clear_color.a)
                return;
            clear_color.r = r;
            clear_color.g = g;
            clear_color.b = b;
            clear_color.a = a;
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        }
        void _colorMask(bool& r, bool& g, bool& b, bool& a) {
            GLboolean _r = (GLboolean)r;
            GLboolean _g = (GLboolean)g;
            GLboolean _b = (GLboolean)b;
            GLboolean _a = (GLboolean)a;
            if (_r == color_mask_r && _g == color_mask_g && _b == color_mask_b && _a == color_mask_a)
                return;
            color_mask_r = _r;
            color_mask_g = _g;
            color_mask_b = _b;
            color_mask_a = _a;
            glColorMask(color_mask_r, color_mask_g, color_mask_b, color_mask_a);
        }
        void _cullFace(uint s){
            //0 = back | 1 = front | 2 = front and back
            if(s == GL_BACK && cull_face_status != 0){
                glCullFace(GL_BACK);
                cull_face_status = 0;
            }else if(s == GL_FRONT && cull_face_status != 1){
                glCullFace(GL_FRONT);
                cull_face_status = 1;
            }else if(s == GL_FRONT_AND_BACK && cull_face_status != 2){
                glCullFace(GL_FRONT_AND_BACK);
                cull_face_status = 2;
            }
        }
        void _setViewport(uint& x, uint& y, uint& w, uint& h){
            if(gl_viewport_data.x == x && gl_viewport_data.y == y && gl_viewport_data.z == w && gl_viewport_data.w == h) return;
            glViewport(GLint(x), GLint(y), GLsizei(w), GLsizei(h));
            gl_viewport_data = glm::uvec4(x,y,w,h);
        }
        void _bindReadFBO(uint& f){
            if(renderManager->glSM.current_bound_read_fbo != f){
                glBindFramebuffer(GL_READ_FRAMEBUFFER, f);
                renderManager->glSM.current_bound_read_fbo = f;
            }
        }
        void _bindDrawFBO(uint& f){
            if(renderManager->glSM.current_bound_draw_fbo != f){
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f);
                renderManager->glSM.current_bound_draw_fbo = f;
            }
        }
        void _bindRBO(uint& r){
            if(renderManager->glSM.current_bound_rbo != r){
                glBindRenderbuffer(GL_RENDERBUFFER, r);
                renderManager->glSM.current_bound_rbo = r;
            }
        }
        void _renderTextures(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight){
            //TODO: optimize by batching the quads into 1 draw call, like in _renderText()
            if (m_TexturesToBeRendered.size() > 0) {
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHUD]->bind();
                auto& mesh = *Mesh::Plane;
                mesh.bind();
                glm::mat4 m = m_IdentityMat4;
                sendUniformMatrix4("VP", m_2DProjectionMatrix);
                for (auto& item : m_TexturesToBeRendered) {
                    sendUniform4("Object_Color", item.col);
                    m = m_IdentityMat4;
                    m = glm::translate(m, glm::vec3(item.pos.x, item.pos.y, -0.001f - item.depth));
                    m = glm::rotate(m, item.rot, glm::vec3(0.0f, 0.0f, 1.0f));
                    if (item.texture) {
                        m = glm::scale(m, glm::vec3(item.texture->width(), item.texture->height(), 1.0f));
                        sendTexture("DiffuseTexture", *item.texture, 0);
                        sendUniform1("DiffuseTextureEnabled", 1);
                    }else{
                        sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
                        sendUniform1("DiffuseTextureEnabled", 0);
                    }
                    m = glm::scale(m, glm::vec3(item.scl.x, item.scl.y, 1.0f));  
                    sendUniformMatrix4("Model", m);
                    mesh.render(false);
                }
            }
        }
        void _renderText(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight){
            if (m_FontsToBeRendered.size() > 0) {
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHUD]->bind();
                glm::mat4 m = m_IdentityMat4;
                float x, y, z;
                x = y = z = 0.0f;
                Mesh& mesh = *(Mesh::FontPlane);
                mesh.bind();
                sendUniformMatrix4("VP", m_2DProjectionMatrix);
                sendUniform1("DiffuseTextureEnabled", 1);
                for (auto& item : m_FontsToBeRendered) {
                    text_pts.clear();
                    text_uvs.clear();
                    text_ind.clear();


                    Font& font = *item.font;
                    auto& newLineGlyph = font.getGlyphData('X');
                    auto& texture = font.getGlyphTexture();
                    sendTexture("DiffuseTexture", texture, 0);
                    sendUniform4("Object_Color", item.col);
                    y = 0.0f;
                    x = 0.0f;
                    z = -0.001f - item.depth;
                    uint i = 0;

                    m = m_IdentityMat4;
                    m = glm::translate(m, glm::vec3(item.pos.x, item.pos.y, 0));
                    m = glm::rotate(m, item.rot, glm::vec3(0, 0, 1));
                    m = glm::scale(m, glm::vec3(item.scl.x, item.scl.y, 1));
                    sendUniformMatrix4("Model", m);

                    for (auto& character : item.text) {
                        if (character == '\n') {
                            y += newLineGlyph.height + 7;
                            x = 0.0f;
                        }else if(character != '\0'){
                            uint accum = i * 4;
                            FontGlyph& chr = font.getGlyphData(character);
                            float startingX = x + chr.xoffset;
                            float startingY = -int(chr.height + chr.yoffset) - y;

                            text_ind.emplace_back(accum + 0);
                            text_ind.emplace_back(accum + 1);
                            text_ind.emplace_back(accum + 2);
                            text_ind.emplace_back(accum + 3);
                            text_ind.emplace_back(accum + 1);
                            text_ind.emplace_back(accum + 0);

                            text_pts.emplace_back(startingX + chr.pts[0].x, startingY + chr.pts[0].y, z);
                            text_pts.emplace_back(startingX + chr.pts[1].x, startingY + chr.pts[1].y, z);
                            text_pts.emplace_back(startingX + chr.pts[2].x, startingY + chr.pts[2].y, z);
                            text_pts.emplace_back(startingX + chr.pts[3].x, startingY + chr.pts[3].y, z);

                            text_uvs.emplace_back(chr.uvs[0].x, chr.uvs[0].y);
                            text_uvs.emplace_back(chr.uvs[1].x, chr.uvs[1].y);
                            text_uvs.emplace_back(chr.uvs[2].x, chr.uvs[2].y);
                            text_uvs.emplace_back(chr.uvs[3].x, chr.uvs[3].y);
                            x += chr.xadvance;
                            ++i;
                        }
                    }
                    mesh.modifyVertices(0, text_pts);
                    mesh.modifyVertices(1, text_uvs);
                    mesh.modifyIndices(text_ind);
                    mesh.render(false);
                }
            }
        }
        void _renderSunLight(SunLight& s) {
            if (!s.isActive()) return;
            auto& body = *s.m_Entity.getComponent<ComponentBody>();
            glm::vec3 pos = body.position();
            Renderer::sendUniform4("LightDataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, 0.0f);
            Renderer::sendUniform4("LightDataC", 0.0f, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("LightDataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, float(s.m_Type));
            Renderer::sendUniform1Safe("Type", 0.0f);

            Renderer::renderFullscreenTriangle();
        }
        void _renderPointLight(PointLight& p) {
            if (!p.isActive()) return;
            Camera& c = *Resources::getCurrentScene()->getActiveCamera();
            auto& body = *p.m_Entity.getComponent<ComponentBody>();
            glm::vec3 pos = body.position();
            if ((!c.sphereIntersectTest(pos, p.m_CullingRadius)) || (c.getDistance(pos) > 1100.0f * p.m_CullingRadius)) //1100.0f is the visibility threshold
                return;
            Renderer::sendUniform4("LightDataA", p.m_AmbientIntensity, p.m_DiffuseIntensity, p.m_SpecularIntensity, 0.0f);
            Renderer::sendUniform4("LightDataB", 0.0f, 0.0f, p.m_C, p.m_L);
            Renderer::sendUniform4("LightDataC", p.m_E, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("LightDataD", p.m_Color.x, p.m_Color.y, p.m_Color.z, float(p.m_Type));
            Renderer::sendUniform4Safe("LightDataE", 0.0f, 0.0f, float(p.m_AttenuationModel), 0.0f);
            Renderer::sendUniform1Safe("Type", 1.0f);

            glm::vec3 camPos = c.getPosition();
            glm::mat4 model = body.modelMatrix();
            glm::mat4 vp = c.getViewProjection();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", vp);

            Renderer::GLEnable(GLState::DEPTH_TEST);
            if (glm::distance(c.getPosition(), pos) <= p.m_CullingRadius) { //inside the light volume
                Renderer::Settings::cullFace(GL_FRONT);
                Renderer::setDepthFunc(DepthFunc::GEqual);
            }
            auto& pointLightMesh = *epriv::InternalMeshes::PointLightBounds;

            pointLightMesh.bind();
            pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            pointLightMesh.unbind();
            Renderer::Settings::cullFace(GL_BACK);
            Renderer::setDepthFunc(DepthFunc::LEqual);
            Renderer::GLDisable(GLState::DEPTH_TEST);
        }
        void _renderDirectionalLight(DirectionalLight& d) {
            if (!d.isActive()) return;
            auto& body = *d.m_Entity.getComponent<ComponentBody>();
            glm::vec3 _forward = body.forward();
            Renderer::sendUniform4("LightDataA", d.m_AmbientIntensity, d.m_DiffuseIntensity, d.m_SpecularIntensity, _forward.x);
            Renderer::sendUniform4("LightDataB", _forward.y, _forward.z, 0.0f, 0.0f);
            Renderer::sendUniform4("LightDataD", d.m_Color.x, d.m_Color.y, d.m_Color.z, float(d.m_Type));
            Renderer::sendUniform1Safe("Type", 0.0f);
            Renderer::renderFullscreenTriangle();
        }
        void _renderSpotLight(SpotLight& s) {
            if (!s.isActive()) return;
            Camera& c = *Resources::getCurrentScene()->getActiveCamera();
            auto& body = *s.m_Entity.getComponent<ComponentBody>();
            glm::vec3 pos = body.position();
            glm::vec3 _forward = body.forward();
            if (!c.sphereIntersectTest(pos, s.m_CullingRadius) || (c.getDistance(pos) > 1100.0f * s.m_CullingRadius))
                return;
            Renderer::sendUniform4("LightDataA", s.m_AmbientIntensity, s.m_DiffuseIntensity, s.m_SpecularIntensity, _forward.x);
            Renderer::sendUniform4("LightDataB", _forward.y, _forward.z, s.m_C, s.m_L);
            Renderer::sendUniform4("LightDataC", s.m_E, pos.x, pos.y, pos.z);
            Renderer::sendUniform4("LightDataD", s.m_Color.x, s.m_Color.y, s.m_Color.z, float(s.m_Type));
            Renderer::sendUniform4Safe("LightDataE", s.m_Cutoff, s.m_OuterCutoff, float(s.m_AttenuationModel), 0.0f);
            Renderer::sendUniform2Safe("VertexShaderData", s.m_OuterCutoff, s.m_CullingRadius);
            Renderer::sendUniform1Safe("Type", 2.0f);

            glm::vec3 camPos = c.getPosition();
            glm::mat4 model = body.modelMatrix();
            glm::mat4 vp = c.getViewProjection();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", vp);

            Renderer::GLEnable(GLState::DEPTH_TEST);
            if (glm::distance(c.getPosition(), pos) <= s.m_CullingRadius) { //inside the light volume                                                 
                Renderer::Settings::cullFace(GL_FRONT);
                Renderer::setDepthFunc(DepthFunc::GEqual);
            }
            auto& spotLightMesh = *epriv::InternalMeshes::SpotLightBounds;

            spotLightMesh.bind();
            spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            spotLightMesh.unbind();
            Renderer::Settings::cullFace(GL_BACK);
            Renderer::setDepthFunc(DepthFunc::LEqual);

            Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
            Renderer::GLDisable(GLState::DEPTH_TEST);
        }
        void _renderRodLight(RodLight& r) {
            if (!r.isActive()) return;
            Camera& c = *Resources::getCurrentScene()->getActiveCamera();
            auto& body = *r.m_Entity.getComponent<ComponentBody>();
            glm::vec3 pos = body.position();
            float cullingDistance = r.m_RodLength + (r.m_CullingRadius * 2.0f);
            if (!c.sphereIntersectTest(pos, cullingDistance) || (c.getDistance(pos) > 1100.0f * cullingDistance))
                return;
            float half = r.m_RodLength / 2.0f;
            glm::vec3 firstEndPt = pos + (body.forward() * half);
            glm::vec3 secndEndPt = pos - (body.forward() * half);
            Renderer::sendUniform4("LightDataA", r.m_AmbientIntensity, r.m_DiffuseIntensity, r.m_SpecularIntensity, firstEndPt.x);
            Renderer::sendUniform4("LightDataB", firstEndPt.y, firstEndPt.z, r.m_C, r.m_L);
            Renderer::sendUniform4("LightDataC", r.m_E, secndEndPt.x, secndEndPt.y, secndEndPt.z);
            Renderer::sendUniform4("LightDataD", r.m_Color.x, r.m_Color.y, r.m_Color.z, float(r.m_Type));
            Renderer::sendUniform4Safe("LightDataE", r.m_RodLength, 0.0f, float(r.m_AttenuationModel), 0.0f);
            Renderer::sendUniform1Safe("Type", 1.0f);

            glm::vec3 camPos = c.getPosition();
            glm::mat4 model = body.modelMatrix();
            glm::mat4 vp = c.getViewProjection();

            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", vp);

            Renderer::GLEnable(GLState::DEPTH_TEST);
            if (glm::distance(c.getPosition(), pos) <= cullingDistance) {
                Renderer::Settings::cullFace(GL_FRONT);
                Renderer::setDepthFunc(DepthFunc::GEqual);
            }
            auto& rodLightMesh = *epriv::InternalMeshes::RodLightBounds;

            rodLightMesh.bind();
            rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
            rodLightMesh.unbind();
            Renderer::Settings::cullFace(GL_BACK);
            Renderer::setDepthFunc(DepthFunc::LEqual);
            Renderer::GLDisable(GLState::DEPTH_TEST);

            Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
        }
        void _passGeometry(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight, Entity* ignore){
            Scene& scene = *Resources::getCurrentScene();
            const glm::vec3& clear = scene.getBackgroundColor();
            const float colors[4] = { clear.r,clear.g,clear.b,1.0f };  
    
            gbuffer.start(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

            Settings::clear(true,true,true); // (0,0,0,0)
            
            Renderer::setDepthFunc(DepthFunc::LEqual);
            //GLDisable(GLState::BLEND_0);

            glClearBufferfv(GL_COLOR,0,colors);
            auto& godRaysPlatform = epriv::Postprocess_GodRays::GodRays;
            if(godRaysPlatform.godRays){
                const float _godraysclearcolor[4] = { 
                    godRaysPlatform.clearColor.r, 
                    godRaysPlatform.clearColor.g, 
                    godRaysPlatform.clearColor.b, 
                    godRaysPlatform.clearColor.a 
                };
                glClearBufferfv(GL_COLOR,2, _godraysclearcolor);
            }

            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);

            //this is needed for sure
            GLEnable(GLState::BLEND_0);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);      

            //RENDER NORMAL OBJECTS HERE
            InternalScenePublicInterface::RenderGeometryOpaque(scene,c);

            //skybox here
            _renderSkybox(scene.skybox());

            InternalScenePublicInterface::RenderGeometryTransparent(scene,c);
        }
        void _passForwardRendering(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight, Entity* ignore){
            Scene& scene = *Resources::getCurrentScene();

            gbuffer.start(GBufferType::Diffuse);
            
            //RENDER NORMAL OBJECTS HERE
            InternalScenePublicInterface::RenderForwardOpaque(scene,c);

            InternalScenePublicInterface::RenderForwardTransparent(scene,c);
        }
        void _passCopyDepth(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::CopyDepth]->bind();

            sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),0);

            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            Renderer::colorMask(true, true, true, true);
        }
        void _passLighting(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight,bool mainRenderFunc){
            Scene& s = *Resources::getCurrentScene(); 
            //if(enabled1)
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLighting]->bind();
            //else
                //m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingOptimized]->bind();
            
            if(RenderManager::GLSL_VERSION < 140){
                sendUniformMatrix4Safe("CameraView",c.getView());
                sendUniformMatrix4Safe("CameraProj",c.getProjection());
                //sendUniformMatrix4Safe("CameraViewProj",c.getViewProjection()); //moved to shader binding function
                sendUniformMatrix4Safe("CameraInvView",c.getViewInverse());
                sendUniformMatrix4Safe("CameraInvProj",c.getProjectionInverse());
                sendUniformMatrix4Safe("CameraInvViewProj",c.getViewProjectionInverse());
                sendUniform4Safe("CameraInfo1",glm::vec4(c.getPosition(),c.getNear()));
                sendUniform4Safe("CameraInfo2",glm::vec4(c.getViewVector(),c.getFar()));
            }
            
            sendUniform4v("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
            sendUniform4("ScreenData",0.0f,gamma,(float)fboWidth,(float)fboHeight);

            sendTexture("gDiffuseMap",gbuffer.getTexture(GBufferType::Diffuse),0);
            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),1);
            sendTexture("gMiscMap",gbuffer.getTexture(GBufferType::Misc),2);
            sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),3);
            sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 4);

            for (const auto& light: InternalScenePublicInterface::GetPointLights(s)){
                _renderPointLight(*light);
            }
            for (const auto& light : InternalScenePublicInterface::GetSpotLights(s)) {
                _renderSpotLight(*light);
            }
            for (const auto& light : InternalScenePublicInterface::GetRodLights(s)) {
                _renderRodLight(*light);
            }
            for (const auto& light : InternalScenePublicInterface::GetSunLights(s)) {
                _renderSunLight(*light);
            }
            for (const auto& light : InternalScenePublicInterface::GetDirectionalLights(s)) {
                _renderDirectionalLight(*light);
            }




            if(mainRenderFunc){
                //do GI here. (only doing GI during the main render pass, not during light probes
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredLightingGI]->bind();
                if(RenderManager::GLSL_VERSION < 140){
                    sendUniformMatrix4Safe("CameraInvView",c.getViewInverse());
                    sendUniformMatrix4Safe("CameraInvProj",c.getProjectionInverse());
                    sendUniformMatrix4Safe("CameraInvViewProj",c.getViewProjectionInverse());
                    sendUniform4Safe("CameraInfo1",glm::vec4(c.getPosition(),c.getNear()));
                    sendUniform4Safe("CameraInfo2",glm::vec4(c.getViewVector(),c.getFar()));
                }
                
                sendUniform4v("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
                sendUniform4("ScreenData",lighting_gi_pack,gamma,(float)fboWidth,(float)fboHeight);
                sendTexture("gDiffuseMap",gbuffer.getTexture(GBufferType::Diffuse),0);
                sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),1);
                sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);
                sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 3);

                SkyboxEmpty* skybox = s.skybox();

                //if(s.lightProbes().size() > 0){
                    /*
                    for(auto& probe:s->lightProbes()){
                        LightProbe* p = probe.second;
                        sendTextureSafe("irradianceMap",p->getIrriadianceMap(),4,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("prefilterMap",p->getPrefilterMap(),5,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("brdfLUT",brdfCook,6);
                        break;
                    }
                    */
                //}else{
                    if(skybox && skybox->texture()->numAddresses() >= 3){
                        sendTextureSafe("irradianceMap",skybox->texture()->address(1),4,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("prefilterMap",skybox->texture()->address(2),5,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("brdfLUT",*brdfCook,6);
                    }else{
                        sendTextureSafe("irradianceMap", Texture::Black->address(0), 4, GL_TEXTURE_2D);
                        sendTextureSafe("prefilterMap", Texture::Black->address(0), 5, GL_TEXTURE_2D);
                        sendTextureSafe("brdfLUT", *brdfCook, 6);
                    }
                //}
                _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            }
            GLDisable(GLState::STENCIL_TEST);
        }
        void _passStencil(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms[EngineInternalShaderPrograms::StencilPass]->bind();

            Scene& s = *Resources::getCurrentScene();
            gbuffer.getMainFBO()->bind();

            GLEnable(GLState::STENCIL_TEST);
            Settings::clear(false,false,true); //stencil is completely filled with 0's
            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
            
            //exclude shadeless normals
            glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),0);
            sendUniform1("Type",0.0f);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);


            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil

            Renderer::colorMask(true, true, true, true);
        }

        void _passBlur(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight,string type, GLuint texture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlur]->bind();

            const float& _divisor = gbuffer.getSmallFBO()->divisor();
            glm::vec2 hv(0.0f);
            if(type == "H"){ hv = glm::vec2(1.0f,0.0f); }
            else{            hv = glm::vec2(0.0f,1.0f); }

            glm::ivec2 Res(fboWidth, fboHeight);

            sendUniform4("strengthModifier", 
                epriv::Postprocess_Bloom::Bloom.blur_strength,
                epriv::Postprocess_Bloom::Bloom.blur_strength,
                epriv::Postprocess_Bloom::Bloom.blur_strength,
                epriv::Postprocess_SSAO::SSAO.m_ssao_blur_strength
            );
            sendUniform2("Resolution", Res);
            sendUniform4("DataA", epriv::Postprocess_Bloom::Bloom.blur_radius,0.0f,hv.x,hv.y);
            sendTexture("image",gbuffer.getTexture(texture),0);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x,_y,0,0);
        }
        void _passFinal(GBuffer& gbuffer, Camera& c, const uint& fboWidth, const uint& fboHeight, GBufferType::Type sceneTexture){
            m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFinal]->bind();

            sendUniform1Safe("HasBloom",int(epriv::Postprocess_Bloom::Bloom.bloom));
            sendUniform1Safe("HasFog",int(epriv::Postprocess_Fog::Fog.fog));

            if(epriv::Postprocess_Fog::Fog.fog){
                sendUniform1Safe("FogDistNull", epriv::Postprocess_Fog::Fog.distNull);
                sendUniform1Safe("FogDistBlend", epriv::Postprocess_Fog::Fog.distBlend);
                sendUniform4Safe("FogColor", epriv::Postprocess_Fog::Fog.color);
                sendTextureSafe("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);
            }
            sendTextureSafe("SceneTexture", gbuffer.getTexture(sceneTexture), 0);
            sendTextureSafe("gBloomMap", gbuffer.getTexture(GBufferType::Bloom), 1);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _renderFullscreenQuad(const uint& width, const uint& height,uint startX,uint startY){
            float w2 = float(width) * 0.5f;
            float h2 = float(height) * 0.5f;
            glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            sendUniformMatrix4("Model", m_IdentityMat4);
            sendUniformMatrix4("VP", p);
            sendUniform2Safe("screenSizeDivideBy2", w2, h2);
            setViewport(startX, startY, width, height);
            m_FullscreenQuad->render();
        }
        void _renderFullscreenTriangle(const uint& width, const uint& height,uint startX,uint startY){
            float w2 = float(width) * 0.5f;
            float h2 = float(height) * 0.5f;
            glm::mat4 p = glm::ortho(-w2, w2, -h2, h2);
            sendUniformMatrix4("Model", m_IdentityMat4);
            sendUniformMatrix4("VP", p);
            sendUniform2Safe("screenSizeDivideBy2", w2, h2);
            setViewport(startX, startY, width, height);
            m_FullscreenTriangle->render();
        }
        void _render(GBuffer& gbuffer, Camera& camera, const uint& fboWidth, const uint& fboHeight,bool& HUD, Entity* ignore,const bool& mainRenderFunc, const GLuint& fbo, const GLuint& rbo){
            //TODO: find out why facing a certain direction causes around 2 - 3 ms frame spike times. determine if this is due to an object or a rendering
            //algorithm. also find out why enabling ssao REDUCES frame ms time. use opengl timers to isolate the troubling functions.
            
            Scene& s = *Resources::getCurrentScene();
            //this is god awful and ugly, but its needed. definately find a way to refactor this properly
            for (uint i = 0; i < 9; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
            if(mainRenderFunc){
                #pragma region Camera UBO
                if(RenderManager::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA){  
                    //TODO: change the manual camera uniform sending (for when glsl version < 140) to give a choice between the two render spaces
                    /*
                    //same simulation and render space
                    m_UBOCameraData.View = camera.getView();
                    m_UBOCameraData.Proj = camera.getProjection();
                    m_UBOCameraData.ViewProj = camera.getViewProjection();
                    m_UBOCameraData.InvProj = camera.getProjectionInverse();
                    m_UBOCameraData.InvView = camera.getViewInverse();
                    m_UBOCameraData.InvViewProj = camera.getViewProjectionInverse();
                    m_UBOCameraData.Info1 = glm::vec4(camera.getPosition(),camera.getNear());
                    m_UBOCameraData.Info2 = glm::vec4(camera.getViewVector(),camera.getFar());
                    m_UBOCameraData.Info3 = glm::vec4(0.0f,0.0f,0.0f, 0.0f);
                    */
                    
                    //this render space places the camera at the origin and offsets submitted model matrices to the vertex shaders by the camera's real simulation position
                    //this helps to deal with shading inaccuracies for when the camera is very far away from the origin
                    m_UBOCameraData.View = ComponentCamera_Functions::GetViewNoTranslation(camera);
                    m_UBOCameraData.Proj = camera.getProjection();
                    m_UBOCameraData.ViewProj = ComponentCamera_Functions::GetViewProjectionNoTranslation(camera);
                    m_UBOCameraData.InvProj = camera.getProjectionInverse();
                    m_UBOCameraData.InvView = ComponentCamera_Functions::GetViewInverseNoTranslation(camera);
                    m_UBOCameraData.InvViewProj = ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(camera);
                    m_UBOCameraData.Info1 = glm::vec4(0.001f,0.001f,0.001f, camera.getNear());
                    m_UBOCameraData.Info2 = glm::vec4(ComponentCamera_Functions::GetViewVectorNoTranslation(camera), camera.getFar());
                    m_UBOCameraData.Info3 = glm::vec4(camera.getPosition(), 0.0f);
                    
                    UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);           
                }
                #pragma endregion
                #pragma region LightProbes
                //if(s.lightProbes().size() > 0){
                    /*
                    for(auto& lightProbe:s->lightProbes()){
                        lightProbe.second->renderCubemap(
                            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude],
                            m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv]
                        );
                    }
                    */
                    //m_GBuffer->resize(fboWidth,fboHeight);
                //}
                #pragma endregion
            }
            _passGeometry(gbuffer,camera,fboWidth,fboHeight,ignore);
            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);

            #pragma region GodRays
            Renderer::GLDisable(GLState::BLEND_0);
            gbuffer.start(GBufferType::GodRays, "RGB", false);
            Settings::clear(true,false,false); //this is needed, clear color should be (0,0,0,0)
            auto& godRaysPlatform = epriv::Postprocess_GodRays::GodRays;
            if (godRaysPlatform.godRays && godRaysPlatform.sun){
                auto& body = *godRaysPlatform.sun->getComponent<ComponentBody>();
                glm::vec3 oPos = body.position();
                glm::vec3 camPos = camera.getPosition();
                glm::vec3 camVec = camera.getViewVector();
                bool infront = Math::isPointWithinCone(camPos, -camVec, oPos, Math::toRadians(godRaysPlatform.fovDegrees));
                if (infront) {
                    glm::vec3 sp = Math::getScreenCoordinates(oPos, false);
                    float alpha = Math::getAngleBetweenTwoVectors(camVec, camPos - oPos, true) / godRaysPlatform.fovDegrees;
                    
                    alpha = glm::pow(alpha, godRaysPlatform.alphaFalloff);
                    alpha = glm::clamp(alpha, 0.01f, 0.99f);
                    if (boost::math::isnan(alpha) || boost::math::isinf(alpha)) { //yes this is needed...
                        alpha = 0.01f;
                    }
                    alpha = 1.0f - alpha;
                    auto& godRaysShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredGodRays];
                    godRaysPlatform.pass(godRaysShader, gbuffer, fboWidth, fboHeight, glm::vec2(sp.x, sp.y),alpha); 
                }
            }

            #pragma endregion

            #pragma region SSAO
            //TODO: investigate why enabling SSAO makes things FASTER
            //TODO: possible optimization: use stencil buffer to reject completely black (or are they white?) pixels during blur passes


            gbuffer.start(GBufferType::Bloom, GBufferType::GodRays, "A", false);
            Settings::clear(true, false, false); //0,0,0,0



            if (epriv::Postprocess_SSAO::SSAO.m_ssao) {
                GLEnable(GLState::BLEND_0);//yes this is absolutely needed
                gbuffer.start(GBufferType::Bloom, "A", false);
                auto& ssaoShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSSAO];
                epriv::Postprocess_SSAO::SSAO.passSSAO(ssaoShader, gbuffer, fboWidth, fboHeight, camera);
                if (epriv::Postprocess_SSAO::SSAO.m_ssao_do_blur) {
                    GLDisable(GLState::BLEND_0); //yes this is absolutely needed
                    auto& ssaoBlurShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBlurSSAO];
                    for (uint i = 0; i < epriv::Postprocess_SSAO::SSAO.m_ssao_blur_num_passes; ++i) {
                        gbuffer.start(GBufferType::GodRays, "A", false);
                        epriv::Postprocess_SSAO::SSAO.passBlur(ssaoBlurShader, gbuffer, fboWidth, fboHeight, "H", GBufferType::Bloom);
                        gbuffer.start(GBufferType::Bloom, "A", false);
                        epriv::Postprocess_SSAO::SSAO.passBlur(ssaoBlurShader, gbuffer, fboWidth, fboHeight, "V", GBufferType::GodRays);
                    }
                }    
            }else{

            }
            #pragma endregion

            GLDisable(GLState::BLEND_0);

            _passStencil(gbuffer,camera,fboWidth,fboHeight); //confirmed, stencil rejection does help
            
            GLEnable(GLState::BLEND_0);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
            
            //this needs to be cleaned up
            if(lighting){
                gbuffer.start(GBufferType::Lighting,"RGB");
                Settings::clear(true,false,false);//this is needed for godrays 0,0,0,0
                _passLighting(gbuffer,camera,fboWidth,fboHeight,mainRenderFunc);
            }
            
            GLDisable(GLState::BLEND_0);
            GLDisable(GLState::STENCIL_TEST);
            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);
            _passForwardRendering(gbuffer,camera,fboWidth,fboHeight,nullptr);
            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);
            
            #pragma region HDR and GodRays addition
            gbuffer.start(GBufferType::Misc);
            ShaderP& hdrShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredHDR];
            epriv::Postprocess_HDR::HDR.pass(hdrShader, gbuffer, fboWidth, fboHeight, godRaysPlatform.godRays, lighting, godRaysPlatform.factor);
            #pragma endregion
            
            #pragma region Bloom
            //TODO: possible optimization: use stencil buffer to reject completely black pixels during blur passes
            if (epriv::Postprocess_Bloom::Bloom.bloom) {
                gbuffer.start(GBufferType::Bloom, "RGB", false);
                ShaderP& bloomShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredBloom];
                epriv::Postprocess_Bloom::Bloom.pass(bloomShader, gbuffer, fboWidth, fboHeight, GBufferType::Lighting);
                for (uint i = 0; i < epriv::Postprocess_Bloom::Bloom.num_passes; ++i) {
                    gbuffer.start(GBufferType::GodRays, "RGB", false);
                    _passBlur(gbuffer, camera, fboWidth, fboHeight, "H", GBufferType::Bloom);
                    gbuffer.start(GBufferType::Bloom, "RGB", false);
                    _passBlur(gbuffer, camera, fboWidth, fboHeight, "V", GBufferType::GodRays);
                }
            }
            #pragma endregion

            GBufferType::Type sceneTexture = GBufferType::Misc;
            GBufferType::Type outTexture = GBufferType::Lighting;
            #pragma region DOF
            if (epriv::Postprocess_DepthOfField::DOF.dof) {
                ShaderP& dofShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredDOF];
                gbuffer.start(outTexture);
                epriv::Postprocess_DepthOfField::DOF.pass(dofShader,gbuffer, fboWidth, fboHeight, sceneTexture);
                sceneTexture = GBufferType::Lighting;
                outTexture = GBufferType::Misc;
            }
            #pragma endregion

            #pragma region Finalization and AA
            if (!mainRenderFunc || aa_algorithm == AntiAliasingAlgorithm::None){
                gbuffer.stop(fbo, rbo);
                _passFinal(gbuffer, camera, fboWidth, fboHeight, sceneTexture);
            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::FXAA){
                gbuffer.start(outTexture);
                _passFinal(gbuffer, camera, fboWidth, fboHeight, sceneTexture);
                sceneTexture = outTexture;
                gbuffer.stop(fbo, rbo);
                auto& fxaaShader = *m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredFXAA];
                epriv::Postprocess_FXAA::FXAA.pass(fxaaShader, gbuffer, fboWidth, fboHeight, sceneTexture);
            }else if (mainRenderFunc && aa_algorithm == AntiAliasingAlgorithm::SMAA){
                gbuffer.start(outTexture);
                _passFinal(gbuffer, camera, fboWidth, fboHeight, sceneTexture);
                auto copy = sceneTexture;
                sceneTexture = outTexture;
                outTexture = copy;

                const float& _fboWidth  = float(fboWidth);
                const float& _fboHeight = float(fboHeight);
                const glm::vec4& SMAA_PIXEL_SIZE = glm::vec4(1.0f / _fboWidth, 1.0f / _fboHeight, _fboWidth, _fboHeight);

                ShaderP& edgeProgram     = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA1];
                ShaderP& blendProgram    = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA2];
                ShaderP& neighborProgram = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA3];
                ShaderP& finalProgram    = *m_InternalShaderPrograms[EngineInternalShaderPrograms::SMAA4];

                epriv::Postprocess_SMAA::SMAA.passEdge(edgeProgram, gbuffer, SMAA_PIXEL_SIZE, fboWidth, fboHeight, sceneTexture, outTexture);
                epriv::Postprocess_SMAA::SMAA.passBlend(blendProgram, gbuffer, SMAA_PIXEL_SIZE, fboWidth, fboHeight, outTexture);
                Renderer::GLDisable(GLState::STENCIL_TEST);
                epriv::Postprocess_SMAA::SMAA.passNeighbor(neighborProgram, gbuffer, SMAA_PIXEL_SIZE, fboWidth, fboHeight, sceneTexture);
                epriv::Postprocess_SMAA::SMAA.passFinal(finalProgram, gbuffer, fboWidth, fboHeight);//unused
            }
            
            #pragma endregion
            //_passCopyDepth(gbuffer,camera,fboWidth,fboHeight);

            #pragma region RenderPhysics
            GLEnable(GLState::BLEND_0);
            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);
            if(mainRenderFunc){
                if(draw_physics_debug  &&  &camera == s.getActiveCamera()){
                    m_InternalShaderPrograms[EngineInternalShaderPrograms::BulletPhysics]->bind();
                    Core::m_Engine->m_PhysicsManager._render();
                }
            }
            #pragma endregion

            //to try and see what the lightprobe is outputting
            /*
            Renderer::unbindFBO(true,true,true);
            Settings::clear();
            LightProbe* pr  = (LightProbe*)(Resources::getCamera("CapsuleLightProbe"));
            Skybox* skybox = (Skybox*)(s->getSkybox());
            if(pr){
                m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox]->bind();
                glm::mat4 view = glm::mat4(glm::mat3(camera->getView()));
                Renderer::sendUniformMatrix4f("VP",camera->getProjection() * view);
                GLuint address = pr->getEnvMap();
                sendTexture("Texture",address,0,GL_TEXTURE_CUBE_MAP);
                Skybox::bindMesh();
                //unbindTextureCubemap(0);
                //m_InternalShaderPrograms[EngineInternalShaderPrograms::DeferredSkybox]->unbind();
            }
            */
            
            #pragma region HUD
            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if(mainRenderFunc){
                if(HUD){
                    Settings::clear(false,true,false); //clear depth only
                    _renderTextures(gbuffer,camera,fboWidth,fboHeight);
                    _renderText(gbuffer,camera,fboWidth,fboHeight);
                }		
                vector_clear(m_FontsToBeRendered);
                vector_clear(m_TexturesToBeRendered);
            }
            #pragma endregion
        }
};


epriv::RenderManager::RenderManager(const char* name,uint w,uint h):m_i(new impl){ 
    m_i->_init(name,w,h); 
    renderManagerImpl = m_i.get();
    renderManager = this;
}
epriv::RenderManager::~RenderManager(){ m_i->_destruct(); }
void epriv::RenderManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::RenderManager::_render(Camera& c, const uint fboW, const uint fboH,bool HUD, Entity* ignore,const bool mainFunc, const GLuint display_fbo, const GLuint display_rbo){m_i->_render(*m_i->m_GBuffer,c,fboW,fboH,HUD,ignore,mainFunc,display_fbo,display_rbo);}
void epriv::RenderManager::_resize(uint w,uint h){ m_i->_resize(w,h); }
void epriv::RenderManager::_resizeGbuffer(uint w,uint h){ m_i->m_GBuffer->resize(w,h); }
void epriv::RenderManager::_onFullscreen(sf::Window* w,sf::VideoMode m,const char* n,uint s,sf::ContextSettings& set){ m_i->_onFullscreen(w,m,n,s,set); }
void epriv::RenderManager::_onOpenGLContextCreation(uint w,uint h,uint _glslVersion,uint _openglVersion){ m_i->_onOpenGLContextCreation(w,h,_glslVersion,_openglVersion); }
void epriv::RenderManager::_renderText(Font* font,string& text,glm::vec2& pos,glm::vec4& color,glm::vec2& scl,float& angle,float& depth){
    m_i->m_FontsToBeRendered.emplace_back(font,text,pos,color,scl,angle,depth);
}
void epriv::RenderManager::_renderTexture(Texture* texture,glm::vec2& pos,glm::vec4& color,glm::vec2& scl,float& angle,float& depth){
    m_i->m_TexturesToBeRendered.emplace_back(texture,pos,color,scl,angle,depth);
}
void epriv::RenderManager::_bindShaderProgram(ShaderP* p){
    auto& currentShaderPgrm = glSM.current_bound_shader_program;
    if(currentShaderPgrm != p){
        glUseProgram(p->program());
        currentShaderPgrm = p;
        currentShaderPgrm->BindableResource::bind();
    }
}
void epriv::RenderManager::_unbindShaderProgram() {
    auto& currentShaderPgrm = glSM.current_bound_shader_program;
    if (currentShaderPgrm) {
        currentShaderPgrm->BindableResource::unbind();
        currentShaderPgrm = nullptr;
        glUseProgram(0);
    }
}
void epriv::RenderManager::_bindMaterial(Material* m){
    auto& currentMaterial = glSM.current_bound_material;
    if(currentMaterial != m){
        currentMaterial = m;
        currentMaterial->BindableResource::bind();
    }
}
void epriv::RenderManager::_unbindMaterial(){
    auto& currentMaterial = glSM.current_bound_material;
    if(currentMaterial){
        currentMaterial->BindableResource::unbind();
        currentMaterial = nullptr;
    }
}
void epriv::RenderManager::_genPBREnvMapData(Texture& texture, uint size1, uint size2){
    m_i->_generatePBREnvMapData(texture,size1,size2);
}

void Renderer::Settings::General::enable1(bool b) { renderManagerImpl->enabled1 = b; }
void Renderer::Settings::General::disable1() { renderManagerImpl->enabled1 = false; }
bool Renderer::Settings::General::enabled1() { return renderManagerImpl->enabled1; }

void Renderer::Settings::Lighting::enable(bool b){ renderManagerImpl->lighting = b; }
void Renderer::Settings::Lighting::disable(){ renderManagerImpl->lighting = false; }
float Renderer::Settings::Lighting::getGIContributionGlobal(){ return renderManagerImpl->lighting_gi_contribution_global; }
void Renderer::Settings::Lighting::setGIContributionGlobal(float gi){ 
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_global = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
float Renderer::Settings::Lighting::getGIContributionDiffuse(){ return renderManagerImpl->lighting_gi_contribution_diffuse; }
void Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){ 
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_diffuse = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
float Renderer::Settings::Lighting::getGIContributionSpecular(){ return renderManagerImpl->lighting_gi_contribution_specular; }
void Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_specular = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
void Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    auto& mgr = *renderManagerImpl;
    mgr.lighting_gi_contribution_global = glm::clamp(g,0.001f,0.999f);
    mgr.lighting_gi_contribution_diffuse = glm::clamp(d,0.001f,0.999f);
    mgr.lighting_gi_contribution_specular = glm::clamp(s,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}

void Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm algorithm){ renderManagerImpl->_setAntiAliasingAlgorithm(algorithm); }
void Renderer::Settings::cullFace(uint s){ renderManagerImpl->_cullFace(s); }
void Renderer::Settings::clear(bool color, bool depth, bool stencil){
    if(!color && !depth && !stencil) return;
    GLuint clearBit = 0x00000000;
    if(color)   clearBit |= GL_COLOR_BUFFER_BIT;
    if(depth)   clearBit |= GL_DEPTH_BUFFER_BIT;
    if(stencil) clearBit |= GL_STENCIL_BUFFER_BIT;
    glClear(clearBit);
}
void Renderer::Settings::enableDrawPhysicsInfo(bool b){ renderManagerImpl->draw_physics_debug = b; }
void Renderer::Settings::disableDrawPhysicsInfo(){ renderManagerImpl->draw_physics_debug = false; }
void Renderer::Settings::setGamma(float g){ renderManagerImpl->gamma = g; }
float Renderer::Settings::getGamma(){ return renderManagerImpl->gamma; }
void Renderer::setDepthFunc(DepthFunc::Func func){ renderManagerImpl->_setDepthFunc(func); }
void Renderer::setViewport(uint x,uint y,uint w,uint h){ renderManagerImpl->_setViewport(x,y,w,h); }
void Renderer::colorMask(bool r, bool g, bool b, bool a) { renderManagerImpl->_colorMask(r,g,b,a); }
void Renderer::clearColor(float r, float g, float b, float a) { renderManagerImpl->_clearColor(r, g, b, a); }
void Renderer::bindTexture(GLuint _textureType,GLuint _textureObject){
    auto& i = *renderManager;
    switch(_textureType){
        case GL_TEXTURE_1D:{
            if(i.glSM.current_bound_texture_1D != _textureObject){
                i.glSM.current_bound_texture_1D = _textureObject;
                glBindTexture(_textureType,_textureObject);
                return;
            }
            break;
        }
        case GL_TEXTURE_2D:{
            if(i.glSM.current_bound_texture_2D != _textureObject){
                i.glSM.current_bound_texture_2D = _textureObject;
                glBindTexture(_textureType,_textureObject);
                return;
            }
            break;
        }
        case GL_TEXTURE_3D:{
            if(i.glSM.current_bound_texture_3D != _textureObject){
                i.glSM.current_bound_texture_3D = _textureObject;
                glBindTexture(_textureType,_textureObject);
                return;
            }
            break;
        }
        case GL_TEXTURE_CUBE_MAP:{
            if(i.glSM.current_bound_texture_cube_map != _textureObject){
                i.glSM.current_bound_texture_cube_map = _textureObject;
                glBindTexture(_textureType,_textureObject);
                return;
            }
            break;
        }
    }
}
void Renderer::bindVAO(const GLuint _vaoObject){
    auto& i = *renderManager;
    if(i.glSM.current_bound_vao != _vaoObject){
        glBindVertexArray(_vaoObject);
        i.glSM.current_bound_vao = _vaoObject;
    }
}
void Renderer::deleteVAO(GLuint& _vaoObject) {
    if (_vaoObject) {
        glDeleteVertexArrays(1,&_vaoObject);
        _vaoObject = 0;
    }
}
void Renderer::genAndBindTexture(GLuint _textureType,GLuint& _textureObject){
    glGenTextures(1, &_textureObject);
    bindTexture(_textureType,_textureObject);
}
void Renderer::genAndBindVAO(GLuint& _vaoObject){
    glGenVertexArrays(1, &_vaoObject);
    bindVAO(_vaoObject);
}
void Renderer::sendTexture(const char* location,Texture& texture,const int slot){
    Renderer::sendTexture(location,texture.address(),slot,texture.type());
}
void Renderer::sendTexture(const char* location,const GLuint textureAddress,const int slot,const GLuint targetType){
    glActiveTexture(GL_TEXTURE0 + slot);
    bindTexture(targetType,textureAddress);
    sendUniform1(location,slot);
}
void Renderer::sendTextureSafe(const char* location,Texture& texture,const int slot){
    Renderer::sendTextureSafe(location,texture.address(),slot,texture.type());
}
void Renderer::sendTextureSafe(const char* location,const GLuint textureAddress,const int slot,const GLuint targetType){
    glActiveTexture(GL_TEXTURE0 + slot);
    bindTexture(targetType,textureAddress);
    sendUniform1Safe(location,slot);
}
void Renderer::bindReadFBO(GLuint fbo){ renderManagerImpl->_bindReadFBO(fbo); }
void Renderer::bindFBO(epriv::FramebufferObject& fbo){ Renderer::bindFBO(fbo.address()); }
void Renderer::bindRBO(epriv::RenderbufferObject& rbo){ Renderer::bindRBO(rbo.address()); }
void Renderer::bindDrawFBO(GLuint fbo){ renderManagerImpl->_bindDrawFBO(fbo); }
void Renderer::bindFBO(GLuint fbo){Renderer::bindReadFBO(fbo);Renderer::bindDrawFBO(fbo);}
void Renderer::bindRBO(GLuint rbo){ renderManagerImpl->_bindRBO(rbo); }
void Renderer::unbindFBO(){ Renderer::bindFBO(GLuint(0)); }
void Renderer::unbindRBO(){ Renderer::bindRBO(GLuint(0)); }
void Renderer::unbindReadFBO(){ Renderer::bindReadFBO(0); }
void Renderer::unbindDrawFBO(){ Renderer::bindDrawFBO(0); }
void Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth){
    renderManagerImpl->m_TexturesToBeRendered.emplace_back(nullptr,pos,col,glm::vec2(w,h),angle,depth);
}
void Renderer::renderTexture(Texture& texture, const glm::vec2& pos, const glm::vec4& col,float angle, const glm::vec2& scl, float depth){
    texture.render(pos,col,angle,scl,depth);
}
void Renderer::renderText(const string& text,Font& font, const glm::vec2& pos, const glm::vec4& color, float angle, const glm::vec2& scl, float depth){
    font.renderText(text,pos,color,angle,scl,depth);
}
void Renderer::renderFullscreenQuad(uint w, uint h, uint startX, uint startY){ 
    renderManagerImpl->_renderFullscreenQuad(w,h,startX,startY); 
}
void Renderer::renderFullscreenTriangle(uint w,uint h, uint startX, uint startY){ 
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

inline const GLint Renderer::getUniformLoc(const char* location){
    auto& m = renderManager->glSM.current_bound_shader_program->uniforms(); if(!m.count(location)) return -1; return m.at(location);
}
inline const GLint& Renderer::getUniformLocUnsafe(const char* location){
    return renderManager->glSM.current_bound_shader_program->uniforms().at(location);
}
