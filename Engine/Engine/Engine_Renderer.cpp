#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_Renderer.h"
#include "Engine_Window.h"
#include "Engine_FullscreenItems.h"
#include "Engine_BuiltInShaders.h"
#include "Engine_BuiltInResources.h"
#include "Engine_Math.h"
#include "Engine_GLStateMachine.h"
#include "GBuffer.h"
#include "Camera.h"
#include "Light.h"
#include "Font.h"
#include "Scene.h"
#include "Texture.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Skybox.h"
#include "Material.h"
#include "FramebufferObject.h"
#include "SMAA_LUT.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
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

epriv::RenderManager::impl* renderManager;

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
            SMAAFrag1Stencil,
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
            DeferredLightingGI,
            CubemapConvolude,
            CubemapPrefilterEnv,
            BRDFPrecomputeCookTorrance,
            Grayscale,
            StencilPass,
            SMAA1Stencil,
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

            glm::vec4 Info1; //posx,posy,posz,near
            glm::vec4 Info2; //viewVecX,viewVecY,viewVecZ,far
        };
    };
};

class epriv::RenderManager::impl final{
    public:

        #pragma region DepthOfFieldInfo
        float dof_bias;
        float dof_focus;
        float dof_blur_radius;
        float dof_aspect_ratio;
        bool dof;
        #pragma endregion

        #pragma region FogInfo
        bool fog;
        float fog_distNull;
        float fog_distBlend;
        glm::vec4 fog_color;
        #pragma endregion

        #pragma region FXAAInfo
        float FXAA_REDUCE_MIN;
        float FXAA_REDUCE_MUL;
        float FXAA_SPAN_MAX;
        #pragma endregion

        #pragma region SMAAInfo
        GLuint SMAA_AreaTexture;
        GLuint SMAA_SearchTexture;
        float SMAA_THRESHOLD;
        uint SMAA_MAX_SEARCH_STEPS;
        uint SMAA_MAX_SEARCH_STEPS_DIAG;
        uint SMAA_CORNER_ROUNDING;
        float SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR;
        float SMAA_DEPTH_THRESHOLD;
        bool SMAA_PREDICATION;
        float SMAA_PREDICATION_THRESHOLD;
        float SMAA_PREDICATION_SCALE;
        float SMAA_PREDICATION_STRENGTH;
        bool SMAA_REPROJECTION;
        float SMAA_REPROJECTION_WEIGHT_SCALE;
        uint SMAA_AREATEX_MAX_DISTANCE;
        uint SMAA_AREATEX_MAX_DISTANCE_DIAG;
        glm::vec2 SMAA_AREATEX_PIXEL_SIZE;
        float SMAA_AREATEX_SUBTEX_SIZE;
        #pragma endregion

        #pragma region BloomInfo
        uint bloom_num_passes;
        bool bloom;
        float bloom_radius;
        float bloom_strength;
        float bloom_scale;
        #pragma endregion

        #pragma region LightingInfo
        bool lighting;
        float lighting_gi_contribution_diffuse;
        float lighting_gi_contribution_specular;
        float lighting_gi_contribution_global;
        float lighting_gi_pack;
        #pragma endregion

        #pragma region GodRaysInfo
        bool godRays;
        float godRays_exposure;
        float godRays_decay;
        float godRays_density;
        float godRays_weight;
        uint godRays_samples;
        float godRays_fovDegrees;
        float godRays_alphaFalloff;
        Entity* godRays_Object;
        #pragma endregion

        #pragma region SSAOInfo
        static const int SSAO_KERNEL_COUNT = 32;
        static const int SSAO_NORMALMAP_SIZE = 128;
        bool ssao;
        bool ssao_do_blur;
        uint ssao_samples;
        uint ssao_blur_num_passes;
        float ssao_blur_radius;
        float ssao_blur_strength;
        float ssao_scale;
        float ssao_intensity;
        float ssao_bias;
        float ssao_radius;
        glm::vec3 ssao_Kernels[SSAO_KERNEL_COUNT];
        GLuint ssao_noise_texture;
        #pragma endregion

        #pragma region HDRInfo
        bool hdr;
        float hdr_exposure;
        HDRAlgorithm::Algorithm hdr_algorithm;
        #pragma endregion

        #pragma region GeneralInfo

        bool enabled1;

        float gamma;
        Texture* brdfCook;
        ShaderP* current_shader_program;
        Material* current_bound_material;
        unsigned char cull_face_status;
        GLuint current_bound_vao;
        GLuint current_bound_read_fbo;
        GLuint current_bound_draw_fbo;
        GLuint current_bound_rbo;
        GLuint current_bound_texture_1D;
        GLuint current_bound_texture_2D;
        GLuint current_bound_texture_3D;
        GLuint current_bound_texture_cube_map;
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
        glm::mat4 m_IdentityMat4;
        glm::mat3 m_IdentityMat3;
        FullscreenQuad* m_FullscreenQuad;
        FullscreenTriangle* m_FullscreenTriangle;
        #pragma endregion

        #pragma region EngineInternalShadersAndPrograms
        vector<Shader*> m_InternalShaders;
        vector<ShaderP*> m_InternalShaderPrograms;

        epriv::UBOCamera m_UBOCameraData;
        #pragma endregion

        void _init(const char* name,uint& w,uint& h){
            #pragma region DepthOfFieldInfo
            dof_bias = 0.6f;
            dof_focus = 2.0f;
            dof_blur_radius = 3.0f;
            dof_aspect_ratio = 1.0f;
            dof = false;
            #pragma endregion

            #pragma region FogInfo
            fog = false;
            fog_distNull = 5.0f;
            fog_distBlend = 65.0f;
            fog_color = glm::vec4(1.0f,1.0f,1.0f,0.95f);
            #pragma endregion

            #pragma region FXAAInfo
            FXAA_REDUCE_MIN = 0.0078125f; // (1 / 128)
            FXAA_REDUCE_MUL = 0.125f;     // (1 / 8)
            FXAA_SPAN_MAX = 8.0f;
            #pragma endregion

            #pragma region SMAAInfo
            SMAA_THRESHOLD = 0.05f;
            SMAA_MAX_SEARCH_STEPS = 32;
            SMAA_MAX_SEARCH_STEPS_DIAG = 16;
            SMAA_CORNER_ROUNDING = 25;
            SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR = 2.0f;
            SMAA_DEPTH_THRESHOLD = (0.1f * SMAA_THRESHOLD);
            SMAA_PREDICATION = false;
            SMAA_PREDICATION_THRESHOLD = 0.01f;
            SMAA_PREDICATION_SCALE = 2.0f;
            SMAA_PREDICATION_STRENGTH = 0.4f;
            SMAA_REPROJECTION = false;
            SMAA_REPROJECTION_WEIGHT_SCALE = 30.0f;
            SMAA_AREATEX_MAX_DISTANCE = 16;
            SMAA_AREATEX_MAX_DISTANCE_DIAG = 20;
            SMAA_AREATEX_PIXEL_SIZE = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
            SMAA_AREATEX_SUBTEX_SIZE = 0.14285714285f; //(1 / 7)
            #pragma endregion

            #pragma region BloomInfo
            bloom_num_passes = 3;
            bloom = true;
            bloom_radius = 0.75f;
            bloom_strength = 0.6f;
            bloom_scale = 7.1f;
            #pragma endregion

            #pragma region LightingInfo
            lighting = true;
            lighting_gi_contribution_diffuse = 1.0f;
            lighting_gi_contribution_specular = 1.0f;
            lighting_gi_contribution_global = 1.0f;
            lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(lighting_gi_contribution_diffuse,lighting_gi_contribution_specular,lighting_gi_contribution_global);
            #pragma endregion

            #pragma region GodRaysInfo
            godRays = true;
            godRays_exposure = 0.15f;
            godRays_decay = 0.96815f;
            godRays_density = 0.926f;
            godRays_weight = 0.58767f;
            godRays_samples = 50;
            godRays_fovDegrees = 75.0f;
            godRays_alphaFalloff = 2.0f;
            godRays_Object = nullptr;
            #pragma endregion

            #pragma region SSAOInfo
            ssao = true;
            ssao_samples = 16;
            ssao_do_blur = true;
            ssao_blur_num_passes = 1;
            ssao_blur_radius = 0.36f;
            ssao_blur_strength = 0.48f;
            ssao_scale = 1.0f;
            ssao_intensity = 1.7f;
            ssao_bias = 0.1f;
            ssao_radius = 0.24f;
            #pragma endregion

            #pragma region HDRInfo
            hdr = true;
            hdr_exposure = 3.0f;
            hdr_algorithm = HDRAlgorithm::Uncharted;
            #pragma endregion

            #pragma region GeneralInfo

            enabled1 = true;

            gamma = 2.2f;
            brdfCook = nullptr;
            current_shader_program = nullptr;
            current_bound_material = nullptr;
            cull_face_status = 0; /* 0 = back | 1 = front | 2 = front and back */
            current_bound_vao = 0;
            current_bound_read_fbo = 0;
            current_bound_draw_fbo = 0;
            current_bound_rbo = 0;
            current_bound_texture_1D = 0;
            current_bound_texture_2D = 0;
            current_bound_texture_3D = 0;
            current_bound_texture_cube_map = 0;
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

            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::EXT_Ansiotropic_Filtering) = _checkOpenGLExtension("GL_EXT_texture_filter_anisotropic");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::ARB_Ansiotropic_Filtering) = _checkOpenGLExtension("GL_ARB_texture_filter_anisotropic");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::EXT_draw_instanced) = _checkOpenGLExtension("GL_EXT_draw_instanced");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::ARB_draw_instanced) = _checkOpenGLExtension("GL_ARB_draw_instanced");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::EXT_separate_shader_objects) = _checkOpenGLExtension("GL_EXT_separate_shader_objects");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::ARB_separate_shader_objects) = _checkOpenGLExtension("GL_ARB_separate_shader_objects");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::EXT_explicit_attrib_location) = _checkOpenGLExtension("GL_EXT_explicit_attrib_location");
            OPENGL_EXTENSIONS.at(OpenGLExtensionEnum::ARB_explicit_attrib_location) = _checkOpenGLExtension("GL_ARB_explicit_attrib_location");
            #pragma endregion

            epriv::EShaders::init();

            #pragma region EngineInternalShaderUBOs
            UniformBufferObject::UBO_CAMERA = new UniformBufferObject("Camera",sizeof(epriv::UBOCamera));
            UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
            #pragma endregion


            #pragma region EngineInternalShadersAndPrograms
            m_InternalShaders.resize(EngineInternalShaders::_TOTAL, nullptr);
            m_InternalShaderPrograms.resize(EngineInternalShaderPrograms::_TOTAL, nullptr);

            m_InternalShaders.at(EngineInternalShaders::BulletPhysicsVertex) = new Shader(epriv::EShaders::bullet_physics_vert,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::BulletPhysicsFrag) = new Shader(epriv::EShaders::bullet_physcis_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::LightingVertex) = new Shader(epriv::EShaders::lighting_vert,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::FullscreenVertex) = new Shader(epriv::EShaders::fullscreen_quad_vertex,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::FXAAFrag) = new Shader(epriv::EShaders::fxaa_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::VertexBasic) = new Shader(epriv::EShaders::vertex_basic,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::VertexHUD) = new Shader(epriv::EShaders::vertex_hud,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::VertexSkybox) = new Shader(epriv::EShaders::vertex_skybox,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::ForwardFrag) = new Shader(epriv::EShaders::forward_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::DeferredFrag) = new Shader(epriv::EShaders::deferred_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::DOFFrag) = new Shader(epriv::EShaders::depth_of_field, ShaderType::Fragment, false);
            m_InternalShaders.at(EngineInternalShaders::DeferredFragHUD) = new Shader(epriv::EShaders::deferred_frag_hud,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::DeferredFragSkybox) = new Shader(epriv::EShaders::deferred_frag_skybox,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::DeferredFragSkyboxFake) = new Shader(epriv::EShaders::deferred_frag_skybox_fake,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::CopyDepthFrag) = new Shader(epriv::EShaders::copy_depth_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SSAOFrag) = new Shader(epriv::EShaders::ssao_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::BloomFrag) = new Shader(epriv::EShaders::bloom_frag, ShaderType::Fragment, false);
            m_InternalShaders.at(EngineInternalShaders::HDRFrag) = new Shader(epriv::EShaders::hdr_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::BlurFrag) = new Shader(epriv::EShaders::blur_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SSAOBlurFrag) = new Shader(epriv::EShaders::ssao_blur_frag, ShaderType::Fragment, false);
            m_InternalShaders.at(EngineInternalShaders::GodRaysFrag) = new Shader(epriv::EShaders::godRays_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::FinalFrag) = new Shader(epriv::EShaders::final_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::LightingFrag) = new Shader(epriv::EShaders::lighting_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::LightingGIFrag) = new Shader(epriv::EShaders::lighting_frag_gi,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::CubemapConvoludeFrag) = new Shader(epriv::EShaders::cubemap_convolude_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::CubemapPrefilterEnvFrag) = new Shader(epriv::EShaders::cubemap_prefilter_envmap_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::BRDFPrecomputeFrag) = new Shader(epriv::EShaders::brdf_precompute,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::GrayscaleFrag) = new Shader(epriv::EShaders::greyscale_frag,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::StencilPassFrag) = new Shader(epriv::EShaders::stencil_passover,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAVertex1) = new Shader(epriv::EShaders::smaa_vertex_1,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAVertex2) = new Shader(epriv::EShaders::smaa_vertex_2,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAVertex3) = new Shader(epriv::EShaders::smaa_vertex_3,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAVertex4) = new Shader(epriv::EShaders::smaa_vertex_4,ShaderType::Vertex,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAFrag1Stencil) = new Shader(epriv::EShaders::smaa_frag_1_stencil,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAFrag1) = new Shader(epriv::EShaders::smaa_frag_1,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAFrag2) = new Shader(epriv::EShaders::smaa_frag_2,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAFrag3) = new Shader(epriv::EShaders::smaa_frag_3,ShaderType::Fragment,false);
            m_InternalShaders.at(EngineInternalShaders::SMAAFrag4) = new Shader(epriv::EShaders::smaa_frag_4,ShaderType::Fragment,false);

            epriv::InternalShaderPrograms::Deferred = new ShaderP("Deferred",m_InternalShaders.at(EngineInternalShaders::VertexBasic),m_InternalShaders.at(EngineInternalShaders::DeferredFrag));
            epriv::InternalShaderPrograms::Forward = new ShaderP("Forward",m_InternalShaders.at(EngineInternalShaders::VertexBasic),m_InternalShaders.at(EngineInternalShaders::ForwardFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BulletPhysics) = new ShaderP("Bullet_Physics",m_InternalShaders.at(EngineInternalShaders::BulletPhysicsVertex),m_InternalShaders.at(EngineInternalShaders::BulletPhysicsFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD) = new ShaderP("Deferred_HUD",m_InternalShaders.at(EngineInternalShaders::VertexHUD),m_InternalShaders.at(EngineInternalShaders::DeferredFragHUD));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredGodRays) = new ShaderP("Deferred_GodsRays",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::GodRaysFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlur) = new ShaderP("Deferred_Blur",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::BlurFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlurSSAO) = new ShaderP("Deferred_Blur_SSAO", m_InternalShaders.at(EngineInternalShaders::FullscreenVertex), m_InternalShaders.at(EngineInternalShaders::SSAOBlurFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHDR) = new ShaderP("Deferred_HDR",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::HDRFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSSAO) = new ShaderP("Deferred_SSAO",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::SSAOFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredDOF) = new ShaderP("Deferred_DOF", m_InternalShaders.at(EngineInternalShaders::FullscreenVertex), m_InternalShaders.at(EngineInternalShaders::DOFFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBloom) = new ShaderP("Deferred_Bloom", m_InternalShaders.at(EngineInternalShaders::FullscreenVertex), m_InternalShaders.at(EngineInternalShaders::BloomFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFinal) = new ShaderP("Deferred_Final",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::FinalFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFXAA) = new ShaderP("Deferred_FXAA",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::FXAAFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox) = new ShaderP("Deferred_Skybox",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::DeferredFragSkybox));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkyboxFake) = new ShaderP("Deferred_Skybox_Fake",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::DeferredFragSkyboxFake));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CopyDepth) = new ShaderP("Copy_Depth",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::CopyDepthFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLighting) = new ShaderP("Deferred_Light",m_InternalShaders.at(EngineInternalShaders::LightingVertex),m_InternalShaders.at(EngineInternalShaders::LightingFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLightingGI) = new ShaderP("Deferred_Light_GI",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::LightingGIFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude) = new ShaderP("Cubemap_Convolude",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::CubemapConvoludeFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv) = new ShaderP("Cubemap_Prefilter_Env",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::CubemapPrefilterEnvFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance) = new ShaderP("BRDF_Precompute_CookTorrance",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::BRDFPrecomputeFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::Grayscale) = new ShaderP("Greyscale_Frag",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::GrayscaleFrag));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::StencilPass) = new ShaderP("Stencil_Pass",m_InternalShaders.at(EngineInternalShaders::LightingVertex),m_InternalShaders.at(EngineInternalShaders::StencilPassFrag));

            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1Stencil) = new ShaderP("Deferred_SMAA_1_Stencil",m_InternalShaders.at(EngineInternalShaders::SMAAVertex1),m_InternalShaders.at(EngineInternalShaders::SMAAFrag1Stencil));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1) = new ShaderP("Deferred_SMAA_1",m_InternalShaders.at(EngineInternalShaders::SMAAVertex1),m_InternalShaders.at(EngineInternalShaders::SMAAFrag1));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA2) = new ShaderP("Deferred_SMAA_2",m_InternalShaders.at(EngineInternalShaders::SMAAVertex2),m_InternalShaders.at(EngineInternalShaders::SMAAFrag2));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA3) = new ShaderP("Deferred_SMAA_3",m_InternalShaders.at(EngineInternalShaders::SMAAVertex3),m_InternalShaders.at(EngineInternalShaders::SMAAFrag3));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA4) = new ShaderP("Deferred_SMAA_4",m_InternalShaders.at(EngineInternalShaders::SMAAVertex4),m_InternalShaders.at(EngineInternalShaders::SMAAFrag4));
            
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

            epriv::InternalMeshes::PointLightBounds = new Mesh(pointLightMesh,false,0.0005f);
            epriv::InternalMeshes::RodLightBounds = new Mesh(rodLightData,false,0.0005f);
            epriv::InternalMeshes::SpotLightBounds = new Mesh(spotLightData,false,0.0005f);

            Mesh::FontPlane = new Mesh("FontPlane",1.0f,1.0f,0.0005f);
            Mesh::Plane = new Mesh("Plane",1.0f,1.0f,0.0005f);
            Mesh::Cube = new Mesh(cubeMesh,false,0.0005f);

            brdfCook = new Texture(512,512,ImagePixelType::FLOAT,ImagePixelFormat::RG,ImageInternalFormat::RG16F);
            brdfCook->setWrapping(TextureWrap::ClampToEdge);	
            epriv::Core::m_Engine->m_ResourceManager->_addTexture(brdfCook);

            #pragma endregion

            m_FullscreenQuad = new FullscreenQuad();
            m_FullscreenTriangle = new FullscreenTriangle();

            uniform_real_distribution<float> randFloats(0.0f,1.0f);
            default_random_engine gen;
            for(uint i = 0; i < SSAO_KERNEL_COUNT; ++i){
                glm::vec3 sample(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,randFloats(gen));
                sample = glm::normalize(sample);
                sample *= randFloats(gen);
                float scale = float(i) / float(SSAO_KERNEL_COUNT);
                float a = 0.1f; float b = 1.0f; float f = scale * scale;
                scale = a + f * (b - a); //basic lerp   
                sample *= scale;
                ssao_Kernels[i] = sample;
            }
            vector<glm::vec3> ssaoNoise;
            for(uint i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i){
                ssaoNoise.emplace_back(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,0.0f);
            }
            genAndBindTexture(GL_TEXTURE_2D,ssao_noise_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE,SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
            GLEnable(GLState::DEPTH_TEST);
            Renderer::setDepthFunc(DepthFunc::LEqual);
            glClearDepth(1.0f);
            glClearStencil(0);
            GLDisable(GLState::STENCIL_TEST);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1); //for non Power of Two textures
    
            //GLEnable(GLState::TEXTURE_CUBE_MAP_SEAMLESS); //very wierd, supported on my gpu and opengl version but it runs REAL slowly, dropping fps to 1
            GLEnable(GLState::DEPTH_CLAMP);

            genAndBindTexture(GL_TEXTURE_2D,SMAA_AreaTexture);
            Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RG8,160,560,0,GL_RG,GL_UNSIGNED_BYTE,areaTexBytes);

            genAndBindTexture(GL_TEXTURE_2D,SMAA_SearchTexture);
            Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
            glTexImage2D(GL_TEXTURE_2D,0,GL_R8,64,16,0,GL_RED,GL_UNSIGNED_BYTE,searchTexBytes);

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

            SAFE_DELETE(epriv::InternalShaderPrograms::Deferred);
            SAFE_DELETE(epriv::InternalShaderPrograms::Forward);

            SAFE_DELETE_VECTOR(m_InternalShaderPrograms);
            SAFE_DELETE_VECTOR(m_InternalShaders);

            glDeleteTextures(1,&ssao_noise_texture);
            glDeleteTextures(1,&SMAA_SearchTexture);
            glDeleteTextures(1,&SMAA_AreaTexture);
        }
        bool _checkOpenGLExtension(const char* e){ if(glewIsExtensionSupported(e)!=0) return true;return 0!=glewIsSupported(e); }
        void _renderSkybox(SkyboxEmpty* skybox){
            Scene* scene = Resources::getCurrentScene();
            Camera* c = scene->getActiveCamera();
            glm::mat4 view = c->getView();
            Math::removeMatrixPosition(view);
            if(skybox){
                m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->bind();
                sendUniformMatrix4f("VP",c->getProjection() * view);
                sendTexture("Texture",skybox->texture()->address(0),0,GL_TEXTURE_CUBE_MAP);
                Skybox::bindMesh();
                skybox->draw();
            }else{//render a fake skybox.
                m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkyboxFake)->bind();
                glm::vec3 bgColor = scene->getBackgroundColor();
                sendUniformMatrix4f("VP",c->getProjection() * view);
                sendUniform4f("Color",bgColor.r,bgColor.g,bgColor.b,1.0f);
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
            glEnable(GL_TEXTURE_2D);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClearDepth(1.0f);
            glClearStencil(0);

            m_GBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);
        }
        void _onOpenGLContextCreation(uint& width,uint& height,uint& _glslVersion,uint _openglVersion){
            epriv::RenderManager::GLSL_VERSION = _glslVersion;
            epriv::RenderManager::OPENGL_VERSION = _openglVersion;
            glewExperimental = GL_TRUE;
            glewInit(); glGetError();//stupid glew always inits an error. nothing we can do about it.
            GLEnable(GLState::TEXTURE_2D); //is this really needed?
            GLEnable(GLState::CULL_FACE);
            Settings::cullFace(GL_BACK);
            SAFE_DELETE(m_GBuffer);
            m_GBuffer = new GBuffer(width,height);
        }
        void _generatePBREnvMapData(Texture* texture,uint& convoludeTextureSize, uint& preEnvFilterSize){
            uint texType = texture->type();
            if(texType != GL_TEXTURE_CUBE_MAP){
                cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
            }
            uint size = convoludeTextureSize;
            bindTexture(texType, texture->address(1));
            Renderer::unbindFBO();
            epriv::FramebufferObject* fbo = new epriv::FramebufferObject(texture->name() + "_fbo_envData",size,size); //try without a depth format
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
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude)->bind();

            sendTexture("cubemap",texture->address(),0,texType);
            setViewport(0,0,size,size);
            for (uint i = 0; i < 6; ++i){
                glm::mat4 vp = captureProjection * captureViews[i];
                sendUniformMatrix4f("VP",vp);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture->address(1),0);
                Settings::clear(true,true,false);
                Skybox::bindMesh();
            }
            Resources::getWindow()->display(); //prevent opengl & windows timeout
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude)->unbind();

            //now gen EnvPrefilterMap for specular IBL
            size = preEnvFilterSize;
            bindTexture(texType, texture->address(2));
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv)->bind();
            sendTexture("cubemap",texture->address(),0,texType);
            sendUniform1f("PiFourDividedByResSquaredTimesSix",12.56637f / float((texture->width() * texture->width())*6));
            sendUniform1i("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
                fbo->resize(mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                sendUniform1f("roughness",roughness);
                float a = roughness * roughness;
                sendUniform1f("a2",a*a);
                for (uint i = 0; i < 6; ++i){
                    glm::mat4 vp = captureProjection * captureViews[i];
                    sendUniformMatrix4f("VP", vp);
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture->address(2),m);
                    Settings::clear(true,true,false);
                    Skybox::bindMesh();
                }
            }
            Resources::getWindow()->display(); //prevent opengl & windows timeout
            fbo->unbind();
            delete fbo;
        }
        void _generateBRDFLUTCookTorrance(uint brdfSize){
            uint& prevReadBuffer = current_bound_read_fbo;
            uint& prevDrawBuffer = current_bound_draw_fbo;

            FramebufferObject* fbo = new FramebufferObject("BRDFLUT_Gen_CookTorr_FBO",brdfSize,brdfSize); //try without a depth format
            fbo->bind();

            bindTexture(GL_TEXTURE_2D, brdfCook->address());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
            Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
            Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToEdge);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,brdfCook->address(), 0);

            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance)->bind();
            sendUniform1i("NUM_SAMPLES",256);
            Settings::clear(true,true,false);
            Renderer::colorMask(true,true,false,false);
            _renderFullscreenTriangle(brdfSize,brdfSize,0,0);
            //cout << "----  BRDF LUT (Cook Torrance) completed ----" << endl;
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance)->unbind();
            Renderer::colorMask(true, true, true, true);

            delete fbo;
            bindReadFBO(prevReadBuffer);
            bindDrawFBO(prevDrawBuffer);
        }
        void _setSMAAQuality(SMAAQualityLevel::Level& l){
            if(l == SMAAQualityLevel::Low){
                SMAA_THRESHOLD = 0.15f;           SMAA_MAX_SEARCH_STEPS = 4;
                SMAA_MAX_SEARCH_STEPS_DIAG = 0;   SMAA_CORNER_ROUNDING = 0;
            }else if(l == SMAAQualityLevel::Medium){
                SMAA_THRESHOLD = 0.1f;            SMAA_MAX_SEARCH_STEPS = 8;
                SMAA_MAX_SEARCH_STEPS_DIAG = 0;   SMAA_CORNER_ROUNDING = 0;
            }else if(l == SMAAQualityLevel::High){
                SMAA_THRESHOLD = 0.1f;            SMAA_MAX_SEARCH_STEPS = 16;
                SMAA_MAX_SEARCH_STEPS_DIAG = 8;   SMAA_CORNER_ROUNDING = 25;
            }else if(l == SMAAQualityLevel::Ultra){
                SMAA_THRESHOLD = 0.05f;           SMAA_MAX_SEARCH_STEPS = 32;
                SMAA_MAX_SEARCH_STEPS_DIAG = 16;  SMAA_CORNER_ROUNDING = 25;
            }
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
            if(current_bound_read_fbo != f){
                glBindFramebuffer(GL_READ_FRAMEBUFFER, f);
                current_bound_read_fbo = f;
            }
        }
        void _bindDrawFBO(uint& f){
            if(current_bound_draw_fbo != f){
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f);
                current_bound_draw_fbo = f;
            }
        }
        void _bindRBO(uint& r){
            if(current_bound_rbo != r){
                glBindRenderbuffer(GL_RENDERBUFFER, r);
                current_bound_rbo = r;
            }
        }
        void _renderTextures(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->bind();
            Mesh::Plane->bind();
            glm::mat4 m = m_IdentityMat4;
            for(auto item:m_TexturesToBeRendered){
                if(item.texture){
                    sendTexture("DiffuseTexture",item.texture,0);
                    sendUniform1i("DiffuseTextureEnabled",1);
                }
                else{
                    sendTexture("DiffuseTexture",0,0);
                    sendUniform1i("DiffuseTextureEnabled",0);
                }
                sendUniform4f("Object_Color",item.col);

                m = m_IdentityMat4;
                m = glm::translate(m, glm::vec3(item.pos.x,item.pos.y,-0.001f - item.depth));
                m = glm::rotate(m, item.rot,glm::vec3(0.0f,0.0f,1.0f));
                if(item.texture)
                    m = glm::scale(m, glm::vec3(item.texture->width(),item.texture->height(),1.0f));
                m = glm::scale(m, glm::vec3(item.scl.x,item.scl.y,1.0f));

                sendUniformMatrix4f("VP",m_2DProjectionMatrix);
                sendUniformMatrix4f("Model",m);

                Mesh::Plane->render(false);
            }
        }
        void _renderText(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->bind();
            glm::mat4 m = m_IdentityMat4;
            float y_offset = 0.0f;
            float x = 0.0f;  
            Mesh& mesh = *(Mesh::FontPlane);
            for(auto item:m_FontsToBeRendered){
                Font& font = *item.font;
                sendTexture("DiffuseTexture",font.getGlyphTexture(),0);
                sendUniform1i("DiffuseTextureEnabled",1);
                sendUniform4f("Object_Color",item.col);
                y_offset = 0;
                x = item.pos.x;        
                for(auto c:item.text){
                    if(c == '\n'){
                        y_offset += (font.getGlyphData('X')->height + 6) * item.scl.y;
                        x = item.pos.x;
                    }
                    else{
                        FontGlyph& chr = *(font.getGlyphData(c));
                        m = m_IdentityMat4;
                        m = glm::translate(m, glm::vec3(x + chr.xoffset ,item.pos.y - (chr.height + chr.yoffset) - y_offset,-0.001f - item.depth));
                        m = glm::rotate(m, item.rot,glm::vec3(0,0,1));
                        m = glm::scale(m, glm::vec3(item.scl.x,item.scl.y,1));

                        sendUniformMatrix4f("VP",m_2DProjectionMatrix);
                        sendUniformMatrix4f("Model",m);

                        mesh.modifyPointsAndUVs(chr.pts,chr.uvs);
                        mesh.bind();
                        mesh.render(false);

                        x += chr.xadvance * item.scl.x;
                    }
                }
            }
        }
        void _passGeometry(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight,Entity* ignore){
            Scene* scene = Resources::getCurrentScene();
            glm::vec3 camPos = scene->getActiveCamera()->getPosition();
            const glm::vec3& clear = scene->getBackgroundColor();
            const float colors[4] = { clear.r,clear.g,clear.b,1.0f };  
    
            if(godRays){ gbuffer.start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); }
            else{        gbuffer.start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA"); }

            Settings::clear(true,true,true);
            Renderer::setDepthFunc(DepthFunc::LEqual);
            GLDisable(GLState::BLEND);//disable blending on all mrts

            glClearBufferfv(GL_COLOR,0,colors);
            if(godRays){
                const float godRays[4] = { 0.03f,0.023f,0.032f,1.0f };
                glClearBufferfv(GL_COLOR,3,godRays);
            }

            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);

            //this is needed for sure
            glEnablei(GL_BLEND,0);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);      

            //RENDER NORMAL OBJECTS HERE
            InternalScenePublicInterface::Render(scene);

            //skybox here
            gbuffer.start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA");
            _renderSkybox(scene->skybox());
            if(godRays){ gbuffer.start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); }          
        }
        void _passForwardRendering(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight,Entity* ignore){
            Scene* scene = Resources::getCurrentScene();

            gbuffer.start(GBufferType::Diffuse);

            //RENDER NORMAL OBJECTS HERE
            InternalScenePublicInterface::RenderForward(scene);
        }
        void _passCopyDepth(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CopyDepth)->bind();

            sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),0);

            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            Renderer::colorMask(true, true, true, true);
        }
        void _passLighting(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight,bool mainRenderFunc){
            Scene* s = Resources::getCurrentScene(); 
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLighting)->bind();
            
            if(RenderManager::GLSL_VERSION < 140){
                sendUniformMatrix4fSafe("CameraView",c.getView());
                sendUniformMatrix4fSafe("CameraProj",c.getProjection());
                //sendUniformMatrix4fSafe("CameraViewProj",c.getViewProjection()); //moved to shader binding function
                sendUniformMatrix4fSafe("CameraInvView",c.getViewInverse());
                sendUniformMatrix4fSafe("CameraInvProj",c.getProjectionInverse());
                sendUniformMatrix4fSafe("CameraInvViewProj",c.getViewProjectionInverse());
                sendUniform4fSafe("CameraInfo1",glm::vec4(glm::vec3(0.0001f),c.getNear()));
                sendUniform4fSafe("CameraInfo2",glm::vec4(c.getViewVectorNoTranslation(),c.getFar()));
            }
            
            sendUniform4fv("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
            sendUniform4f("ScreenData",0.0f,gamma,(float)fboWidth,(float)fboHeight);

            sendTexture("gDiffuseMap",gbuffer.getTexture(GBufferType::Diffuse),0);
            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),1);
            sendTexture("gMiscMap",gbuffer.getTexture(GBufferType::Misc),2);
            sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),3);
            sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 4);

            for (auto light: epriv::InternalScenePublicInterface::GetLights(s)){
                light->lighten();
            }
            if(mainRenderFunc){
                //do GI here. (only doing GI during the main render pass, not during light probes
                m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLightingGI)->bind();
                
                if(RenderManager::GLSL_VERSION < 140){
                    sendUniformMatrix4fSafe("CameraInvView",c.getViewInverse());
                    sendUniformMatrix4fSafe("CameraInvProj",c.getProjectionInverse());
                    sendUniformMatrix4fSafe("CameraInvViewProj",c.getViewProjectionInverse());
                    sendUniform4fSafe("CameraInfo1",glm::vec4(glm::vec3(0.0001f),c.getNear()));
                    sendUniform4fSafe("CameraInfo2",glm::vec4(c.getViewVectorNoTranslation(),c.getFar()));
                }
                
                sendUniform4fv("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
                sendUniform4f("ScreenData",lighting_gi_pack,gamma,(float)fboWidth,(float)fboHeight);
                sendTexture("gDiffuseMap",gbuffer.getTexture(GBufferType::Diffuse),0);
                sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),1);
                sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);
                sendTexture("gSSAOMap", gbuffer.getTexture(GBufferType::Bloom), 3);

                SkyboxEmpty* skybox = s->skybox();

                if(s->lightProbes().size() > 0){
                    /*
                    for(auto probe:s->lightProbes()){
                        LightProbe* p = probe.second;
                        sendTextureSafe("irradianceMap",p->getIrriadianceMap(),4,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("prefilterMap",p->getPrefilterMap(),5,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("brdfLUT",brdfCook,6);
                        break;
                    }
                    */
                }else{
                    if(skybox && skybox->texture()->numAddresses() >= 3){
                        sendTextureSafe("irradianceMap",skybox->texture()->address(1),4,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("prefilterMap",skybox->texture()->address(2),5,GL_TEXTURE_CUBE_MAP);
                        sendTextureSafe("brdfLUT",brdfCook,6);
                    }
                }
                _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            }
            GLDisable(GLState::STENCIL_TEST);
        }
        void _passSSAO(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSSAO)->bind();
            float _divisor = gbuffer.getSmallFBO()->divisor();
            if(RenderManager::GLSL_VERSION < 140){
                sendUniformMatrix4fSafe("CameraInvViewProj",c.getViewProjectionInverse());
                sendUniformMatrix4fSafe("CameraInvProj",c.getProjectionInverse());
                sendUniform4fSafe("CameraInfo1",glm::vec4(glm::vec3(0.0001f),c.getNear()));
                sendUniform4fSafe("CameraInfo2",glm::vec4(c.getViewVectorNoTranslation(),c.getFar()));
            }  
            sendUniform4f("SSAOInfo",ssao_radius,ssao_intensity,ssao_bias,ssao_scale);
            sendUniform4i("SSAOInfoA",int(ssao),int(bloom),ssao_samples,SSAO_NORMALMAP_SIZE);//change to 4f eventually?

            sendUniform3fv("poisson[0]",ssao_Kernels,SSAO_KERNEL_COUNT);

            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),0);
            sendTexture("gRandomMap",ssao_noise_texture,1,GL_TEXTURE_2D);
            sendTexture("gDepthMap",gbuffer.getTexture(GBufferType::Depth),2);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x,_y,0,0);
        }
        void _passBloom(GBuffer& gbuffer, Camera& c, uint& fboWidth, uint& fboHeight) {
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBloom)->bind();
            float _divisor = gbuffer.getSmallFBO()->divisor();
            sendUniform1f("bloomScale", bloom_scale);

            sendTexture("gMiscMap", gbuffer.getTexture(GBufferType::Misc), 0);
            sendTexture("gLightMap", gbuffer.getTexture(GBufferType::Lighting), 1);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x, _y, 0, 0);
        }
        void _passStencil(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            Renderer::colorMask(false, false, false, false);
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::StencilPass)->bind();

            Scene& s = *Resources::getCurrentScene();
            gbuffer.getMainFBO()->bind();

            GLEnable(GLState::STENCIL_TEST);
            Settings::clear(false,false,true); //stencil is completely filled with 0's
            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_ALWAYS, 0x00000000, 0x00000000);
            
            //exclude shadeless normals
            glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
            sendTexture("gNormalMap",gbuffer.getTexture(GBufferType::Normal),0);
            sendUniform1f("Type",0.0f);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);


            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_NOTEQUAL, 0x00000000, 0xFFFFFFFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil

            Renderer::colorMask(true, true, true, true);
        }
        void _passGodsRays(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight,glm::vec2 lightScrnPos,bool behind,float alpha){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredGodRays)->bind();
            float _divisor = gbuffer.getSmallFBO()->divisor();
            sendUniform4f("RaysInfo",godRays_exposure,godRays_decay,godRays_density,godRays_weight);
            sendUniform2f("lightPositionOnScreen",lightScrnPos.x/float(fboWidth),lightScrnPos.y/float(fboHeight));
            sendUniform1i("samples",godRays_samples);
            sendUniform1i("behind",int(behind));
            sendUniform1f("alpha",alpha);
            sendTexture("firstPass",gbuffer.getTexture(GBufferType::Lighting),0);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x,_y,0,0);
        }
        void _passHDR(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHDR)->bind();

            sendUniform4fSafe("HDRInfo",hdr_exposure,float(int(hdr)), godRays_exposure,float(int(hdr_algorithm)));
            sendUniform2iSafe("Has",int(godRays), int(lighting));

            sendTextureSafe("lightingBuffer",gbuffer.getTexture(GBufferType::Lighting),0);
            sendTextureSafe("gDiffuseMap",gbuffer.getTexture(GBufferType::Diffuse),1);
            sendTextureSafe("gNormalMap",gbuffer.getTexture(GBufferType::Normal),2);
            sendTextureSafe("gGodsRaysMap",gbuffer.getTexture(GBufferType::GodRays),3);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _passDOF(GBuffer& gbuffer, Camera& c, uint& fboWidth, uint& fboHeight) {
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredDOF)->bind();

            sendUniform4fSafe("Data",dof_blur_radius,dof_bias,dof_focus,dof_aspect_ratio);
    
            sendTextureSafe("inTexture", gbuffer.getTexture(GBufferType::Lighting), 0);
            sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

            _renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
        }
        void _passBlur(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight,string type, GLuint texture,string channels){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlur)->bind();

            float _divisor = gbuffer.getSmallFBO()->divisor();
            glm::vec4 rgba(0.0f);
            glm::vec2 hv(0.0f);
            if(channels.find("R") != string::npos) rgba.x = 1.0f;
            if(channels.find("G") != string::npos) rgba.y = 1.0f;
            if(channels.find("B") != string::npos) rgba.z = 1.0f;
            if(channels.find("A") != string::npos) rgba.w = 1.0f;
            if(type == "H"){ hv = glm::vec2(1.0f,0.0f); }
            else{            hv = glm::vec2(0.0f,1.0f); }

            sendUniform4f("strengthModifier",bloom_strength,bloom_strength,bloom_strength,ssao_blur_strength);
            sendUniform4f("DataA",bloom_radius,0.0f,hv.x,hv.y);
            sendUniform4f("RGBA",rgba);
            sendTexture("image",gbuffer.getTexture(texture),0);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x,_y,0,0);
        }
        void _passBlurSSAO(GBuffer& gbuffer, Camera& c, uint& fboWidth, uint& fboHeight, string type, GLuint texture) {
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlurSSAO)->bind();

            float _divisor = gbuffer.getSmallFBO()->divisor();
            glm::vec2 hv(0.0f);

            if (type == "H") { hv = glm::vec2(1.0f, 0.0f); } else { hv = glm::vec2(0.0f, 1.0f); }

            sendUniform1f("strengthModifier", ssao_blur_strength);
            sendUniform4f("Data", ssao_blur_radius, 0.0f, hv.x, hv.y);
            sendTexture("image", gbuffer.getTexture(texture), 0);

            uint _x = uint(float(fboWidth) * _divisor);
            uint _y = uint(float(fboHeight) * _divisor);
            _renderFullscreenTriangle(_x, _y, 0, 0);
        }
        void _passFXAA(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFXAA)->bind();

            sendUniform1f("FXAA_REDUCE_MIN",FXAA_REDUCE_MIN);
            sendUniform1f("FXAA_REDUCE_MUL",FXAA_REDUCE_MUL);
            sendUniform1f("FXAA_SPAN_MAX",FXAA_SPAN_MAX);

            sendUniform2f("invRes",1.0f / float(fboWidth),1.0f / float(fboHeight));
            sendTexture("inTexture",gbuffer.getTexture(GBufferType::Lighting),0);
            sendTextureSafe("edgeTexture",gbuffer.getTexture(GBufferType::Misc),1);
            sendTexture("depthTexture",gbuffer.getTexture(GBufferType::Depth),2);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _passSMAA(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            float _fboWidth = float(fboWidth);
            float _fboHeight = float(fboHeight);
            glm::vec4 SMAA_PIXEL_SIZE = glm::vec4(1.0f / _fboWidth, 1.0f / _fboHeight, _fboWidth, _fboHeight);

            #pragma region PassEdge
            gbuffer.start(GBufferType::Misc);
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1)->bind();

            Settings::clear(true,false,true);//color, stencil is completely filled with 0's

            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
            glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
            GLEnable(GLState::STENCIL_TEST);

            sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);

            sendUniform4fSafe("SMAAInfo1Floats",SMAA_THRESHOLD,SMAA_DEPTH_THRESHOLD,SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR,SMAA_PREDICATION_THRESHOLD);
            sendUniform2fSafe("SMAAInfo1FloatsA",SMAA_PREDICATION_SCALE,SMAA_PREDICATION_STRENGTH);

            sendUniform1iSafe("SMAA_PREDICATION",int(SMAA_PREDICATION));

            sendTexture("textureMap",gbuffer.getTexture(GBufferType::Lighting),0);
            sendTextureSafe("texturePredication",gbuffer.getTexture(GBufferType::Diffuse),1);

            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);

            glStencilMask(0xFFFFFFFF);
            glStencilFunc(GL_EQUAL, 0x00000001, 0x00000001);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil

            #pragma endregion
    
            #pragma region PassBlend
            gbuffer.start(GBufferType::Normal);
            Settings::clear(true,false,false); //clear color only

            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA2)->bind();
            sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);

            sendTexture("edge_tex",gbuffer.getTexture(GBufferType::Misc),0);
            sendTexture("area_tex",SMAA_AreaTexture,1,GL_TEXTURE_2D);
            sendTexture("search_tex",SMAA_SearchTexture,2,GL_TEXTURE_2D);

            sendUniform1iSafe("SMAA_MAX_SEARCH_STEPS",SMAA_MAX_SEARCH_STEPS);

            sendUniform4iSafe("SMAAInfo2Ints",SMAA_MAX_SEARCH_STEPS_DIAG,SMAA_AREATEX_MAX_DISTANCE,SMAA_AREATEX_MAX_DISTANCE_DIAG,SMAA_CORNER_ROUNDING);
            sendUniform4fSafe("SMAAInfo2Floats",SMAA_AREATEX_PIXEL_SIZE.x,SMAA_AREATEX_PIXEL_SIZE.y,SMAA_AREATEX_SUBTEX_SIZE,(float(SMAA_CORNER_ROUNDING) / 100.0f));

            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            #pragma endregion

            GLDisable(GLState::STENCIL_TEST);

            #pragma region PassNeighbor
            //gbuffer.start(GBufferType::Misc);
            gbuffer.stop();
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA3)->bind();
            sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
            sendTextureSafe("textureMap",gbuffer.getTexture(GBufferType::Lighting),0); //need original final image from first smaa pass
            sendTextureSafe("blend_tex",gbuffer.getTexture(GBufferType::Normal),1);

            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            #pragma endregion

            #pragma region PassFinalCustom
            /*
            //this pass is optional. lets skip it for now
            //gbuffer.start(GBufferType::Lighting);
            gbuffer.stop();
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA4)->bind();
            renderFullscreenTriangle(fboWidth,fboHeight,0,0);
            */  
            #pragma endregion
        }
        void _passFinal(GBuffer& gbuffer,Camera& c,uint& fboWidth, uint& fboHeight){
            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFinal)->bind();

            sendUniform1iSafe("HasBloom",int(bloom));
            sendUniform1iSafe("HasFog",int(fog));

            if(fog){
                sendUniform1fSafe("FogDistNull",fog_distNull);
                sendUniform1fSafe("FogDistBlend",fog_distBlend);
                sendUniform4fSafe("FogColor",fog_color);
                sendTextureSafe("gDepthMap",gbuffer.getTexture(GBufferType::Depth),3);
            }
            sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 0);
            sendTextureSafe("gMiscMap", gbuffer.getTexture(GBufferType::Misc), 1);
            sendTextureSafe("gBloomMap", gbuffer.getTexture(GBufferType::Bloom), 2);
            _renderFullscreenTriangle(fboWidth,fboHeight,0,0);
        }
        void _renderFullscreenQuad(uint& width,uint& height,uint startX,uint startY){
            float w2 = float(width) * 0.5f;
            float h2 = float(height) * 0.5f;
            glm::mat4 p = glm::ortho(-w2,w2,-h2,h2);
            sendUniformMatrix4f("MVP",p);
            sendUniform2fSafe("screenSizeDivideBy2",w2,h2);
            setViewport(startX,startY,width,height);
            m_FullscreenQuad->render();
        }
        void _renderFullscreenTriangle(uint& width,uint& height,uint startX,uint startY){
            float w2 = float(width) * 0.5f;
            float h2 = float(height) * 0.5f;
            glm::mat4 p = glm::ortho(-w2,w2,-h2,h2);
            sendUniformMatrix4f("MVP",p);
            sendUniform2fSafe("screenSizeDivideBy2",w2,h2);
            setViewport(startX,startY,width,height);
            m_FullscreenTriangle->render();
        }
        void _render(GBuffer& gbuffer,Camera& camera,uint& fboWidth,uint& fboHeight,bool& HUD, Entity* ignore,bool& mainRenderFunc,GLuint& fbo, GLuint& rbo){
            Scene* s = Resources::getCurrentScene();
            //restore default state, might have to increase this as we use more textures
            for(uint i = 0; i < 7; ++i){ 
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D,0);
                glBindTexture(GL_TEXTURE_CUBE_MAP,0);
            }
            if(mainRenderFunc){
                #pragma region Camera UBO
                //Camera UBO update
                //NOTE: camera render info is different than simulated camera info, the position of the camera is always at the origin to prevent
                //shading and render calculation errors. Likewise the model matrices of the objects sent to the rendering pipeline
                //have their positions offset by the camera to make up for this shift
                if(RenderManager::GLSL_VERSION >= 140 && UniformBufferObject::UBO_CAMERA){
                    m_UBOCameraData.View = camera.getView();
                    m_UBOCameraData.Proj = camera.getProjection();
                    m_UBOCameraData.ViewProj = camera.getViewProjection();
                    m_UBOCameraData.InvProj = camera.getProjectionInverse();
                    m_UBOCameraData.InvView = camera.getViewInverse();
                    m_UBOCameraData.InvViewProj = camera.getViewProjectionInverse();
                    m_UBOCameraData.Info1 = glm::vec4(glm::vec3(0.0001f),camera.getNear());
                    m_UBOCameraData.Info2 = glm::vec4(camera.getViewVectorNoTranslation(),camera.getFar());
                    UniformBufferObject::UBO_CAMERA->updateData(&m_UBOCameraData);
                
                
                }
                #pragma endregion
                #pragma region LightProbes
                if(s->lightProbes().size() > 0){
                    /*
                    for(auto lightProbe:s->lightProbes()){
                        lightProbe.second->renderCubemap(
                            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude),
                            m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv)
                        );
                    }
                    */
                    m_GBuffer->resize(fboWidth,fboHeight);
                }
                #pragma endregion
            }

            _passGeometry(gbuffer,camera,fboWidth,fboHeight,ignore);

            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);

            #pragma region GodRays
            if (godRays && godRays_Object) {
                gbuffer.start(GBufferType::GodRays, "RGBA", false);
                auto& body = *godRays_Object->getComponent<ComponentBody>();
                glm::vec3 oPos = body.position();
                glm::vec3 sp = Math::getScreenCoordinates(oPos, false);
                glm::vec3 camPos = camera.getPosition();
                glm::vec3 camVec = camera.getViewVector();
                bool behind = Math::isPointWithinCone(camPos, -camVec, oPos, Math::toRadians(godRays_fovDegrees));
                float alpha = Math::getAngleBetweenTwoVectors(camVec, camPos - oPos, true) / godRays_fovDegrees;

                alpha = glm::pow(alpha, godRays_alphaFalloff);
                alpha = glm::clamp(alpha, 0.0001f, 0.9999f);

                Settings::clear(true,false,false);
                _passGodsRays(gbuffer, camera, fboWidth, fboHeight, glm::vec2(sp.x, sp.y), !behind, 1.0f - alpha);
            }
            #pragma endregion

            #pragma region SSAO
            gbuffer.start(GBufferType::Bloom, "A", false);
            Settings::clear(true, false, false);
            if (ssao) {
                _passSSAO(gbuffer, camera, fboWidth, fboHeight);
                if (ssao_do_blur) {
                    for (uint i = 0; i < ssao_blur_num_passes; ++i) {
                        gbuffer.start(GBufferType::GodRays, "A", false);
                        _passBlurSSAO(gbuffer, camera, fboWidth, fboHeight, "H", GBufferType::Bloom);
                        gbuffer.start(GBufferType::Bloom, "A", false);
                        _passBlurSSAO(gbuffer, camera, fboWidth, fboHeight, "V", GBufferType::GodRays);
                    }
                }
            }   
            #pragma endregion

            GLDisable(GLState::BLEND);

            _passStencil(gbuffer,camera,fboWidth,fboHeight); //confirmed, stencil rejection does help

            GLEnable(GLState::BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

            if(lighting && epriv::InternalScenePublicInterface::GetLights(s).size() > 0){
                gbuffer.start(GBufferType::Lighting,"RGB");
                Settings::clear(true,false,false);//this is needed for godrays
                _passLighting(gbuffer,camera,fboWidth,fboHeight,mainRenderFunc);
            }

            GLDisable(GLState::BLEND);
            GLDisable(GLState::STENCIL_TEST);
            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);
            _passForwardRendering(gbuffer,camera,fboWidth,fboHeight,nullptr);
            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);
            
            #pragma region HDR and GodRays addition
            gbuffer.start(GBufferType::Misc);
            _passHDR(gbuffer,camera,fboWidth,fboHeight);
            #pragma endregion

            #pragma region Bloom
            if (bloom) {
                gbuffer.start(GBufferType::Bloom, "RGB", false);
                _passBloom(gbuffer, camera, fboWidth, fboHeight);
                for (uint i = 0; i < bloom_num_passes; ++i) {
                    gbuffer.start(GBufferType::GodRays, "RGB", false);
                    _passBlur(gbuffer, camera, fboWidth, fboHeight, "H", GBufferType::Bloom, "RGB");
                    gbuffer.start(GBufferType::Bloom, "RGB", false);
                    _passBlur(gbuffer, camera, fboWidth, fboHeight, "V", GBufferType::GodRays, "RGB");
                }
            }
            #pragma endregion

            #pragma region DOF
            if (dof) {
                gbuffer.start(GBufferType::Lighting);
                _passDOF(gbuffer, camera, fboWidth, fboHeight);
            }
            #pragma endregion

            #pragma region Finalization and AA
            //not the main render function - dont do AA
            if (!mainRenderFunc){
                gbuffer.stop(fbo, rbo);
                _passFinal(gbuffer, camera, fboWidth, fboHeight);
            }else{
                if (aa_algorithm == AntiAliasingAlgorithm::None){
                    gbuffer.stop(fbo, rbo);
                    _passFinal(gbuffer, camera, fboWidth, fboHeight);
                }else if (aa_algorithm == AntiAliasingAlgorithm::FXAA){
                    gbuffer.start(GBufferType::Lighting);
                    _passFinal(gbuffer, camera, fboWidth, fboHeight);
                    gbuffer.stop(fbo, rbo);
                    _passFXAA(gbuffer, camera, fboWidth, fboHeight);
                }else if (aa_algorithm == AntiAliasingAlgorithm::SMAA){
                    gbuffer.start(GBufferType::Lighting);
                    _passFinal(gbuffer, camera, fboWidth, fboHeight);
                    _passSMAA(gbuffer, camera, fboWidth, fboHeight);
                }
            }
            #pragma endregion
            //_passCopyDepth(gbuffer,camera,fboWidth,fboHeight);

            #pragma region RenderPhysics
            GLEnable(GLState::BLEND);
            GLDisable(GLState::DEPTH_TEST);
            GLDisable(GLState::DEPTH_MASK);
            if(mainRenderFunc){
                if(draw_physics_debug  &&  &camera == s->getActiveCamera()){
                    m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BulletPhysics)->bind();
                    Core::m_Engine->m_PhysicsManager->_render();
                }
            }
            #pragma endregion

            //to try and see what the lightprobe is outputting
            /*
            Renderer::unbindFBO();
            Settings::clear();
            LightProbe* pr  = (LightProbe*)(Resources::getCamera("CapsuleLightProbe"));
            Skybox* skybox = (Skybox*)(s->getSkybox());
            if(pr){
                m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->bind();
                glm::mat4 view = glm::mat4(glm::mat3(camera->getView()));
                Renderer::sendUniformMatrix4f("VP",camera->getProjection() * view);
                GLuint address = pr->getEnvMap();
                sendTexture("Texture",address,0,GL_TEXTURE_CUBE_MAP);
                Skybox::bindMesh();
                //unbindTextureCubemap(0);
                m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->unbind();
            }
            */
    
            #pragma region HUD
            GLEnable(GLState::DEPTH_TEST);
            GLEnable(GLState::DEPTH_MASK);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if(mainRenderFunc){
                if(HUD){
                    Settings::clear(false,true,false); //clear depth only
                    GLEnable(GLState::ALPHA_TEST);
                    glAlphaFunc(GL_GREATER, 0.1f);
                    _renderTextures(gbuffer,camera,fboWidth,fboHeight);
                    _renderText(gbuffer,camera,fboWidth,fboHeight);
                    GLDisable(GLState::ALPHA_TEST);
                }		
                vector_clear(m_FontsToBeRendered);
                vector_clear(m_TexturesToBeRendered);
            }
            #pragma endregion
        }
};
epriv::RenderManager::RenderManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); renderManager = m_i.get(); }
epriv::RenderManager::~RenderManager(){ m_i->_destruct(); }
void epriv::RenderManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::RenderManager::_render(Camera* c,uint fboW,uint fboH,bool HUD,Entity* ignore,bool mainFunc,GLuint display_fbo,GLuint display_rbo){m_i->_render(*m_i->m_GBuffer,*c,fboW,fboH,HUD,ignore,mainFunc,display_fbo,display_rbo);}
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
    auto& currentShaderPgrm = m_i->current_shader_program;
    if(currentShaderPgrm != p){
        glUseProgram(p->program());
        currentShaderPgrm = p;
        currentShaderPgrm->BindableResource::bind();
    }
}
void epriv::RenderManager::_unbindShaderProgram() {
    auto& currentShaderPgrm = m_i->current_shader_program;
    if (currentShaderPgrm) {
        currentShaderPgrm->BindableResource::unbind();
        currentShaderPgrm = nullptr;
        glUseProgram(0);
    }
}
void epriv::RenderManager::_bindMaterial(Material* m){
    auto& currentMaterial = m_i->current_bound_material;
    if(currentMaterial != m){
        currentMaterial = m;
        currentMaterial->BindableResource::bind();
    }
}
void epriv::RenderManager::_unbindMaterial(){
    auto& currentMaterial = m_i->current_bound_material;
    if(currentMaterial){
        currentMaterial->BindableResource::unbind();
        currentMaterial = nullptr;
    }
}
void epriv::RenderManager::_genPBREnvMapData(Texture* texture, uint size1, uint size2){
    m_i->_generatePBREnvMapData(texture,size1,size2);
}



epriv::RenderPipeline::RenderPipeline(ShaderP* _shaderProgram) {
    shaderProgram = _shaderProgram;
}
epriv::RenderPipeline::~RenderPipeline() {
    SAFE_DELETE_VECTOR(materialNodes);
}
void epriv::RenderPipeline::render() {
    shaderProgram->bind();
    for (auto materialNode : materialNodes) {
        if (materialNode->meshNodes.size() > 0) {
            materialNode->material->bind();
            for (auto meshNode : materialNode->meshNodes) {
                if (meshNode->instanceNodes.size() > 0) {
                    meshNode->mesh->bind();
                    for (auto instanceNode : meshNode->instanceNodes) {
                        if (instanceNode->instance->passedRenderCheck()) {
                            instanceNode->instance->bind();
                            meshNode->mesh->render(false);
                            instanceNode->instance->unbind();
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if (renderManager->current_shader_program != shaderProgram) {
                        shaderProgram->bind();
                        materialNode->material->bind();
                    }
                    meshNode->mesh->unbind();
                }
            }
        }
    }
}

void Renderer::Settings::General::enable1(bool b) { renderManager->enabled1 = b; }
void Renderer::Settings::General::disable1() { renderManager->enabled1 = false; }
bool Renderer::Settings::General::enabled1() { return renderManager->enabled1; }


void Renderer::Settings::DepthOfField::enable(bool b) { renderManager->dof = b; }
void Renderer::Settings::DepthOfField::disable() { renderManager->dof = false; }
bool Renderer::Settings::DepthOfField::enabled() { return renderManager->dof; }
float Renderer::Settings::DepthOfField::getFocus() { return renderManager->dof_focus; }
void Renderer::Settings::DepthOfField::setFocus(float f) { renderManager->dof_focus = glm::max(0.0f, f); }
float Renderer::Settings::DepthOfField::getBias() { return renderManager->dof_bias; }
void Renderer::Settings::DepthOfField::setBias(float b) { renderManager->dof_bias = b; }
float Renderer::Settings::DepthOfField::getBlurRadius() { return renderManager->dof_blur_radius; }
void Renderer::Settings::DepthOfField::setBlurRadius(float r) { renderManager->dof_blur_radius = glm::max(0.0f, r); }
float Renderer::Settings::DepthOfField::getAspectRatio() { return renderManager->dof_aspect_ratio; }
void Renderer::Settings::DepthOfField::setAspectRatio(float r) { renderManager->dof_aspect_ratio = r; }
bool Renderer::Settings::Fog::enabled(){ return renderManager->fog; }
void Renderer::Settings::Fog::enable(bool b){ renderManager->fog = b; }
void Renderer::Settings::Fog::disable(){ renderManager->fog = false; }
void Renderer::Settings::Fog::setColor(glm::vec4& color){ Renderer::Settings::Fog::setColor(color.r,color.g,color.b,color.a); }
void Renderer::Settings::Fog::setColor(float r,float g,float b,float a){ Math::setColor(renderManager->fog_color,r,g,b,a); }
void Renderer::Settings::Fog::setNullDistance(float d){ renderManager->fog_distNull = d; }
void Renderer::Settings::Fog::setBlendDistance(float d){ renderManager->fog_distBlend = d; }
float Renderer::Settings::Fog::getNullDistance(){ return renderManager->fog_distNull; }
float Renderer::Settings::Fog::getBlendDistance(){ return renderManager->fog_distBlend; }
void Renderer::Settings::FXAA::setReduceMin(float r){ renderManager->FXAA_REDUCE_MIN = glm::max(0.0f,r); }
void Renderer::Settings::FXAA::setReduceMul(float r){ renderManager->FXAA_REDUCE_MUL = glm::max(0.0f,r); }
void Renderer::Settings::FXAA::setSpanMax(float r){ renderManager->FXAA_SPAN_MAX = glm::max(0.0f,r); }
float Renderer::Settings::FXAA::getReduceMin(){ return renderManager->FXAA_REDUCE_MIN; }
float Renderer::Settings::FXAA::getReduceMul(){ return renderManager->FXAA_REDUCE_MUL; }
float Renderer::Settings::FXAA::getSpanMax(){ return renderManager->FXAA_SPAN_MAX; }
bool Renderer::Settings::HDR::enabled(){ return renderManager->hdr; }
void Renderer::Settings::HDR::enable(bool b){ renderManager->hdr = b; }
void Renderer::Settings::HDR::disable(){ renderManager->hdr = false; }
float Renderer::Settings::HDR::getExposure(){ return renderManager->hdr_exposure; }
void Renderer::Settings::HDR::setExposure(float e){ renderManager->hdr_exposure = e; }
void Renderer::Settings::HDR::setAlgorithm(HDRAlgorithm::Algorithm a){ renderManager->hdr_algorithm = a; }
bool Renderer::Settings::Bloom::enabled() { return renderManager->bloom; }
uint Renderer::Settings::Bloom::getNumPasses() { return renderManager->bloom_num_passes; }
void Renderer::Settings::Bloom::setNumPasses(uint p) { renderManager->bloom_num_passes = p; }
void Renderer::Settings::Bloom::enable(bool b){ renderManager->bloom = b; }
void Renderer::Settings::Bloom::disable(){ renderManager->bloom = false; }
float Renderer::Settings::Bloom::getRadius(){ return renderManager->bloom_radius; }
float Renderer::Settings::Bloom::getStrength(){ return renderManager->bloom_strength; }
void Renderer::Settings::Bloom::setRadius(float r){ renderManager->bloom_radius = glm::max(0.0f,r); }
void Renderer::Settings::Bloom::setStrength(float r){ renderManager->bloom_strength = glm::max(0.0f,r); }
float Renderer::Settings::Bloom::getScale(){ return renderManager->bloom_scale; }
void Renderer::Settings::Bloom::setScale(float s){ renderManager->bloom_scale = glm::max(0.0f,s); }
void Renderer::Settings::SMAA::setThreshold(float f){ renderManager->SMAA_THRESHOLD = f; }
void Renderer::Settings::SMAA::setSearchSteps(uint s){ renderManager->SMAA_MAX_SEARCH_STEPS = s; }
void Renderer::Settings::SMAA::disableCornerDetection(){ renderManager->SMAA_CORNER_ROUNDING = 0; }
void Renderer::Settings::SMAA::enableCornerDetection(uint c){ renderManager->SMAA_CORNER_ROUNDING = c; }
void Renderer::Settings::SMAA::disableDiagonalDetection(){ renderManager->SMAA_MAX_SEARCH_STEPS_DIAG = 0; }
void Renderer::Settings::SMAA::enableDiagonalDetection(uint d){ renderManager->SMAA_MAX_SEARCH_STEPS_DIAG = d; }
void Renderer::Settings::SMAA::setPredicationThreshold(float f){ renderManager->SMAA_PREDICATION_THRESHOLD = f; }
void Renderer::Settings::SMAA::setPredicationScale(float f){ renderManager->SMAA_PREDICATION_SCALE = f; }
void Renderer::Settings::SMAA::setPredicationStrength(float s){ renderManager->SMAA_PREDICATION_STRENGTH = s; }
void Renderer::Settings::SMAA::setReprojectionScale(float s){ renderManager->SMAA_REPROJECTION_WEIGHT_SCALE = s; }
void Renderer::Settings::SMAA::enablePredication(bool b){ renderManager->SMAA_PREDICATION = b; }
void Renderer::Settings::SMAA::disablePredication(){ renderManager->SMAA_PREDICATION = false; }
void Renderer::Settings::SMAA::enableReprojection(bool b){ renderManager->SMAA_REPROJECTION = b; }
void Renderer::Settings::SMAA::disableReprojection(){ renderManager->SMAA_REPROJECTION = false; }
void Renderer::Settings::SMAA::setQuality(SMAAQualityLevel::Level level){ renderManager->_setSMAAQuality(level); }
bool Renderer::Settings::GodRays::enabled(){ return renderManager->godRays; }
void Renderer::Settings::GodRays::enable(bool b = true){ renderManager->godRays = b; }
void Renderer::Settings::GodRays::disable(){ renderManager->godRays = false; }
float Renderer::Settings::GodRays::getExposure(){ return renderManager->godRays_exposure; }
float Renderer::Settings::GodRays::getDecay(){ return renderManager->godRays_decay; }
float Renderer::Settings::GodRays::getDensity(){ return renderManager->godRays_density; }
float Renderer::Settings::GodRays::getWeight(){ return renderManager->godRays_weight; }
uint Renderer::Settings::GodRays::getSamples(){ return renderManager->godRays_samples; }
float Renderer::Settings::GodRays::getFOVDegrees(){ return renderManager->godRays_fovDegrees; }
float Renderer::Settings::GodRays::getAlphaFalloff(){ return renderManager->godRays_alphaFalloff; }
void Renderer::Settings::GodRays::setExposure(float e){ renderManager->godRays_exposure = e; }
void Renderer::Settings::GodRays::setDecay(float d){ renderManager->godRays_decay = d; }
void Renderer::Settings::GodRays::setDensity(float d){ renderManager->godRays_density = d; }
void Renderer::Settings::GodRays::setWeight(float w){ renderManager->godRays_weight = w; }
void Renderer::Settings::GodRays::setSamples(uint s){ renderManager->godRays_samples = s; }
void Renderer::Settings::GodRays::setFOVDegrees(float d){ renderManager->godRays_fovDegrees = d; }
void Renderer::Settings::GodRays::setAlphaFalloff(float a){ renderManager->godRays_alphaFalloff = a; }
void Renderer::Settings::GodRays::setObject(uint& id){ renderManager->godRays_Object = Components::GetEntity(id); }
void Renderer::Settings::GodRays::setObject(Entity* entity){ renderManager->godRays_Object = entity; }
Entity* Renderer::Settings::GodRays::getObject(){ return renderManager->godRays_Object; }
void Renderer::Settings::Lighting::enable(bool b){ renderManager->lighting = b; }
void Renderer::Settings::Lighting::disable(){ renderManager->lighting = false; }
float Renderer::Settings::Lighting::getGIContributionGlobal(){ return renderManager->lighting_gi_contribution_global; }
void Renderer::Settings::Lighting::setGIContributionGlobal(float gi){ 
    auto mgr = *renderManager;
    mgr.lighting_gi_contribution_global = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
float Renderer::Settings::Lighting::getGIContributionDiffuse(){ return renderManager->lighting_gi_contribution_diffuse; }
void Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){ 
    auto mgr = *renderManager;
    mgr.lighting_gi_contribution_diffuse = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
float Renderer::Settings::Lighting::getGIContributionSpecular(){ return renderManager->lighting_gi_contribution_specular; }
void Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    auto mgr = *renderManager;
    mgr.lighting_gi_contribution_specular = glm::clamp(gi,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
void Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    auto mgr = *renderManager;
    mgr.lighting_gi_contribution_global = glm::clamp(g,0.001f,0.999f);
    mgr.lighting_gi_contribution_diffuse = glm::clamp(d,0.001f,0.999f);
    mgr.lighting_gi_contribution_specular = glm::clamp(s,0.001f,0.999f);
    mgr.lighting_gi_pack = Math::pack3FloatsInto1FloatUnsigned(mgr.lighting_gi_contribution_diffuse,mgr.lighting_gi_contribution_specular,mgr.lighting_gi_contribution_global);
}
bool Renderer::Settings::SSAO::enabled(){ return renderManager->ssao;  }
void Renderer::Settings::SSAO::enable(bool b){ renderManager->ssao = b;  }
void Renderer::Settings::SSAO::disable(){ renderManager->ssao = false;  }
void Renderer::Settings::SSAO::enableBlur(bool b){ renderManager->ssao_do_blur = b;  }
void Renderer::Settings::SSAO::disableBlur(){ renderManager->ssao_do_blur = false;  }
float Renderer::Settings::SSAO::getBlurRadius() { return renderManager->ssao_blur_radius; }
void Renderer::Settings::SSAO::setBlurRadius(float r) { renderManager->ssao_blur_radius = glm::max(0.0f, r); }
float Renderer::Settings::SSAO::getBlurStrength(){ return renderManager->ssao_blur_strength; }
float Renderer::Settings::SSAO::getIntensity(){ return renderManager->ssao_intensity; }
float Renderer::Settings::SSAO::getRadius(){ return renderManager->ssao_radius; }
float Renderer::Settings::SSAO::getScale(){ return renderManager->ssao_scale; }
float Renderer::Settings::SSAO::getBias(){ return renderManager->ssao_bias; }
uint Renderer::Settings::SSAO::getSamples(){ return renderManager->ssao_samples; }
void Renderer::Settings::SSAO::setBlurStrength(float s){ renderManager->ssao_blur_strength = glm::max(0.0f,s); }
void Renderer::Settings::SSAO::setIntensity(float i){ renderManager->ssao_intensity = glm::max(0.0f,i); }
void Renderer::Settings::SSAO::setRadius(float r){ renderManager->ssao_radius = glm::max(0.0f,r); }
void Renderer::Settings::SSAO::setScale(float s){ renderManager->ssao_scale = glm::max(0.0f,s); }
void Renderer::Settings::SSAO::setBias(float b){ renderManager->ssao_bias = b; }
void Renderer::Settings::SSAO::setSamples(uint s){ renderManager->ssao_samples = s; }
void Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm algorithm){ renderManager->_setAntiAliasingAlgorithm(algorithm); }
void Renderer::Settings::cullFace(uint s){ renderManager->_cullFace(s); }
void Renderer::Settings::clear(bool color, bool depth, bool stencil){
    if(!color && !depth && !stencil) return;
    GLuint clearBit = 0x00000000;
    if(color)   clearBit |= GL_COLOR_BUFFER_BIT;
    if(depth)   clearBit |= GL_DEPTH_BUFFER_BIT;
    if(stencil) clearBit |= GL_STENCIL_BUFFER_BIT;
    glClear(clearBit);
}
void Renderer::Settings::enableDrawPhysicsInfo(bool b){ renderManager->draw_physics_debug = b; }
void Renderer::Settings::disableDrawPhysicsInfo(){ renderManager->draw_physics_debug = false; }
void Renderer::Settings::setGamma(float g){ renderManager->gamma = g; }
float Renderer::Settings::getGamma(){ return renderManager->gamma; }
void Renderer::setDepthFunc(DepthFunc::Func func){ renderManager->_setDepthFunc(func); }
void Renderer::setViewport(uint x,uint y,uint w,uint h){ renderManager->_setViewport(x,y,w,h); }
void Renderer::colorMask(bool r, bool g, bool b, bool a) { renderManager->_colorMask(r,g,b,a); }
void Renderer::clearColor(float r, float g, float b, float a) { renderManager->_clearColor(r, g, b, a); }
void Renderer::bindTexture(GLuint _textureType,GLuint _textureObject){
    auto& i = *renderManager;
    switch(_textureType){
        case GL_TEXTURE_1D:{
            if(i.current_bound_texture_1D != _textureObject){
                i.current_bound_texture_1D = _textureObject;
                glBindTexture(_textureType,_textureObject);
            }
            break;
        }
        case GL_TEXTURE_2D:{
            if(i.current_bound_texture_2D != _textureObject){
                i.current_bound_texture_2D = _textureObject;
                glBindTexture(_textureType,_textureObject);
            }
            break;
        }
        case GL_TEXTURE_3D:{
            if(i.current_bound_texture_3D != _textureObject){
                i.current_bound_texture_3D = _textureObject;
                glBindTexture(_textureType,_textureObject);
            }
            break;
        }
        case GL_TEXTURE_CUBE_MAP:{
            if(i.current_bound_texture_cube_map != _textureObject){
                i.current_bound_texture_cube_map = _textureObject;
                glBindTexture(_textureType,_textureObject);
            }
            break;
        }
    }
}
void Renderer::bindVAO(const GLuint _vaoObject){
    auto& i = *renderManager;
    if(i.current_bound_vao != _vaoObject){
        glBindVertexArray(_vaoObject);
        i.current_bound_vao = _vaoObject;
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
void Renderer::sendTexture(const char* location,Texture* texture,const uint slot){Renderer::sendTexture(location,texture->address(),slot,texture->type());}
void Renderer::sendTexture(const char* location,const GLuint textureAddress,const uint slot,const GLuint targetType){
    glActiveTexture(GL_TEXTURE0 + slot);
    bindTexture(targetType,textureAddress);
    sendUniform1i(location,slot);
}
void Renderer::sendTextureSafe(const char* location,Texture* texture,const uint slot){Renderer::sendTextureSafe(location,texture->address(),slot,texture->type());}
void Renderer::sendTextureSafe(const char* location,const GLuint textureAddress,const uint slot,const GLuint targetType){
    glActiveTexture(GL_TEXTURE0 + slot);
    bindTexture(targetType,textureAddress);
    sendUniform1iSafe(location,slot);
}
void Renderer::bindReadFBO(GLuint fbo){ renderManager->_bindReadFBO(fbo); }
void Renderer::bindFBO(epriv::FramebufferObject* fbo){ Renderer::bindFBO(fbo->address()); }
void Renderer::bindRBO(epriv::RenderbufferObject* rbo){ Renderer::bindRBO(rbo->address()); }
void Renderer::bindDrawFBO(GLuint fbo){ renderManager->_bindDrawFBO(fbo); }
void Renderer::bindFBO(GLuint fbo){Renderer::bindReadFBO(fbo);Renderer::bindDrawFBO(fbo);}
void Renderer::bindRBO(GLuint rbo){ renderManager->_bindRBO(rbo); }
void Renderer::unbindFBO(){ Renderer::bindFBO(GLuint(0)); }
void Renderer::unbindRBO(){ Renderer::bindRBO(GLuint(0)); }
void Renderer::unbindReadFBO(){ Renderer::bindReadFBO(0); }
void Renderer::unbindDrawFBO(){ Renderer::bindDrawFBO(0); }
void Renderer::renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth){
    renderManager->m_TexturesToBeRendered.emplace_back(nullptr,pos,col,glm::vec2(w,h),angle,depth);
}
void Renderer::renderTexture(Texture* texture,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth){
    texture->render(pos,col,angle,scl,depth);
}
void Renderer::renderText(string& text,Font* font, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth){
    font->renderText(text,pos,color,angle,scl,depth);
}
void Renderer::renderFullscreenQuad(uint w, uint h, uint startX, uint startY){ renderManager->_renderFullscreenQuad(w,h,startX,startY); }
void Renderer::renderFullscreenTriangle(uint w,uint h, uint startX, uint startY){ renderManager->_renderFullscreenTriangle(w,h,startX,startY); }
inline const GLint Renderer::getUniformLoc(const char* location){
    auto& m = renderManager->current_shader_program->uniforms(); if(!m.count(location)) return -1; return m.at(location);
}
inline const GLint& Renderer::getUniformLocUnsafe(const char* location){
    return renderManager->current_shader_program->uniforms().at(location); 
}
