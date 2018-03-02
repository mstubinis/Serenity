#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_Renderer.h"
#include "Engine_Window.h"
#include "Engine_FullscreenItems.h"
#include "Engine_BuiltInShaders.h"
#include "Engine_BuiltInResources.h"
#include "GBuffer.h"
#include "Camera.h"
#include "Light.h"
#include "Font.h"
#include "Scene.h"
#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Material.h"
#include "Object.h"
#include "ObjectDisplay.h"
#include "ObjectDynamic.h"
#include "FramebufferObject.h"
#include "SMAA_LUT.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <random>
#include <math.h>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

Mesh* epriv::InternalMeshes::PointLightBounds = nullptr;
Mesh* epriv::InternalMeshes::RodLightBounds = nullptr;
Mesh* epriv::InternalMeshes::SpotLightBounds = nullptr;

ShaderP* epriv::InternalShaderPrograms::Deferred = nullptr;


namespace Engine{
	namespace epriv{
		struct EngineInternalShaders final{
			enum Shader{
				FullscreenVertex,
				FXAAFrag,
				VertexBasic,
				VertexHUD,
				VertexSkybox,
				DeferredFrag,
				DeferredFragHUD,
				DeferredFragSkybox,
				DeferredFragSkyboxFake,
				CopyDepthFrag,
				SSAOFrag,
				HDRFrag,
				BlurFrag,
				GodRaysFrag,
				FinalFrag,
				LightingFrag,
				LightingGIFrag,
				CubemapConvoludeFrag,
				CubemapPrefilterEnvFrag,
				BRDFPrecomputeFrag,
				GrayscaleFrag,
				EdgeCannyBlurFrag,
				EdgeCannyFrag,
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

				_TOTAL
			};
		};
		struct EngineInternalShaderPrograms final{
			enum Program{
				//Deferred, //using the internal resource static one instead
				DeferredHUD,
				DeferredGodRays,
				DeferredBlur,
				DeferredHDR,
				DeferredSSAO,
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
				EdgeCannyBlur,
				EdgeCannyFrag,
				StencilPass,
				SMAA1Stencil,
				SMAA1,
				SMAA2,
				SMAA3,
				SMAA4,

				_TOTAL
			};
		};




		struct TextureRenderInfo final{
			Texture* texture;
			glm::vec2 pos, scl;
			glm::vec4 col;
			float rot, depth;
			TextureRenderInfo(){
				texture = nullptr; pos = scl = glm::vec2(0); col = glm::vec4(1); rot = depth = 0;
			}
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
			FontRenderInfo(){
				font = nullptr; pos = scl = glm::vec2(0); col = glm::vec4(1); rot = depth = 0;
				text = "";
			}
			FontRenderInfo(Font* _font, std::string _text, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth){
				font = _font; pos = _pos; col = _col; scl = _scl; rot = _rot; depth = _depth;
				text = _text;
			}
		};
	};
};

class epriv::RenderManager::impl final{
    public:
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
		bool bloom;
		float bloom_radius;
		float bloom_strength;
        #pragma endregion

		#pragma region LightingInfo
		bool lighting;
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
		#pragma endregion

		#pragma region SSAOInfo
        static const int SSAO_KERNEL_COUNT = 32;
        static const int SSAO_NORMALMAP_SIZE = 128;
		bool ssao;
		bool ssao_do_blur;
		uint ssao_samples;
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
		float gamma;
		Texture* brdfCook;
		ShaderP* current_shader_program;
		Material* current_bound_material;
		unsigned char cull_face_status;
		GLuint current_bound_read_fbo;
		GLuint current_bound_draw_fbo;
		GLuint current_bound_rbo;
		AntiAliasingAlgorithm::Algorithm aa_algorithm;
		glm::uvec4 gl_viewport_data;
		bool draw_physics_debug;

		GBuffer* m_gBuffer;
		glm::mat4 m_2DProjectionMatrix;
		vector<FontRenderInfo> m_FontsToBeRendered;
		vector<TextureRenderInfo> m_TexturesToBeRendered;
		vector<ShaderP*> m_GeometryPassShaderPrograms;
		vector<ShaderP*> m_ForwardPassShaderPrograms;
		glm::mat4 m_IdentityMat4;
		glm::mat3 m_IdentityMat3;
		FullscreenQuad* m_FullscreenQuad;
		FullscreenTriangle* m_FullscreenTriangle;
		#pragma endregion

		#pragma region EngineInternalShadersAndPrograms
		vector<Shader*> m_InternalShaders;
		vector<ShaderP*> m_InternalShaderPrograms;

		#pragma endregion

		void _init(const char* name,uint& w,uint& h){
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
			bloom = true;
			bloom_radius = 0.84f;
			bloom_strength = 2.5f;
			#pragma endregion

			#pragma region LightingInfo
			lighting = true;
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
			#pragma endregion

			#pragma region SSAOInfo
			ssao = true;
			ssao_do_blur = true;
			ssao_samples = 8;
			ssao_blur_strength = 0.5f;
			ssao_scale = 1.0f;
			ssao_intensity = 1.1f;
			ssao_bias = 0.495f;
			ssao_radius = 0.5f;
			#pragma endregion

			#pragma region HDRInfo
			hdr = true;
			hdr_exposure = 3.0f;
			hdr_algorithm = HDRAlgorithm::UNCHARTED;
			#pragma endregion

			#pragma region GeneralInfo
			gamma = 2.2f;
			brdfCook = nullptr;
			current_shader_program = nullptr;
			current_bound_material = nullptr;
			cull_face_status = 0; /* 0 = back | 1 = front | 2 = front and back */
			current_bound_read_fbo = 0;
			current_bound_draw_fbo = 0;
			current_bound_rbo = 0;
			aa_algorithm = AntiAliasingAlgorithm::FXAA;
			gl_viewport_data = glm::uvec4(0,0,0,0);
			#ifdef _DEBUG
			    draw_physics_debug = true;
			#else
			    draw_physics_debug = false;
			#endif

			m_gBuffer = nullptr;
			m_2DProjectionMatrix = glm::ortho(0.0f,float(w),0.0f,float(h),0.005f,1000.0f);
			m_IdentityMat4 = glm::mat4(1.0f);
			m_IdentityMat3 = glm::mat3(1.0f);
			#pragma endregion
		}
		void _postInit(const char* name,uint& width,uint& height){
			Engine::Shaders::Detail::ShadersManagement::init();

			m_InternalShaders.resize(EngineInternalShaders::_TOTAL,nullptr);
			m_InternalShaderPrograms.resize(EngineInternalShaderPrograms::_TOTAL,nullptr);

			#pragma region EngineInternalShadersAndPrograms
			m_InternalShaders.at(EngineInternalShaders::FullscreenVertex) = new Shader("vert_fullscreenQuad",Shaders::Detail::ShadersManagement::fullscreen_quad_vertex,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::FXAAFrag) = new Shader("frag_fxaa",Shaders::Detail::ShadersManagement::fxaa_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::VertexBasic) = new Shader("vert_basic",Shaders::Detail::ShadersManagement::vertex_basic,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::VertexHUD) = new Shader("vert_hud",Shaders::Detail::ShadersManagement::vertex_hud,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::VertexSkybox) = new Shader("vert_skybox",Shaders::Detail::ShadersManagement::vertex_skybox,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::DeferredFrag) = new Shader("deferred_frag",Shaders::Detail::ShadersManagement::deferred_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::DeferredFragHUD) = new Shader("deferred_frag_hud",Shaders::Detail::ShadersManagement::deferred_frag_hud,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::DeferredFragSkybox) = new Shader("deferred_frag_skybox",Shaders::Detail::ShadersManagement::deferred_frag_skybox,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::DeferredFragSkyboxFake) = new Shader("deferred_frag_skybox_fake",Shaders::Detail::ShadersManagement::deferred_frag_skybox_fake,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::CopyDepthFrag) = new Shader("copy_depth_frag",Shaders::Detail::ShadersManagement::copy_depth_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SSAOFrag) = new Shader("ssao_frag",Shaders::Detail::ShadersManagement::ssao_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::HDRFrag) = new Shader("hdr_frag",Shaders::Detail::ShadersManagement::hdr_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::BlurFrag) = new Shader("blur_frag",Shaders::Detail::ShadersManagement::blur_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::GodRaysFrag) = new Shader("godrays_frag",Shaders::Detail::ShadersManagement::godRays_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::FinalFrag) = new Shader("final_frag",Shaders::Detail::ShadersManagement::final_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::LightingFrag) = new Shader("lighting_frag",Shaders::Detail::ShadersManagement::lighting_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::LightingGIFrag) = new Shader("lighting_frag_gi",Shaders::Detail::ShadersManagement::lighting_frag_gi,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::CubemapConvoludeFrag) = new Shader("cubemap_convolude_frag",Shaders::Detail::ShadersManagement::cubemap_convolude_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::CubemapPrefilterEnvFrag) = new Shader("cubemap_prefilterEnv_frag",Shaders::Detail::ShadersManagement::cubemap_prefilter_envmap_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::BRDFPrecomputeFrag) = new Shader("brdf_precompute_frag",Shaders::Detail::ShadersManagement::brdf_precompute,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::GrayscaleFrag) = new Shader("greyscale_frag",Shaders::Detail::ShadersManagement::greyscale_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::EdgeCannyBlurFrag) = new Shader("edge_canny_blur",Shaders::Detail::ShadersManagement::edge_canny_blur,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::EdgeCannyFrag) = new Shader("edge_canny_frag",Shaders::Detail::ShadersManagement::edge_canny_frag,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::StencilPassFrag) = new Shader("stencil_pass",Shaders::Detail::ShadersManagement::stencil_passover,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAVertex1) = new Shader("smaa_vert_1",Shaders::Detail::ShadersManagement::smaa_vertex_1,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAVertex2) = new Shader("smaa_vert_2",Shaders::Detail::ShadersManagement::smaa_vertex_2,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAVertex3) = new Shader("smaa_vert_3",Shaders::Detail::ShadersManagement::smaa_vertex_3,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAVertex4) = new Shader("smaa_vert_4",Shaders::Detail::ShadersManagement::smaa_vertex_4,ShaderType::Vertex,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAFrag1Stencil) = new Shader("smaa_frag_1_stencil",Shaders::Detail::ShadersManagement::smaa_frag_1_stencil,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAFrag1) = new Shader("smaa_frag_1",Shaders::Detail::ShadersManagement::smaa_frag_1,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAFrag2) = new Shader("smaa_frag_2",Shaders::Detail::ShadersManagement::smaa_frag_2,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAFrag3) = new Shader("smaa_frag_3",Shaders::Detail::ShadersManagement::smaa_frag_3,ShaderType::Fragment,false);
			m_InternalShaders.at(EngineInternalShaders::SMAAFrag4) = new Shader("smaa_frag_4",Shaders::Detail::ShadersManagement::smaa_frag_4,ShaderType::Fragment,false);

			epriv::InternalShaderPrograms::Deferred = new ShaderP("Deferred",m_InternalShaders.at(EngineInternalShaders::VertexBasic),m_InternalShaders.at(EngineInternalShaders::DeferredFrag),ShaderRenderPass::Geometry);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD) = new ShaderP("Deferred_HUD",m_InternalShaders.at(EngineInternalShaders::VertexHUD),m_InternalShaders.at(EngineInternalShaders::DeferredFragHUD),ShaderRenderPass::Geometry);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredGodRays) = new ShaderP("Deferred_GodsRays",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::GodRaysFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlur) = new ShaderP("Deferred_Blur",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::BlurFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHDR) = new ShaderP("Deferred_HDR",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::HDRFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSSAO) = new ShaderP("Deferred_SSAO",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::SSAOFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFinal) = new ShaderP("Deferred_Final",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::FinalFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFXAA) = new ShaderP("Deferred_FXAA",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::FXAAFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox) = new ShaderP("Deferred_Skybox",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::DeferredFragSkybox),ShaderRenderPass::Geometry);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkyboxFake) = new ShaderP("Deferred_Skybox_Fake",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::DeferredFragSkyboxFake),ShaderRenderPass::Geometry);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CopyDepth) = new ShaderP("Copy_Depth",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::CopyDepthFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLighting) = new ShaderP("Deferred_Light",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::LightingFrag),ShaderRenderPass::Lighting);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLightingGI) = new ShaderP("Deferred_Light_GI",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::LightingGIFrag),ShaderRenderPass::Lighting);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude) = new ShaderP("Cubemap_Convolude",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::CubemapConvoludeFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv) = new ShaderP("Cubemap_Prefilter_Env",m_InternalShaders.at(EngineInternalShaders::VertexSkybox),m_InternalShaders.at(EngineInternalShaders::CubemapPrefilterEnvFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance) = new ShaderP("BRDF_Precompute_CookTorrance",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::BRDFPrecomputeFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::Grayscale) = new ShaderP("Greyscale_Frag",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::GrayscaleFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur) = new ShaderP("Deferred_Edge_Canny_Blur",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::EdgeCannyBlurFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyFrag) = new ShaderP("Deferred_Edge_Canny",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::EdgeCannyFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::StencilPass) = new ShaderP("Stencil_Pass",m_InternalShaders.at(EngineInternalShaders::FullscreenVertex),m_InternalShaders.at(EngineInternalShaders::StencilPassFrag),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1Stencil) = new ShaderP("Deferred_SMAA_1_Stencil",m_InternalShaders.at(EngineInternalShaders::SMAAVertex1),m_InternalShaders.at(EngineInternalShaders::SMAAFrag1Stencil),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1) = new ShaderP("Deferred_SMAA_1",m_InternalShaders.at(EngineInternalShaders::SMAAVertex1),m_InternalShaders.at(EngineInternalShaders::SMAAFrag1),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA2) = new ShaderP("Deferred_SMAA_2",m_InternalShaders.at(EngineInternalShaders::SMAAVertex2),m_InternalShaders.at(EngineInternalShaders::SMAAFrag2),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA3) = new ShaderP("Deferred_SMAA_3",m_InternalShaders.at(EngineInternalShaders::SMAAVertex3),m_InternalShaders.at(EngineInternalShaders::SMAAFrag3),ShaderRenderPass::Postprocess);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA4) = new ShaderP("Deferred_SMAA_4",m_InternalShaders.at(EngineInternalShaders::SMAAVertex4),m_InternalShaders.at(EngineInternalShaders::SMAAFrag4),ShaderRenderPass::Postprocess);
			
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

			epriv::InternalMeshes::PointLightBounds = new Mesh(pointLightMesh,CollisionType::None,false,0.0005f);
			epriv::InternalMeshes::RodLightBounds = new Mesh(rodLightData,CollisionType::None,false,0.0005f);
			epriv::InternalMeshes::SpotLightBounds = new Mesh(spotLightData,CollisionType::None,false,0.0005f);

			Mesh::Plane = new Mesh("Plane",1.0f,1.0f,0.0005f);
			Mesh::Cube = new Mesh(cubeMesh,CollisionType::None,false,0.0005f);

			brdfCook = new Texture("BRDFCookTorrance",512,512,ImageInternalFormat::RG16F,ImagePixelFormat::RG,ImagePixelType::FLOAT,GL_TEXTURE_2D,1.0f);
			brdfCook->setWrapping(TextureWrap::ClampToEdge);	

            #pragma endregion

			m_FullscreenQuad = new FullscreenQuad();
	        m_FullscreenTriangle = new FullscreenTriangle();

			uniform_real_distribution<float> randFloats(0.0f,1.0f);
			default_random_engine gen;
			vector<glm::vec3> kernels;
			for(uint i = 0; i < SSAO_KERNEL_COUNT; ++i){
				glm::vec3 sample(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,randFloats(gen));
				sample = glm::normalize(sample);
				sample *= randFloats(gen);
				float scale = float(i) / float(SSAO_KERNEL_COUNT);
				float a = 0.1f; float b = 1.0f; float f = scale * scale;
				scale = a + f * (b - a); //basic lerp   
				sample *= scale;
				kernels.push_back(sample);
			}
			copy(kernels.begin(),kernels.end(),ssao_Kernels);
			vector<glm::vec3> ssaoNoise;
			for(uint i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i){
				glm::vec3 noise(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,0.0f); 
				ssaoNoise.push_back(noise);
			}
			glGenTextures(1, &ssao_noise_texture);
			glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE,SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
			GLEnable(GLState::DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1); //for non Power of Two textures
    
			//GLEnable(GLState::TEXTURE_CUBE_MAP_SEAMLESS); //used for IBL. but its not implemented on my GPU :(

			glGenTextures(1,&SMAA_AreaTexture);
			glBindTexture(GL_TEXTURE_2D,SMAA_AreaTexture);
			Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
			Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RG8,160,560,0,GL_RG,GL_UNSIGNED_BYTE,areaTexBytes);
			glBindTexture(GL_TEXTURE_2D,0);

			glGenTextures(1,&SMAA_SearchTexture);
			glBindTexture(GL_TEXTURE_2D,SMAA_SearchTexture);
			Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
			Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
			glTexImage2D(GL_TEXTURE_2D,0,GL_R8,64,16,0,GL_RED,GL_UNSIGNED_BYTE,searchTexBytes);
			glBindTexture(GL_TEXTURE_2D,0);

			glClearStencil(0);
			GLDisable(GLState::STENCIL_TEST);

			_generateBRDFLUTCookTorrance(512);
		}
		void _destruct(){
			SAFE_DELETE(m_gBuffer);
			SAFE_DELETE(m_FullscreenQuad);
			SAFE_DELETE(m_FullscreenTriangle);

			SAFE_DELETE(epriv::InternalMeshes::PointLightBounds);
			SAFE_DELETE(epriv::InternalMeshes::RodLightBounds);
			SAFE_DELETE(epriv::InternalMeshes::SpotLightBounds);
			SAFE_DELETE(Mesh::Plane);
			SAFE_DELETE(Mesh::Cube);
			SAFE_DELETE(epriv::InternalShaderPrograms::Deferred);

			for(auto program:m_InternalShaderPrograms) SAFE_DELETE(program);
			for(auto shader:m_InternalShaders) SAFE_DELETE(shader);

			glDeleteTextures(1,&ssao_noise_texture);
			glDeleteTextures(1,&SMAA_SearchTexture);
			glDeleteTextures(1,&SMAA_AreaTexture);
		}
		void _renderSkybox(SkyboxEmpty* s){
			Scene* scene = Resources::getCurrentScene();
			Camera* c = scene->getActiveCamera();
			glm::mat4 view = c->getView();
			Math::removeMatrixPosition(view);
			if(s != nullptr){
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->bind();
				Renderer::sendUniformMatrix4f("VP",c->getProjection() * view);
				Renderer::bindTexture("Texture",s->texture()->address(0),0,GL_TEXTURE_CUBE_MAP);
				Skybox::bindMesh();
				Renderer::unbindTextureCubemap(0);//yes, this is needed.
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->unbind();
			}
			else{//render a fake skybox.
				Skybox::initMesh();
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkyboxFake)->bind();
				glm::vec3 bgColor = scene->getBackgroundColor();
				Renderer::sendUniformMatrix4f("VP",c->getProjection() * view);
				Renderer::sendUniform4f("Color",bgColor.r,bgColor.g,bgColor.b,1.0f);
				Skybox::bindMesh();
				Renderer::unbindTextureCubemap(0);//yes, this is needed.
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkyboxFake)->unbind();
			}
		}
		void _resize(uint& w, uint& h){
			m_2DProjectionMatrix = glm::ortho(0.0f,(float)w,0.0f,(float)h,0.005f,1000.0f);
			m_gBuffer->resize(w,h);
		}
		void _onFullscreen(sf::Window* sfWindow,sf::VideoMode& videoMode,const char* winName,uint& style,sf::ContextSettings& settings){
            SAFE_DELETE(m_gBuffer);
            sfWindow->create(videoMode,winName,style,settings);

            m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);

			//oh yea the opengl context is lost, gotta restore the state machine
			Renderer::RestoreGLState();
			glClearStencil(0);
			glDepthFunc(GL_LEQUAL);
		}
		void _onOpenGLContextCreation(uint& width,uint& height){
            glewExperimental = GL_TRUE;
            glewInit(); glGetError();//stupid glew always inits an error. nothing we can do about it.
			Renderer::GLEnable(GLState::TEXTURE_2D); //is this really needed?
			Renderer::GLEnable(GLState::CULL_FACE);
            Renderer::Settings::cullFace(GL_BACK);
            SAFE_DELETE(m_gBuffer);
            m_gBuffer = new GBuffer(width,height);
		}
		void _generatePBREnvMapData(Texture* texture,uint& convoludeTextureSize, uint& preEnvFilterSize){
			uint texType = texture->type();
			if(texType != GL_TEXTURE_CUBE_MAP){
				cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
			}
			uint size = convoludeTextureSize;
			glBindTexture(texType, texture->address(1));
			Renderer::unbindFBO();
			epriv::FramebufferObject* fbo = new epriv::FramebufferObject(texture->name() + "_fbo_envData",size,size,ImageInternalFormat::Depth16);
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

			Renderer::bindTexture("cubemap",texture->address(),0,texType);
			Renderer::setViewport(0,0,size,size);
			for (uint i = 0; i < 6; ++i){
				glm::mat4 vp = captureProjection*captureViews[i];
				Renderer::sendUniformMatrix4f("VP",vp);
				glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture->address(1),0);
				Renderer::Settings::clear(true,true,false);
				Skybox::bindMesh();
			}
			cout << "---- " + texture->name() + " (Cubemap): convolution done ----" << endl;
			Resources::getWindow()->display(); //prevent opengl & windows timeout
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude)->unbind();

			//now gen EnvPrefilterMap for specular IBL
			size = preEnvFilterSize;
			glBindTexture(texType, texture->address(2));
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv)->bind();
			Renderer::bindTexture("cubemap",texture->address(),0,texType);
			Renderer::sendUniform1f("PiFourDividedByResSquaredTimesSix",12.56637f / float((texture->width() * texture->width())*6));
			Renderer::sendUniform1i("NUM_SAMPLES",32);
			uint maxMipLevels = 5;
			for (uint m = 0; m < maxMipLevels; ++m){
				uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
				fbo->resize(mipSize,mipSize);
				float roughness = (float)m/(float)(maxMipLevels-1);
				Renderer::sendUniform1f("roughness",roughness);
				float a = roughness * roughness;
				Renderer::sendUniform1f("a2",a*a);
				for (uint i = 0; i < 6; ++i){
					glm::mat4 vp = captureProjection * captureViews[i];
					Renderer::sendUniformMatrix4f("VP", vp);
					glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,texture->address(2),m);
					Renderer::Settings::clear(true,true,false);
					Skybox::bindMesh();
				}
			}
			cout << "---- " + texture->name() + " (Cubemap): prefilter done ----" << endl;
			Resources::getWindow()->display(); //prevent opengl & windows timeout

			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv)->unbind();
			fbo->unbind();
			delete fbo;
		}
		void _generateBRDFLUTCookTorrance(uint brdfSize){
			uint& prevReadBuffer = current_bound_read_fbo;
			uint& prevDrawBuffer = current_bound_draw_fbo;

			FramebufferObject* fbo = new FramebufferObject("BRDFLUT_Gen_CookTorr_FBO",brdfSize,brdfSize,ImageInternalFormat::Depth16);
			fbo->bind();

			glBindTexture(GL_TEXTURE_2D, brdfCook->address());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
			Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
			Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToEdge);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,brdfCook->address(), 0);

			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance)->bind();
			Renderer::sendUniform1i("NUM_SAMPLES",256);
			Renderer::Settings::clear(true,true,false);
			glColorMask(GL_TRUE,GL_TRUE,GL_FALSE,GL_FALSE);
			_renderFullscreenTriangle(brdfSize,brdfSize);
			cout << "----  BRDF LUT (Cook Torrance) completed ----" << endl;
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::BRDFPrecomputeCookTorrance)->unbind();
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

			delete fbo;
			Renderer::bindReadFBO(prevReadBuffer);
			Renderer::bindDrawFBO(prevDrawBuffer);
		}
		void _setSMAAQuality(SMAAQualityLevel::Level& l){
			if(l == SMAAQualityLevel::Low){
				SMAA_THRESHOLD = 0.15f;           SMAA_MAX_SEARCH_STEPS = 4;
				SMAA_MAX_SEARCH_STEPS_DIAG = 0;   SMAA_CORNER_ROUNDING = 0;
			}
			else if(l == SMAAQualityLevel::Medium){
				SMAA_THRESHOLD = 0.1f;            SMAA_MAX_SEARCH_STEPS = 8;
				SMAA_MAX_SEARCH_STEPS_DIAG = 0;   SMAA_CORNER_ROUNDING = 0;
			}
			else if(l == SMAAQualityLevel::High){
				SMAA_THRESHOLD = 0.1f;            SMAA_MAX_SEARCH_STEPS = 16;
				SMAA_MAX_SEARCH_STEPS_DIAG = 8;   SMAA_CORNER_ROUNDING = 25;
			}
			else if(l == SMAAQualityLevel::Ultra){
				SMAA_THRESHOLD = 0.05f;           SMAA_MAX_SEARCH_STEPS = 32;
				SMAA_MAX_SEARCH_STEPS_DIAG = 16;  SMAA_CORNER_ROUNDING = 25;
			}
		}
		void _setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm& algorithm){
			if(aa_algorithm != algorithm){ aa_algorithm = algorithm; }
		}
		void _cullFace(uint& s){
			//0 = back | 1 = front | 2 = front and back
			if(s == GL_BACK && cull_face_status != 0){
				glCullFace(GL_BACK);
				cull_face_status = 0;
			}
			else if(s == GL_FRONT && cull_face_status != 1){
				glCullFace(GL_FRONT);
				cull_face_status = 1;
			}
			else if(s == GL_FRONT_AND_BACK && cull_face_status != 2){
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
		void _renderTextures(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->bind();
			Mesh::Plane->bind();
			for(auto item:m_TexturesToBeRendered){
				if(item.texture != nullptr){
					bindTexture("DiffuseTexture",item.texture,0);
					sendUniform1i("DiffuseTextureEnabled",1);
				}
				else{
					bindTexture("DiffuseTexture",0,0);
					sendUniform1i("DiffuseTextureEnabled",0);
				}
				sendUniform4f("Object_Color",item.col.r,item.col.g,item.col.b,item.col.a);

				glm::mat4 model = m_IdentityMat4;
				model = glm::translate(model, glm::vec3(item.pos.x,item.pos.y,-0.001f - item.depth));
				model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
				if(item.texture != nullptr)
					model = glm::scale(model, glm::vec3(item.texture->width(),item.texture->height(),1));
				model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

				sendUniformMatrix4f("VP",m_2DProjectionMatrix);
				sendUniformMatrix4f("Model",model);

				Mesh::Plane->render();
			}
			Mesh::Plane->unbind();
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->unbind();
		}
		void _renderText(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->bind();
			for(auto item:m_FontsToBeRendered){
				bindTexture("DiffuseTexture",item.font->getFontData()->getGlyphTexture(),0);
				sendUniform1i("DiffuseTextureEnabled",1);
				sendUniform4f("Object_Color",item.col.x,item.col.y,item.col.z,item.col.w);

				float y_offset = 0;
				float x = item.pos.x;
				for(auto c:item.text){
					if(c == '\n'){
						y_offset += (item.font->getFontData()->getGlyphData('X')->height+6) * item.scl.y;
						x = item.pos.x;
					}
					else{
						FontGlyph* chr = item.font->getFontData()->getGlyphData(c);

						chr->m_Model = m_IdentityMat4;
						chr->m_Model = glm::translate(chr->m_Model, glm::vec3(x + chr->xoffset ,item.pos.y - (chr->height + chr->yoffset) - y_offset,-0.001f - item.depth));
						chr->m_Model = glm::rotate(chr->m_Model, item.rot,glm::vec3(0,0,1));
						chr->m_Model = glm::scale(chr->m_Model, glm::vec3(item.scl.x,item.scl.y,1));

						sendUniformMatrix4f("VP",m_2DProjectionMatrix);
						sendUniformMatrix4f("Model",chr->m_Model);
						chr->char_mesh->bind();
						chr->char_mesh->render();
						chr->char_mesh->unbind();
						x += (chr->xadvance) * item.scl.x;
					}
				}
			}
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHUD)->unbind();
		}
		void _passGeometry(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,Object* ignore){
			Scene* scene = Resources::getCurrentScene();
			glm::vec3 clear = scene->getBackgroundColor();
			const float colors[4] = { clear.r,clear.g,clear.b,1.0f };  
	
			if(godRays)
				gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); 
			else
				gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA");

			Settings::clear(true,true,true);
			glDepthFunc(GL_LEQUAL);
			GLDisable(GLState::BLEND);//disable blending on all mrts

			glClearBufferfv(GL_COLOR,0,colors);
			if(godRays){
				const float godRays[4] = { 0.03f,0.023f,0.032f,1.0f };
				glClearBufferfv(GL_COLOR,3,godRays);
			}
			glEnablei(GL_BLEND,0); //enable blending on diffuse mrt only
			glBlendEquationi(GL_FUNC_ADD,0);
			glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

			//TODO: move skybox rendering to the last after moving planetary atmosphere to forward rendering pass
			gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA");
			_renderSkybox(scene->skybox());


			if(godRays)
				gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); 

			//RENDER BACKGROUND OBJECTS THAT ARE IN FRONT OF SKYBOX HERE

			GLEnable(GLState::DEPTH_TEST);
			GLEnable(GLState::DEPTH_MASK);

			//RENDER NORMAL OBJECTS HERE
			for(auto shader:m_GeometryPassShaderPrograms){
				vector<Material*>& shaderMaterials = shader->getMaterials(); 
				if(shaderMaterials.size() > 0){
					shader->bind();
					for(auto material:shaderMaterials){
						vector<MaterialMeshEntry*>& materialMeshes = material->getMeshEntries(); 
						if(materialMeshes.size() > 0){
							material->bind();
							for(auto materialMeshEntry:materialMeshes){
								MaterialMeshEntry* entry = materialMeshEntry;
								Mesh* entryMesh = entry->mesh();
								entryMesh->bind();
								for(auto meshInstance:materialMeshEntry->meshInstances()){
									boost::weak_ptr<Object> o = Resources::getObjectPtr(meshInstance.first);
									Object* object = o.lock().get();
									if(exists(o) && scene->objects().count(object->name()) && (object != ignore)){
										if(object->passedRenderCheck()){ //culling check
											object->bind();
											for(auto meshInstance:meshInstance.second){
												meshInstance->bind(); //render also
												meshInstance->unbind();
											}
											object->unbind();
										}
									}
									//protect against any custom changes by restoring to the regular shader and material
									if(current_shader_program != shader){
										shader->bind();
										material->bind();
									}
								}
								entryMesh->unbind();
							}
							material->unbind();
						}
					}
					shader->unbind();
				}
			}
			//TODO: move skybox rendering here after moving planetary atmosphere to forward rendering pass

			GLDisable(GLState::DEPTH_TEST);
			GLDisable(GLState::DEPTH_MASK);

			//RENDER FOREGROUND OBJECTS HERE
		}
		void _passForwardRendering(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,Object* ignore){
			Scene* scene = Resources::getCurrentScene();

			for(auto shaderProgram:m_ForwardPassShaderPrograms){
				vector<Material*>& shaderMaterials = shaderProgram->getMaterials(); if(shaderMaterials.size() > 0){
				shaderProgram->bind();   
				for(auto material:shaderMaterials){
					vector<MaterialMeshEntry*>& materialMeshes = material->getMeshEntries(); if(materialMeshes.size() > 0){
					material->bind();
					for(auto meshEntry:materialMeshes){
						meshEntry->mesh()->bind();
						for(auto instance:meshEntry->meshInstances()){
							boost::weak_ptr<Object> o = Resources::getObjectPtr(instance.first);
							Object* object = o.lock().get();
							if(exists(o) && scene->objects().count(object->name()) && (object != ignore)){
								if(object->passedRenderCheck()){ //culling check
									object->bind();
									for(auto meshInstance:instance.second){
										meshInstance->bind(); //render also
										meshInstance->unbind();
									}
									object->unbind();
								}
							}
							//protect against any custom changes by restoring to the regular shader and material
							if(current_shader_program != shaderProgram){
								shaderProgram->bind();
								material->bind();
							}
						}
						meshEntry->mesh()->unbind();
					}
					material->unbind();}
				}
				shaderProgram->unbind();}
			}
		}
		void _passCopyDepth(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CopyDepth)->bind();

			bindTexture("gDepthMap",gbuffer->getTexture(GBufferType::Depth),0);

			_renderFullscreenTriangle(fbufferWidth,fbufferHeight);

			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CopyDepth)->unbind();
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		}
		void _passLighting(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,bool mainRenderFunc){
			Scene* s = Resources::getCurrentScene();
    
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLighting)->bind();

			glm::mat4 invVP = c->getViewProjInverted();
			glm::mat4 invP = glm::inverse(c->getProjection());
			glm::vec3 campos = c->getPosition();
			float cNear = c->getNear(); float cFar = c->getFar();
    
			sendUniformMatrix4fSafe("VP",c->getViewProjection());
			sendUniformMatrix4fSafe("invVP",invVP);
			sendUniformMatrix4fSafe("invP",invP);
			sendUniform4fSafe("CamPosGamma",campos.x, campos.y, campos.z,gamma);

			sendUniform4fvSafe("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());

			sendUniform4fSafe("ScreenData",cNear,cFar,(float)fbufferWidth,(float)fbufferHeight);

			bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0);
			bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),1);
			bindTextureSafe("gMiscMap",gbuffer->getTexture(GBufferType::Misc),2);
			bindTextureSafe("gDepthMap",gbuffer->getTexture(GBufferType::Depth),3);

			for (auto light:s->lights()){
				light.second->lighten();
			}
			for(uint i = 0; i < 4; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLighting)->unbind();
			if(mainRenderFunc){
				//do GI here. (only doing GI during the main render pass, not during light probes
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLightingGI)->bind();

				sendUniformMatrix4fSafe("invVP",invVP);
				sendUniformMatrix4fSafe("invP",invP);
				sendUniform4fvSafe("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
				sendUniform4fSafe("CamPosGamma",campos.x, campos.y, campos.z,gamma);
				sendUniform4fSafe("ScreenData",cNear,cFar,float(fbufferWidth),float(fbufferHeight));
				bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0);
				bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),1);
				bindTextureSafe("gDepthMap",gbuffer->getTexture(GBufferType::Depth),2);

				SkyboxEmpty* skybox = s->skybox();

				if(s->lightProbes().size() > 0){
					for(auto probe:s->lightProbes()){
						LightProbe* p = probe.second;
						bindTextureSafe("irradianceMap",p->getIrriadianceMap(),3,GL_TEXTURE_CUBE_MAP);
						bindTextureSafe("prefilterMap",p->getPrefilterMap(),4,GL_TEXTURE_CUBE_MAP);
						bindTextureSafe("brdfLUT",brdfCook,5);
						break;
					}
				}
				else{
					if(skybox != nullptr && skybox->texture()->numAddresses() >= 3){
						bindTextureSafe("irradianceMap",skybox->texture()->address(1),3,GL_TEXTURE_CUBE_MAP);
						bindTextureSafe("prefilterMap",skybox->texture()->address(2),4,GL_TEXTURE_CUBE_MAP);
						bindTextureSafe("brdfLUT",brdfCook,5);
					}
				}

				_renderFullscreenTriangle(fbufferWidth,fbufferHeight);
				for(uint i = 0; i < 3; ++i){ unbindTexture2D(i); }
				unbindTextureCubemap(3);
				unbindTextureCubemap(4);
				unbindTexture2D(5);
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredLightingGI)->unbind();
			}
			GLDisable(GLState::STENCIL_TEST);
		}
		void _passSSAO(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSSAO)->bind();

			sendUniform1iSafe("doSSAO",int(ssao));
			sendUniform1iSafe("doBloom",int(bloom));

			glm::vec3 camPos = c->getPosition();
			sendUniformMatrix4fSafe("View",c->getView());
			sendUniformMatrix4fSafe("Projection",c->getProjection());
			sendUniformMatrix4fSafe("invVP",c->getViewProjInverted());
			sendUniformMatrix4fSafe("invP",glm::inverse(c->getProjection()));
			sendUniform1fSafe("nearz",c->getNear());
			sendUniform1fSafe("farz",c->getFar());

			sendUniform3fSafe("CameraPosition",camPos.x,camPos.y,camPos.z);
    
			sendUniform4fSafe("SSAOInfo",ssao_radius,ssao_intensity,ssao_bias,ssao_scale);
    
			sendUniform1iSafe("Samples",ssao_samples);
			sendUniform1iSafe("NoiseTextureSize",SSAO_NORMALMAP_SIZE);
    
			float _divisor = gbuffer->getBuffer(GBufferType::Bloom)->divisor();
			sendUniform1fSafe("fbufferDivisor",_divisor);
    
			sendUniform3fvSafe("poisson[0]",ssao_Kernels,SSAO_KERNEL_COUNT);

			bindTexture("gNormalMap",gbuffer->getTexture(GBufferType::Normal),0);
			bindTexture("gRandomMap",ssao_noise_texture,1,GL_TEXTURE_2D);
			bindTexture("gMiscMap",gbuffer->getTexture(GBufferType::Misc),2);
			bindTexture("gLightMap",gbuffer->getTexture(GBufferType::Lighting),3);
			bindTexture("gDepthMap",gbuffer->getTexture(GBufferType::Depth),4);

			_renderFullscreenTriangle(fboWidth,fboHeight);

			for(uint i = 0; i < 5; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSSAO)->unbind();
		}
		void _passStencil(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::StencilPass)->bind();

			gbuffer->getMainFBO()->bind();

			Settings::clear(false,false,true); //stencil is completely filled with 0's
			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
			GLEnable(GLState::STENCIL_TEST);

			bindTexture("gNormalMap",gbuffer->getTexture(GBufferType::Normal),0);
			_renderFullscreenTriangle(fbufferWidth,fbufferHeight);

			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_EQUAL, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);//Do not change stencil

			for(uint i = 0; i < 1; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::StencilPass)->unbind();
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

		}
		void _passEdgeCanny(GBuffer* gbuffer,Camera* c,uint& fboWidth,uint& fboHeight,GLuint texture){
    
			//texture is the lighting buffer which is the final pass results
    
			gbuffer->start(GBufferType::Misc);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyFrag)->bind();
			bindTexture("textureMap",gbuffer->getTexture(texture),0);
			_renderFullscreenTriangle(fboWidth,fboHeight);
			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyFrag)->unbind();
    
			//misc is now greyscale scene. lighting is still final scene

			gbuffer->start(GBufferType::Diffuse);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->bind();
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Misc),0);
			_renderFullscreenTriangle(fboWidth,fboHeight);
			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->unbind();
			/*
			//blur it again
			gbuffer->start(epriv::GBufferType::Misc);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->bind();
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Diffuse),0);
			renderFullscreenTriangle(fboWidth,fboHeight);
			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->unbind();
			//blur it again
			gbuffer->start(GBufferType::Diffuse);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->bind();
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Misc),0);
			renderFullscreenTriangle(fboWidth,fboHeight);
			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyBlur)->unbind();
			*/

			//misc is now the final blurred greyscale image
			gbuffer->start(GBufferType::Misc);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyFrag)->bind();
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Diffuse),0);
			_renderFullscreenTriangle(fboWidth,fboHeight);
			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::EdgeCannyFrag)->unbind();
			//diffuse is the end result of the edge program. lighting is the final pass that we still need
		}
		void _passGodsRays(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,glm::vec2 lightScrnPos,bool behind,float alpha){
			Settings::clear(true,false,false);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredGodRays)->bind();

			sendUniform4f("RaysInfo",godRays_exposure,godRays_decay,godRays_density,godRays_weight);

			sendUniform2f("lightPositionOnScreen",lightScrnPos.x/float(fbufferWidth),lightScrnPos.y/float(fbufferHeight));

			sendUniform1i("samples",godRays_samples);
			sendUniform1i("behind",int(behind));
			sendUniform1f("alpha",alpha);

			float _divisor = gbuffer->getBuffer(GBufferType::GodRays)->divisor();
			sendUniform1f("fbufferDivisor",_divisor);

			bindTexture("firstPass",gbuffer->getTexture(GBufferType::Lighting),0);

			_renderFullscreenTriangle(fbufferWidth,fbufferHeight);

			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredGodRays)->unbind();
		}
		void _passHDR(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHDR)->bind();

			sendUniform4f("HDRInfo",hdr_exposure,float(int(hdr)),float(int(bloom)),float(int(hdr_algorithm)));

			sendUniform1iSafe("HasLighting",int(lighting));

			bindTextureSafe("lightingBuffer",gbuffer->getTexture(GBufferType::Lighting),0);
			bindTextureSafe("bloomBuffer",gbuffer->getTexture(GBufferType::Bloom),1);
			bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),2);
			bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),3);
			_renderFullscreenTriangle(fbufferWidth,fbufferHeight);

			for(uint i = 0; i < 4; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredHDR)->unbind();
		}
		void _passBlur(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,string type, GLuint texture,string channels){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlur)->bind();

			sendUniform1f("radius",bloom_radius);
			sendUniform4f("strengthModifier",bloom_strength,bloom_strength,bloom_strength,ssao_blur_strength);

			float _divisor = gbuffer->getBuffer(GBufferType::Bloom)->divisor();
			sendUniform1f("fbufferDivisor",_divisor);

			glm::vec4 rgba(0.0f);
			if(channels.find("R") != string::npos) rgba.x = 1.0f;
			if(channels.find("G") != string::npos) rgba.y = 1.0f;
			if(channels.find("B") != string::npos) rgba.z = 1.0f;
			if(channels.find("A") != string::npos) rgba.w = 1.0f;

			sendUniform4f("RGBA",rgba.x,rgba.y,rgba.z,rgba.w);

			if(type == "H"){ sendUniform2f("HV",1.0f,0.0f); }
			else{            sendUniform2f("HV",0.0f,1.0f); }

			bindTexture("textureMap",gbuffer->getTexture(texture),0);

			_renderFullscreenTriangle(fbufferWidth,fbufferHeight);

			unbindTexture2D(0);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredBlur)->unbind();
		}
		void _passFXAA(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight,bool renderAA){
			if(!renderAA) return;

			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFXAA)->bind();

			sendUniform1f("FXAA_REDUCE_MIN",FXAA_REDUCE_MIN);
			sendUniform1f("FXAA_REDUCE_MUL",FXAA_REDUCE_MUL);
			sendUniform1f("FXAA_SPAN_MAX",FXAA_SPAN_MAX);

			sendUniform2f("resolution",float(fboWidth),float(fboHeight));
			bindTexture("sampler0",gbuffer->getTexture(GBufferType::Lighting),0);
			bindTextureSafe("edgeTexture",gbuffer->getTexture(GBufferType::Misc),1);
			bindTexture("depthTexture",gbuffer->getTexture(GBufferType::Depth),2);
			_renderFullscreenTriangle(fboWidth,fboHeight);

			for(uint i = 0; i < 3; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFXAA)->unbind();
		}
		void _passSMAA(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight,bool renderAA){
			if(!renderAA) return;
			glm::vec4 SMAA_PIXEL_SIZE = glm::vec4(float(1.0f / float(fboWidth)), float(1.0f / float(fboHeight)), float(fboWidth), float(fboHeight));

			/*
			#pragma region StencilEdgePass
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1Stencil)->bind();
			gbuffer->getMainFBO()->bind();

			Settings::clear(false,false,true); //stencil is completely filled with 0's
			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
			GLEnable(GLState::STENCIL_TEST);

			sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
			sendUniform1f("SMAA_THRESHOLD",SMAA_THRESHOLD);
			sendUniform1fSafe("SMAA_DEPTH_THRESHOLD",SMAA_DEPTH_THRESHOLD);
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Lighting),0);
			_renderFullscreenTriangle(fboWidth,fboHeight);

			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_EQUAL, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil

			for(uint i = 0; i < 1; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1Stencil)->unbind();
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

			#pragma endregion
			*/

			#pragma region PassEdge
			gbuffer->start(GBufferType::Misc);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1)->bind();

			Settings::clear(true,false,true); 
			//Settings::clear(false,false,true);

			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
			GLEnable(GLState::STENCIL_TEST);

			sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
			sendUniform1f("SMAA_THRESHOLD",SMAA_THRESHOLD);
			sendUniform1fSafe("SMAA_DEPTH_THRESHOLD",SMAA_DEPTH_THRESHOLD);
			sendUniform1fSafe("SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR",SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR);
			sendUniform1iSafe("SMAA_PREDICATION",int(SMAA_PREDICATION));
			sendUniform1fSafe("SMAA_PREDICATION_THRESHOLD",SMAA_PREDICATION_THRESHOLD);
			sendUniform1fSafe("SMAA_PREDICATION_SCALE",SMAA_PREDICATION_SCALE);
			sendUniform1fSafe("SMAA_PREDICATION_STRENGTH",SMAA_PREDICATION_STRENGTH);
			bindTexture("textureMap",gbuffer->getTexture(GBufferType::Lighting),0);
			bindTextureSafe("texturePredication",gbuffer->getTexture(GBufferType::Diffuse),1);

			_renderFullscreenTriangle(fboWidth,fboHeight);

			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_EQUAL, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil

			for(uint i = 0; i < 2; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA1)->unbind();
			#pragma endregion
	
			#pragma region PassBlend
			gbuffer->start(GBufferType::Normal);
			Settings::clear(true,false,false); //clear color only

			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA2)->bind();
			sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
			sendUniform1iSafe("SMAA_MAX_SEARCH_STEPS",SMAA_MAX_SEARCH_STEPS);

			bindTexture("edge_tex",gbuffer->getTexture(GBufferType::Misc),0);
			bindTexture("area_tex",SMAA_AreaTexture,1,GL_TEXTURE_2D);
			bindTexture("search_tex",SMAA_SearchTexture,2,GL_TEXTURE_2D);

			sendUniform1iSafe("SMAA_MAX_SEARCH_STEPS_DIAG",SMAA_MAX_SEARCH_STEPS_DIAG);
			sendUniform1iSafe("SMAA_AREATEX_MAX_DISTANCE",SMAA_AREATEX_MAX_DISTANCE);
			sendUniform1iSafe("SMAA_AREATEX_MAX_DISTANCE_DIAG",SMAA_AREATEX_MAX_DISTANCE_DIAG);
			sendUniform2fSafe("SMAA_AREATEX_PIXEL_SIZE",SMAA_AREATEX_PIXEL_SIZE);
			sendUniform1fSafe("SMAA_AREATEX_SUBTEX_SIZE",SMAA_AREATEX_SUBTEX_SIZE);
			sendUniform1iSafe("SMAA_CORNER_ROUNDING",SMAA_CORNER_ROUNDING);
			sendUniform1fSafe("SMAA_CORNER_ROUNDING_NORM",(float(SMAA_CORNER_ROUNDING) / 100.0f));

			_renderFullscreenTriangle(fboWidth,fboHeight);

			for(uint i = 0; i < 3; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA2)->unbind();
			#pragma endregion

			GLDisable(GLState::STENCIL_TEST);

			#pragma region PassNeighbor
			//gbuffer->start(GBufferType::Misc);
			gbuffer->stop();
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA3)->bind();
			sendUniform4f("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
			bindTextureSafe("textureMap",gbuffer->getTexture(GBufferType::Lighting),0); //need original final image from first smaa pass
			bindTextureSafe("blend_tex",gbuffer->getTexture(GBufferType::Normal),1);

			_renderFullscreenTriangle(fboWidth,fboHeight);

			for(uint i = 0; i < 2; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA3)->unbind();
			#pragma endregion

			#pragma region PassFinalCustom
			/*
			//this pass is optional. lets skip it for now
			//gbuffer->start(GBufferType::Lighting);
			gbuffer->stop();
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA4)->bind();
			renderFullscreenTriangle(fboWidth,fboHeight);
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::SMAA4)->unbind();
			*/  
			#pragma endregion
		}
		void _passFinal(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight){
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFinal)->bind();

			sendUniform1iSafe("HasSSAO",int(ssao));
			sendUniform1iSafe("HasRays",int(godRays));
			sendUniform1fSafe("godRaysExposure",godRays_exposure);

			bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0); 
			bindTextureSafe("gMiscMap",gbuffer->getTexture(GBufferType::Misc),1);
			bindTextureSafe("gGodsRaysMap",gbuffer->getTexture(GBufferType::GodRays),2);
			bindTextureSafe("gBloomMap",gbuffer->getTexture(GBufferType::Bloom),3);
			//bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),4);
			//bindTextureSafe("gLightMap",gbuffer->getTexture(GBufferType::Lighting),5);

			_renderFullscreenTriangle(fboWidth,fboHeight);

			for(uint i = 0; i < 4; ++i){ unbindTexture2D(i); }
			m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredFinal)->unbind();
		}
		void _renderFullscreenQuad(uint& width,uint& height){
			float w2 = float(width) * 0.5f;
			float h2 = float(height) * 0.5f;
			glm::mat4 p = glm::ortho(-w2,w2,-h2,h2);
			sendUniformMatrix4f("MVP",p);
			sendUniform2f("screenSizeDivideBy2",w2,h2);
			setViewport(0,0,width,height);

			m_FullscreenQuad->render();
		}
		void _renderFullscreenTriangle(uint& width,uint& height){
			float w2 = float(width) * 0.5f;
			float h2 = float(height) * 0.5f;
			glm::mat4 p = glm::ortho(-w2,w2,-h2,h2);
			sendUniformMatrix4f("MVP",p);
			sendUniform2f("screenSizeDivideBy2",w2,h2);
			setViewport(0,0,width,height);
			//apparently drawing oversized triangles is better performance wise as a quad will process the pixels along the triangles' diagonal twice,
			//and culling out the unseen geometry from the oversized triangle (opengl does this automatically) is alot cheaper than the alternative render diagonal pixels twice
    
			m_FullscreenTriangle->render();
		}
		void _render(GBuffer* gbuffer,Camera* camera,uint& fboWidth,uint& fboHeight,bool& doSSAO, bool& doGodRays, bool& doAA,bool& HUD, Object* ignore,bool& mainRenderFunc,GLuint& fbo, GLuint& rbo){
			Scene* s = Resources::getCurrentScene();
			if(mainRenderFunc){
				if(s->lightProbes().size() > 0){
					for(auto lightProbe:s->lightProbes()){
						lightProbe.second->renderCubemap(
							m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapConvolude),
							m_InternalShaderPrograms.at(EngineInternalShaderPrograms::CubemapPrefilterEnv)
						);
					}
					m_gBuffer->resize(fboWidth,fboHeight);
				}
			}

			if(doSSAO == false) Renderer::Settings::SSAO::disable();
			if(doGodRays == false) Renderer::Settings::GodRays::disable();
			if(doAA == false) aa_algorithm = AntiAliasingAlgorithm::None;

			_passGeometry(gbuffer,camera,fboWidth,fboHeight,ignore);

			if(godRays){
				gbuffer->start(GBufferType::GodRays,"RGBA",false);
				Object* o = Resources::getObject("Sun");
				glm::vec3 sp = Math::getScreenCoordinates(o->getPosition(),false);
				glm::vec3 camPos = camera->getPosition();
				glm::vec3 oPos = o->getPosition();
				glm::vec3 camVec = camera->getViewVector();
				bool behind = Math::isPointWithinCone(camPos,-camVec,oPos,Math::toRadians(godRays_fovDegrees));
				float alpha = Math::getAngleBetweenTwoVectors(camVec,camPos - oPos,true) / godRays_fovDegrees;

				alpha = glm::pow(alpha,godRays_alphaFalloff);
				alpha = glm::clamp(alpha,0.0001f,0.9999f);

				_passGodsRays(gbuffer,camera,fboWidth,fboHeight,glm::vec2(sp.x,sp.y),!behind,1.0f - alpha);
			}
			GLDisable(GLState::BLEND);

			//confirm, stencil rejection does help
			_passStencil(gbuffer,camera,fboWidth,fboHeight);

			GLEnable(GLState::BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			if(lighting == true && s->lights().size() > 0){
				gbuffer->start(GBufferType::Lighting,"RGB");
				Renderer::Settings::clear(true,false,false);//this is needed for godrays
				_passLighting(gbuffer,camera,fboWidth,fboHeight,mainRenderFunc);
			}
			GLDisable(GLState::BLEND);
			//_passForwardRendering(c,fboWidth,fbufferHeight,ignore);

			GLDisable(GLState::STENCIL_TEST);

			string _channels;
			bool isdoingssao = false;
			if(doSSAO && ssao){ isdoingssao = true; _channels = "RGBA"; }
			else{                                   _channels = "RGB";  }

			gbuffer->start(GBufferType::Bloom,_channels,false);
			_passSSAO(gbuffer,camera,fboWidth,fboHeight); //ssao AND bloom
			if(ssao_do_blur || bloom){
				gbuffer->start(GBufferType::Free2,_channels,false);
				_passBlur(gbuffer,camera,fboWidth,fboHeight,"H",GBufferType::Bloom,_channels);
				gbuffer->start(GBufferType::Bloom,_channels,false);
				_passBlur(gbuffer,camera,fboWidth,fboHeight,"V",GBufferType::Free2,_channels);
			}
			gbuffer->start(GBufferType::Misc);
			_passHDR(gbuffer,camera,fboWidth,fboHeight);

	
			bool doingaa = false;
			if(doAA && aa_algorithm != AntiAliasingAlgorithm::None) doingaa = true;

			if(aa_algorithm == AntiAliasingAlgorithm::None || !doingaa){
				gbuffer->stop(fbo,rbo);
				_passFinal(gbuffer,camera,fboWidth,fboHeight);
			}
			else if(aa_algorithm == AntiAliasingAlgorithm::FXAA && doingaa){
				gbuffer->start(GBufferType::Lighting);
				_passFinal(gbuffer,camera,fboWidth,fboHeight);

				//_passEdgeCanny(gbuffer,camera,fboWidth,fboHeight,GBufferType::Lighting);

				gbuffer->stop(fbo,rbo);
				_passFXAA(gbuffer,camera,fboWidth,fboHeight,doingaa);
			}
			else if(aa_algorithm == AntiAliasingAlgorithm::SMAA && doingaa){
				gbuffer->start(GBufferType::Lighting);
				_passFinal(gbuffer,camera,fboWidth,fboHeight);
				_passSMAA(gbuffer,camera,fboWidth,fboHeight,doingaa);
			}

			_passCopyDepth(gbuffer,camera,fboWidth,fboHeight);

			GLEnable(GLState::BLEND);
			GLDisable(GLState::DEPTH_TEST);
			GLDisable(GLState::DEPTH_MASK);
			if(mainRenderFunc){
				if(draw_physics_debug && camera == s->getActiveCamera()){
					Core::m_Engine->m_PhysicsManager->_render();
				}
			}

			//to try and see what the lightprobe is outputting
			/*
			Renderer::unbindFBO();
			Settings::clear();
			LightProbe* pr  = (LightProbe*)(Resources::getCamera("CapsuleLightProbe"));
			Skybox* skybox = (Skybox*)(s->getSkybox());
			if(pr != nullptr){
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->bind();
				glm::mat4 view = glm::mat4(glm::mat3(camera->getView()));
				Renderer::sendUniformMatrix4f("VP",camera->getProjection() * view);
				GLuint address = pr->getEnvMap();
				Renderer::bindTexture("Texture",address,0,GL_TEXTURE_CUBE_MAP);
				Skybox::bindMesh();
				Renderer::unbindTextureCubemap(0);
				m_InternalShaderPrograms.at(EngineInternalShaderPrograms::DeferredSkybox)->unbind();
			}
			*/
	
			GLEnable(GLState::DEPTH_TEST);
			GLEnable(GLState::DEPTH_MASK);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if(mainRenderFunc){
				if(HUD == true){
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
		}

};
epriv::RenderManager::RenderManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::RenderManager::~RenderManager(){ m_i->_destruct(); }
void epriv::RenderManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::RenderManager::_render(GBuffer* g,Camera* c,uint fboW,uint fboH,bool ssao,bool rays,bool AA,bool HUD,Object* ignore,bool mainFunc,GLuint display_fbo,GLuint display_rbo){ m_i->_render(g,c,fboW,fboH,ssao,rays,AA,HUD,ignore,mainFunc,display_fbo,display_rbo); }
void epriv::RenderManager::_render(Camera* c,uint fboW,uint fboH,bool ssao,bool rays,bool AA,bool HUD,Object* ignore,bool mainFunc,GLuint display_fbo,GLuint display_rbo){m_i->_render(Core::m_Engine->m_RenderManager->m_i->m_gBuffer,c,fboW,fboH,ssao,rays,AA,HUD,ignore,mainFunc,display_fbo,display_rbo);}
void epriv::RenderManager::_resize(uint w,uint h){ m_i->_resize(w,h); }
void epriv::RenderManager::_resizeGbuffer(uint w,uint h){ Core::m_Engine->m_RenderManager->m_i->m_gBuffer->resize(w,h); }
void epriv::RenderManager::_onFullscreen(sf::Window* w,sf::VideoMode m,const char* n,uint s,sf::ContextSettings& set){ m_i->_onFullscreen(w,m,n,s,set); }
void epriv::RenderManager::_onOpenGLContextCreation(uint w,uint h){ m_i->_onOpenGLContextCreation(w,h); }

void epriv::RenderManager::_renderText(Font* font,string text,glm::vec2 pos,glm::vec4 color,glm::vec2 scl,float angle,float depth){
	Core::m_Engine->m_RenderManager->m_i->m_FontsToBeRendered.push_back(FontRenderInfo(font,text,pos,color,scl,angle,depth));
}
void epriv::RenderManager::_renderTexture(Texture* texture,glm::vec2 pos,glm::vec4 color,glm::vec2 scl,float angle,float depth){
	Core::m_Engine->m_RenderManager->m_i->m_TexturesToBeRendered.push_back(TextureRenderInfo(texture,pos,color,scl,angle,depth));
}
void epriv::RenderManager::_addShaderToStage(ShaderP* program,uint stage){
    if(stage == ShaderRenderPass::Geometry){
        Core::m_Engine->m_RenderManager->m_i->m_GeometryPassShaderPrograms.push_back(program);
    }
    else if(stage == ShaderRenderPass::Forward){
        Core::m_Engine->m_RenderManager->m_i->m_ForwardPassShaderPrograms.push_back(program);
    }
    else if(stage == ShaderRenderPass::Lighting){
    }
    else if(stage == ShaderRenderPass::Postprocess){
    }
    else{
    }
}
void epriv::RenderManager::_bindShaderProgram(ShaderP* p){
    if(Core::m_Engine->m_RenderManager->m_i->current_shader_program != p){
		glUseProgram(p->program());
        Core::m_Engine->m_RenderManager->m_i->current_shader_program = p;
    }
}
bool epriv::RenderManager::_bindMaterial(Material* m){
    if(Core::m_Engine->m_RenderManager->m_i->current_bound_material != m){
        Core::m_Engine->m_RenderManager->m_i->current_bound_material = m;
		return true;
    }
	return false;
}
bool epriv::RenderManager::_unbindMaterial(){
    if(Core::m_Engine->m_RenderManager->m_i->current_bound_material != nullptr){   
        Core::m_Engine->m_RenderManager->m_i->current_bound_material = nullptr;
		return true;
    }
	return false;
}
void epriv::RenderManager::_genPBREnvMapData(Texture* texture, uint size1, uint size2){
	m_i->_generatePBREnvMapData(texture,size1,size2);
}

void Renderer::Settings::FXAA::setReduceMin(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_REDUCE_MIN = glm::max(0.0f,r); }
void Renderer::Settings::FXAA::setReduceMul(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_REDUCE_MUL = glm::max(0.0f,r); }
void Renderer::Settings::FXAA::setSpanMax(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_SPAN_MAX = glm::max(0.0f,r); }
float Renderer::Settings::FXAA::getReduceMin(){ return epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_REDUCE_MIN; }
float Renderer::Settings::FXAA::getReduceMul(){ return epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_REDUCE_MUL; }
float Renderer::Settings::FXAA::getSpanMax(){ return epriv::Core::m_Engine->m_RenderManager->m_i->FXAA_SPAN_MAX; }
bool Renderer::Settings::HDR::enabled(){ return epriv::Core::m_Engine->m_RenderManager->m_i->hdr; }
void Renderer::Settings::HDR::enable(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->hdr = b; }
void Renderer::Settings::HDR::disable(){ epriv::Core::m_Engine->m_RenderManager->m_i->hdr = false; }
float Renderer::Settings::HDR::getExposure(){ return epriv::Core::m_Engine->m_RenderManager->m_i->hdr_exposure; }
void Renderer::Settings::HDR::setExposure(float e){ epriv::Core::m_Engine->m_RenderManager->m_i->hdr_exposure = e; }
void Renderer::Settings::HDR::setAlgorithm(HDRAlgorithm::Algorithm a){ epriv::Core::m_Engine->m_RenderManager->m_i->hdr_algorithm = a; }
void Renderer::Settings::Bloom::enable(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->bloom = b; }
void Renderer::Settings::Bloom::disable(){ epriv::Core::m_Engine->m_RenderManager->m_i->bloom = false; }
float Renderer::Settings::Bloom::getRadius(){ return epriv::Core::m_Engine->m_RenderManager->m_i->bloom_radius; }
float Renderer::Settings::Bloom::getStrength(){ return epriv::Core::m_Engine->m_RenderManager->m_i->bloom_strength; }
void Renderer::Settings::Bloom::setRadius(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->bloom_radius = glm::max(0.0f,r); }
void Renderer::Settings::Bloom::setStrength(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->bloom_strength = glm::max(0.0f,r); }
void Renderer::Settings::SMAA::setThreshold(float f){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_THRESHOLD = f; }
void Renderer::Settings::SMAA::setSearchSteps(uint s){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_MAX_SEARCH_STEPS = s; }
void Renderer::Settings::SMAA::disableCornerDetection(){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_CORNER_ROUNDING = 0; }
void Renderer::Settings::SMAA::enableCornerDetection(uint c){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_CORNER_ROUNDING = c; }
void Renderer::Settings::SMAA::disableDiagonalDetection(){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_MAX_SEARCH_STEPS_DIAG = 0; }
void Renderer::Settings::SMAA::enableDiagonalDetection(uint d){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_MAX_SEARCH_STEPS_DIAG = d; }
void Renderer::Settings::SMAA::setPredicationThreshold(float f){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_PREDICATION_THRESHOLD = f; }
void Renderer::Settings::SMAA::setPredicationScale(float f){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_PREDICATION_SCALE = f; }
void Renderer::Settings::SMAA::setPredicationStrength(float s){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_PREDICATION_STRENGTH = s; }
void Renderer::Settings::SMAA::setReprojectionScale(float s){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_REPROJECTION_WEIGHT_SCALE = s; }
void Renderer::Settings::SMAA::enablePredication(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_PREDICATION = b; }
void Renderer::Settings::SMAA::disablePredication(){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_PREDICATION = false; }
void Renderer::Settings::SMAA::enableReprojection(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_REPROJECTION = b; }
void Renderer::Settings::SMAA::disableReprojection(){ epriv::Core::m_Engine->m_RenderManager->m_i->SMAA_REPROJECTION = false; }
void Renderer::Settings::SMAA::setQuality(SMAAQualityLevel::Level level){ epriv::Core::m_Engine->m_RenderManager->m_i->_setSMAAQuality(level); }
bool Renderer::Settings::GodRays::enabled(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays; }
void Renderer::Settings::GodRays::enable(bool b = true){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays = b; }
void Renderer::Settings::GodRays::disable(){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays = false; }
float Renderer::Settings::GodRays::getExposure(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_exposure; }
float Renderer::Settings::GodRays::getDecay(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_decay; }
float Renderer::Settings::GodRays::getDensity(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_density; }
float Renderer::Settings::GodRays::getWeight(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_weight; }
uint Renderer::Settings::GodRays::getSamples(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_samples; }
float Renderer::Settings::GodRays::getFOVDegrees(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_fovDegrees; }
float Renderer::Settings::GodRays::getAlphaFalloff(){ return epriv::Core::m_Engine->m_RenderManager->m_i->godRays_alphaFalloff; }
void Renderer::Settings::GodRays::setExposure(float e){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_exposure = e; }
void Renderer::Settings::GodRays::setDecay(float d){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_decay = d; }
void Renderer::Settings::GodRays::setDensity(float d){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_density = d; }
void Renderer::Settings::GodRays::setWeight(float w){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_weight = w; }
void Renderer::Settings::GodRays::setSamples(uint s){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_samples = s; }
void Renderer::Settings::GodRays::setFOVDegrees(float d){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_fovDegrees = d; }
void Renderer::Settings::GodRays::setAlphaFalloff(float a){ epriv::Core::m_Engine->m_RenderManager->m_i->godRays_alphaFalloff = a; }
void Renderer::Settings::Lighting::enable(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->lighting = b; }
void Renderer::Settings::Lighting::disable(){ epriv::Core::m_Engine->m_RenderManager->m_i->lighting = false; }
bool Renderer::Settings::SSAO::enabled(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao;  }
void Renderer::Settings::SSAO::enable(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao = b;  }
void Renderer::Settings::SSAO::disable(){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao = false;  }
void Renderer::Settings::SSAO::enableBlur(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_do_blur = b;  }
void Renderer::Settings::SSAO::disableBlur(){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_do_blur = false;  }
float Renderer::Settings::SSAO::getBlurStrength(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_blur_strength; }
float Renderer::Settings::SSAO::getIntensity(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_intensity; }
float Renderer::Settings::SSAO::getRadius(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_radius; }
float Renderer::Settings::SSAO::getScale(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_scale; }
float Renderer::Settings::SSAO::getBias(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_bias; }
uint Renderer::Settings::SSAO::getSamples(){ return epriv::Core::m_Engine->m_RenderManager->m_i->ssao_samples; }
void Renderer::Settings::SSAO::setBlurStrength(float s){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_blur_strength = glm::max(0.0f,s); }
void Renderer::Settings::SSAO::setIntensity(float i){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_intensity = glm::max(0.0f,i); }
void Renderer::Settings::SSAO::setRadius(float r){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_radius = glm::max(0.0f,r); }
void Renderer::Settings::SSAO::setScale(float s){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_scale = glm::max(0.0f,s); }
void Renderer::Settings::SSAO::setBias(float b){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_bias = b; }
void Renderer::Settings::SSAO::setSamples(uint s){ epriv::Core::m_Engine->m_RenderManager->m_i->ssao_samples = s; }
void Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm algorithm){ epriv::Core::m_Engine->m_RenderManager->m_i->_setAntiAliasingAlgorithm(algorithm); }
void Renderer::Settings::cullFace(uint s){ epriv::Core::m_Engine->m_RenderManager->m_i->_cullFace(s); }
void Renderer::Settings::clear(bool color, bool depth, bool stencil){
    if(!color && !depth && !stencil) return;
	//if(depth){ enableDepthMask(); }
	if(color == true && depth == true && stencil == true)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	else if(color == true && depth == true && stencil == false)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else if(color == true && depth == false && stencil == false)
		glClear(GL_COLOR_BUFFER_BIT);
	else if(color == false && depth == true && stencil == true)
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	else if(color == false && depth == false && stencil == true)
		glClear(GL_STENCIL_BUFFER_BIT);
	else if(color == false && depth == true && stencil == false)
		glClear(GL_DEPTH_BUFFER_BIT);
	else if(color == true && depth == false && stencil == true)
		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void Renderer::Settings::enableDrawPhysicsInfo(bool b){ epriv::Core::m_Engine->m_RenderManager->m_i->draw_physics_debug = b; }
void Renderer::Settings::disableDrawPhysicsInfo(){ epriv::Core::m_Engine->m_RenderManager->m_i->draw_physics_debug = false; }
void Renderer::Settings::setGamma(float g){ epriv::Core::m_Engine->m_RenderManager->m_i->gamma = g; }
float Renderer::Settings::getGamma(){ return epriv::Core::m_Engine->m_RenderManager->m_i->gamma; }

void Renderer::setViewport(uint x,uint y,uint w,uint h){ epriv::Core::m_Engine->m_RenderManager->m_i->_setViewport(x,y,w,h); }
void Renderer::bindTexture(const char* l,Texture* t,uint s){Renderer::bindTexture(l,t->address(),s,t->type());}
void Renderer::bindTexture(const char* l,GLuint a,uint s,GLuint t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t,a);
    sendUniform1i(l,s);
}
void Renderer::bindTextureSafe(const char* l,Texture* t,uint slot){Renderer::bindTextureSafe(l,t->address(),slot,t->type());}
void Renderer::bindTextureSafe(const char* l,GLuint a,uint s,GLuint t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t,a);
    sendUniform1iSafe(l,s);
}
void Renderer::bindReadFBO(GLuint r){ epriv::Core::m_Engine->m_RenderManager->m_i->_bindReadFBO(r); }
void Renderer::bindFBO(epriv::FramebufferObject* fbo){ Renderer::bindFBO(fbo->address()); }
void Renderer::bindRBO(epriv::RenderbufferObject* rbo){ Renderer::bindRBO(rbo->address()); }
void Renderer::bindDrawFBO(GLuint d){ epriv::Core::m_Engine->m_RenderManager->m_i->_bindDrawFBO(d); }
void Renderer::bindFBO(GLuint f){Renderer::bindReadFBO(f);Renderer::bindDrawFBO(f);}
void Renderer::bindRBO(GLuint r){ epriv::Core::m_Engine->m_RenderManager->m_i->_bindRBO(r); }
void Renderer::unbindFBO(){ Renderer::bindFBO(GLuint(0)); }
void Renderer::unbindRBO(){ Renderer::bindRBO(GLuint(0)); }
void Renderer::unbindReadFBO(){ Renderer::bindReadFBO(0); }
void Renderer::unbindDrawFBO(){ Renderer::bindDrawFBO(0); }
void Renderer::unbindTexture(uint s,Texture* t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t->type(),0);
}
void Renderer::unbindTexture2D(uint s){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(GL_TEXTURE_2D,0);
}
void Renderer::unbindTextureCubemap(uint s){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
}
void Renderer::renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth){
	epriv::Core::m_Engine->m_RenderManager->m_i->m_TexturesToBeRendered.push_back(epriv::TextureRenderInfo(nullptr,pos,col,glm::vec2(w,h),angle,depth));
}
void Renderer::renderTexture(Texture* texture,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth){
    texture->render(pos,col,angle,scl,depth);
}
void Renderer::renderText(string& text,Font* font, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth){
    font->renderText(text,pos,color,angle,scl,depth);
}
void Renderer::renderFullscreenQuad(uint w, uint h){ epriv::Core::m_Engine->m_RenderManager->m_i->_renderFullscreenQuad(w,h); }
void Renderer::renderFullscreenTriangle(uint w,uint h){ epriv::Core::m_Engine->m_RenderManager->m_i->_renderFullscreenTriangle(w,h); }
inline const GLint Renderer::getUniformLoc(const char* location){const unordered_map<string,GLint>& m = epriv::Core::m_Engine->m_RenderManager->m_i->current_shader_program->uniforms();if(!m.count(location))return - 1;return m.at(location);}
inline const GLint& Renderer::getUniformLocUnsafe(const char* location){return epriv::Core::m_Engine->m_RenderManager->m_i->current_shader_program->uniforms().at(location); }