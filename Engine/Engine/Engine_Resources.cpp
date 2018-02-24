#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_Resources.h"
#include "Engine_Sounds.h"
#include "Engine_Window.h"
#include "ShaderProgram.h"
#include "Object.h"
#include "Light.h"
#include "ObjectDynamic.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Texture.h"
#include "Font.h"
#include "Scene.h"
#include "Engine_BuiltInShaders.h"

#include <boost/make_shared.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

using namespace Engine;
using namespace std;

template<class V,class S>S _incrementName(unordered_map<S,V>& m,const S n){S r=n;if(m.size()>0){uint c=0;while(m.count(r)){r=n+" "+boost::lexical_cast<S>(c);c++;}}return r;}
template<class V,class S>S _incrementName(map<S,V>& m,const S n){S r=n;if(m.size()>0){uint c=0;while(m.count(r)){r=n+" "+boost::lexical_cast<S>(c);c++;}}return r;}
template<class V, class O,class S>void _addToContainer(map<S,V>& m,const S& n,O& o){if(m.size()>0&&m.count(n)){o.reset();return;}m.emplace(n,o);}
template<class V, class O,class S>void _addToContainer(unordered_map<S,V>& m,const S& n,O& o){if(m.size()>0&&m.count(n)){o.reset();return;}m.emplace(n,o);}
template<class V,class S>void* _getFromContainer(map<S,V>& m,const S& n){if(!m.count(n))return nullptr;return m.at(n).get();}
template<class V,class S>void* _getFromContainer(unordered_map<S,V>& m,const S& n){if(!m.count(n))return nullptr;return m.at(n).get();}
template<class V,class S>void _removeFromContainer(map<S,V>& m,const S& n){if(m.size()>0&&m.count(n)){m.at(n).reset();m.erase(n);}}
template<class V,class S>void _removeFromContainer(unordered_map<S,V>& m,const S& n){if(m.size()>0&&m.count(n)){m.at(n).reset();m.erase(n);}}

class epriv::ResourceManager::impl final{
    public:
		//TODO: convert to this resource system --------------------------------------------
		static const uint MAX_ENTRIES = 8192;
		HandleEntry m_Resources[MAX_ENTRIES];
		int m_activeEntryCount;
		uint32 m_firstFreeEntry;

		void _Reset(){
			m_activeEntryCount = 0;
			m_firstFreeEntry = 0;
			for (int i = 0; i < MAX_ENTRIES - 1; ++i){
				m_Resources[i] = HandleEntry(i + 1);
			}
			m_Resources[MAX_ENTRIES - 1] = HandleEntry();
			m_Resources[MAX_ENTRIES - 1].m_endOfList = true;
		}
		Handle _Add(BaseR p, uint32 type){
			assert(m_activeEntryCount < MAX_ENTRIES - 1);
			assert(type >= 0 && type <= 31);

			const int newIndex = m_firstFreeEntry;
			assert(newIndex < MAX_ENTRIES);
			assert(m_Resources[newIndex].m_active == false);
			assert(!m_Resources[newIndex].m_endOfList);

			m_firstFreeEntry = m_Resources[newIndex].m_nextFreeIndex;
			m_Resources[newIndex].m_nextFreeIndex = 0;
			m_Resources[newIndex].m_counter = m_Resources[newIndex].m_counter + 1;
			if (m_Resources[newIndex].m_counter == 0){
				m_Resources[newIndex].m_counter = 1;
			}
			m_Resources[newIndex].m_active = true;
			m_Resources[newIndex].m_resource = p;

			++m_activeEntryCount;
			return Handle (newIndex, m_Resources[newIndex].m_counter, type);
		}
		void _Visualize(){
			std::cout << "--------- Visualizing Resource Array ---------" << std::endl;
			for (int i = 0; i < MAX_ENTRIES - 1; ++i){
				HandleEntry& e = m_Resources[i];

				if(e.m_resource){
					std::cout << i << ": Active: " << e.m_active << " , Counter: " << e.m_counter << " , Name: " << ((EngineResource*)e.m_resource)->name() << std::endl;
				}
			}
			std::cout << "----------------------------------------------" << std::endl;
		}
		void _Update(Handle h, BaseR p){
			const int index = h.m_index;
			assert(m_entries[index].m_counter == h.m_counter);
			assert(m_entries[index].m_active == true);

			m_Resources[index].m_resource = p;
		}
		void _Remove(const Handle h){
			const uint32 index = h.m_index;
			assert(m_entries[index].m_counter == h.m_counter);
			assert(m_entries[index].m_active == true);

			m_Resources[index].m_nextFreeIndex = m_firstFreeEntry;
			//m_Resources[index].m_active = 0; //im sure this works just as fine as the line below
			m_Resources[index].m_active = false;
			m_firstFreeEntry = index;

			--m_activeEntryCount;
		}
		BaseR _Get(Handle h) const{
			BaseR p = NULL;
			if (!_Get(h, p)) return NULL;
			return p;
		}
		bool _Get(const Handle h, BaseR& out) const{
			const int index = h.m_index;
			if (m_Resources[index].m_counter != h.m_counter || m_Resources[index].m_active == false)
				return false;
			out = m_Resources[index].m_resource;
			return true;
		}
		template<typename T> inline bool _GetAs(Handle h, T*& out) const {
			BaseR _void;
			const bool rv = _Get(h,_void);
			//out = union_cast<T>(_void);
			out = (T*)_void;
			return rv;
		}
		//-----------------------------------------------------------------------------------------------


		Engine_Window* m_Window;
        Scene* m_CurrentScene;
		bool m_DynamicMemory;

        unordered_map<string,boost::shared_ptr<MeshInstance>> m_MeshInstances;
        unordered_map<string,boost::shared_ptr<Scene>> m_Scenes;
        unordered_map<string,boost::shared_ptr<Object>> m_Objects;
        unordered_map<string,boost::shared_ptr<Camera>> m_Cameras;
        unordered_map<string,boost::shared_ptr<Font>> m_Fonts;
        unordered_map<string,boost::shared_ptr<Mesh>> m_Meshes;
        unordered_map<string,boost::shared_ptr<Texture>> m_Textures;
        unordered_map<string,boost::shared_ptr<Material>> m_Materials;
        unordered_map<string,boost::shared_ptr<Shader>> m_Shaders;
        unordered_map<string,boost::shared_ptr<ShaderP>> m_ShaderPrograms;
		unordered_map<string,boost::shared_ptr<SoundData>> m_SoundDatas;

		void _init(const char* name,const uint& width,const uint& height){
			m_CurrentScene = nullptr;
			m_DynamicMemory = false;
		}
		void _postInit(const char* name,uint width,uint height){
			m_Window = new Engine_Window(name,width,height);

			Engine::Shaders::Detail::ShadersManagement::init();

			//add a basic cube mesh
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
			#pragma endregion
			Resources::addMesh("Cube",cubeMesh,CollisionType::None,false);

			#pragma region Shaders
			Shader* fullscreenVertexShader = new Shader("vert_fullscreenQuad",Engine::Shaders::Detail::ShadersManagement::fullscreen_quad_vertex,ShaderType::Vertex,false);
			Shader* fxaa = new Shader("frag_fxaa",Engine::Shaders::Detail::ShadersManagement::fxaa_frag,ShaderType::Fragment,false);
			Shader* vertexBasic = new Shader("vert_basic",Engine::Shaders::Detail::ShadersManagement::vertex_basic,ShaderType::Vertex,false);
			Shader* vertexHUD = new Shader("vert_hud",Engine::Shaders::Detail::ShadersManagement::vertex_hud,ShaderType::Vertex,false);
			Shader* vertexSkybox = new Shader("vert_skybox",Engine::Shaders::Detail::ShadersManagement::vertex_skybox,ShaderType::Vertex,false);
			Shader* deferredFrag = new Shader("deferred_frag",Engine::Shaders::Detail::ShadersManagement::deferred_frag,ShaderType::Fragment,false);
			Shader* deferredFragHUD = new Shader("deferred_frag_hud",Engine::Shaders::Detail::ShadersManagement::deferred_frag_hud,ShaderType::Fragment,false);
			Shader* deferredFragSkybox = new Shader("deferred_frag_skybox",Engine::Shaders::Detail::ShadersManagement::deferred_frag_skybox,ShaderType::Fragment,false);
			Shader* deferredFragSkyboxFake = new Shader("deferred_frag_skybox_fake",Engine::Shaders::Detail::ShadersManagement::deferred_frag_skybox_fake,ShaderType::Fragment,false);
			Shader* copyDepth = new Shader("copy_depth_frag",Engine::Shaders::Detail::ShadersManagement::copy_depth_frag,ShaderType::Fragment,false);
			Shader* ssao = new Shader("ssao_frag",Engine::Shaders::Detail::ShadersManagement::ssao_frag,ShaderType::Fragment,false);
			Shader* hdr = new Shader("hdr_frag",Engine::Shaders::Detail::ShadersManagement::hdr_frag,ShaderType::Fragment,false);
			Shader* blur = new Shader("blur_frag",Engine::Shaders::Detail::ShadersManagement::blur_frag,ShaderType::Fragment,false);
			Shader* godrays = new Shader("godrays_frag",Engine::Shaders::Detail::ShadersManagement::godRays_frag,ShaderType::Fragment,false);
			Shader* finalFrag = new Shader("final_frag",Engine::Shaders::Detail::ShadersManagement::final_frag,ShaderType::Fragment,false);
			Shader* lightingFrag = new Shader("lighting_frag",Engine::Shaders::Detail::ShadersManagement::lighting_frag,ShaderType::Fragment,false);

			Shader* lightingFragGI = new Shader("lighting_frag_gi",Engine::Shaders::Detail::ShadersManagement::lighting_frag_gi,ShaderType::Fragment,false);
			Shader* cubemapConvolude = new Shader("cubemap_convolude_frag",Engine::Shaders::Detail::ShadersManagement::cubemap_convolude_frag,ShaderType::Fragment,false);
			Shader* cubemapPrefilterEnv = new Shader("cubemap_prefilterEnv_frag",Engine::Shaders::Detail::ShadersManagement::cubemap_prefilter_envmap_frag,ShaderType::Fragment,false);
			Shader* brdfPrecompute = new Shader("brdf_precompute_frag",Engine::Shaders::Detail::ShadersManagement::brdf_precompute,ShaderType::Fragment,false);
			Shader* greyscale = new Shader("greyscale_frag",Engine::Shaders::Detail::ShadersManagement::greyscale_frag,ShaderType::Fragment,false);
			Shader* edgeCannyBlur = new Shader("edge_canny_blur",Engine::Shaders::Detail::ShadersManagement::edge_canny_blur,ShaderType::Fragment,false);
			Shader* edgeCannyFrag = new Shader("edge_canny_frag",Engine::Shaders::Detail::ShadersManagement::edge_canny_frag,ShaderType::Fragment,false);
			Shader* stencilPass = new Shader("stencil_pass",Engine::Shaders::Detail::ShadersManagement::stencil_passover,ShaderType::Fragment,false);
    
			Shader* smaa_vert_1 = new Shader("smaa_vert_1",Engine::Shaders::Detail::ShadersManagement::smaa_vertex_1,ShaderType::Vertex,false);
			Shader* smaa_vert_2 = new Shader("smaa_vert_2",Engine::Shaders::Detail::ShadersManagement::smaa_vertex_2,ShaderType::Vertex,false);
			Shader* smaa_vert_3 = new Shader("smaa_vert_3",Engine::Shaders::Detail::ShadersManagement::smaa_vertex_3,ShaderType::Vertex,false);
			Shader* smaa_vert_4 = new Shader("smaa_vert_4",Engine::Shaders::Detail::ShadersManagement::smaa_vertex_4,ShaderType::Vertex,false);
			Shader* smaa_frag_1_stencil = new Shader("smaa_frag_1_stencil",Engine::Shaders::Detail::ShadersManagement::smaa_frag_1_stencil,ShaderType::Fragment,false);
			Shader* smaa_frag_1 = new Shader("smaa_frag_1",Engine::Shaders::Detail::ShadersManagement::smaa_frag_1,ShaderType::Fragment,false);
			Shader* smaa_frag_2 = new Shader("smaa_frag_2",Engine::Shaders::Detail::ShadersManagement::smaa_frag_2,ShaderType::Fragment,false);
			Shader* smaa_frag_3 = new Shader("smaa_frag_3",Engine::Shaders::Detail::ShadersManagement::smaa_frag_3,ShaderType::Fragment,false);
			Shader* smaa_frag_4 = new Shader("smaa_frag_4",Engine::Shaders::Detail::ShadersManagement::smaa_frag_4,ShaderType::Fragment,false);
			#pragma endregion

			#pragma region ShaderPrograms
			Resources::addShaderProgram("Deferred",vertexBasic,deferredFrag,ShaderRenderPass::Geometry);
			Resources::addShaderProgram("Deferred_HUD",vertexHUD,deferredFragHUD,ShaderRenderPass::Geometry);
			Resources::addShaderProgram("Deferred_GodsRays",fullscreenVertexShader,godrays,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Blur",fullscreenVertexShader,blur,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_HDR",fullscreenVertexShader,hdr,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_SSAO",fullscreenVertexShader,ssao,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Final",fullscreenVertexShader,finalFrag,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_FXAA",fullscreenVertexShader,fxaa,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Skybox",vertexSkybox,deferredFragSkybox,ShaderRenderPass::Geometry);
			Resources::addShaderProgram("Deferred_Skybox_Fake",vertexSkybox,deferredFragSkyboxFake,ShaderRenderPass::Geometry);
			Resources::addShaderProgram("Copy_Depth",fullscreenVertexShader,copyDepth,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Light",fullscreenVertexShader,lightingFrag,ShaderRenderPass::Lighting);
			Resources::addShaderProgram("Deferred_Light_GI",fullscreenVertexShader,lightingFragGI,ShaderRenderPass::Lighting);
			Resources::addShaderProgram("Cubemap_Convolude",vertexSkybox,cubemapConvolude,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Cubemap_Prefilter_Env",vertexSkybox,cubemapPrefilterEnv,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("BRDF_Precompute_CookTorrance",fullscreenVertexShader,brdfPrecompute,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Greyscale_Frag",fullscreenVertexShader,greyscale,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Edge_Canny_Blur",fullscreenVertexShader,edgeCannyBlur,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_Edge_Canny",fullscreenVertexShader,edgeCannyFrag,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Stencil_Pass",fullscreenVertexShader,stencilPass,ShaderRenderPass::Postprocess);

			Resources::addShaderProgram("Deferred_SMAA_1_Stencil",smaa_vert_1,smaa_frag_1_stencil,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_SMAA_1",smaa_vert_1,smaa_frag_1,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_SMAA_2",smaa_vert_2,smaa_frag_2,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_SMAA_3",smaa_vert_3,smaa_frag_3,ShaderRenderPass::Postprocess);
			Resources::addShaderProgram("Deferred_SMAA_4",smaa_vert_4,smaa_frag_4,ShaderRenderPass::Postprocess);
			#pragma endregion

			Texture* brdfCook = new Texture("BRDFCookTorrance",512,512,ImageInternalFormat::RG16F,ImagePixelFormat::RG,ImagePixelType::FLOAT,GL_TEXTURE_2D,1.0f);
			brdfCook->setWrapping(TextureWrap::ClampToEdge);

			Resources::addMaterial("Default","","","","","Deferred");

			Resources::addMesh("Plane",1.0f,1.0f);
		}
		void _destruct(){
			for(uint i = 0; i < MAX_ENTRIES; ++i){
				SAFE_DELETE(m_Resources[i].m_resource); 
			}


			for (auto it = m_MeshInstances.begin();it != m_MeshInstances.end(); ++it )   it->second.reset();
			for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )                 it->second.reset();
			for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )             it->second.reset();
			for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                   it->second.reset();
			for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )           it->second.reset();
			for (auto it = m_ShaderPrograms.begin();it != m_ShaderPrograms.end(); ++it ) it->second.reset();
			for (auto it = m_Shaders.begin();it != m_Shaders.end(); ++it )               it->second.reset();
			for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )               it->second.reset();
			for (auto it = m_Cameras.begin();it != m_Cameras.end(); ++it )               it->second.reset();
			for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )                 it->second.reset();
			for (auto it = m_SoundDatas.begin();it != m_SoundDatas.end(); ++it )         it->second.reset();
			SAFE_DELETE(m_Window);
		}
};
void epriv::ResourceManager::_init(const char* n,uint w,uint h){
	m_i->_postInit(n,w,h);
}



void epriv::ResourceManager::_addResource(BaseR r,ResourceType::Type t){
	epriv::Core::m_Engine->m_ResourceManager->m_i->_Add(r,(uint)t);
}








epriv::ResourceManager::ResourceManager(const char* name,uint width,uint height):m_i(new impl){
	m_i->_init(name,width,height);
}
epriv::ResourceManager::~ResourceManager(){
	m_i->_destruct();
}
string Engine::Data::reportTime(){
	return epriv::Core::m_Engine->m_TimeManager->reportTime();
}
string Engine::Data::reportTimeRendering(){
	return epriv::Core::m_Engine->m_TimeManager->reportTimeRendering();
}
float Engine::Resources::dt(){ return epriv::Core::m_Engine->m_TimeManager->dt(); }
float Engine::Resources::applicationTime(){ return epriv::Core::m_Engine->m_TimeManager->applicationTime(); }
Scene* Engine::Resources::getCurrentScene(){ return epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene; }

bool epriv::ResourceManager::_hasMaterial(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Materials.count(n); }
bool epriv::ResourceManager::_hasMesh(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Meshes.count(n); }
bool epriv::ResourceManager::_hasTexture(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Textures.count(n); }
bool epriv::ResourceManager::_hasObject(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Objects.count(n); }
bool epriv::ResourceManager::_hasFont(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Fonts.count(n); }
bool epriv::ResourceManager::_hasScene(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Scenes.count(n); }
bool epriv::ResourceManager::_hasMeshInstance(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_MeshInstances.count(n); }
bool epriv::ResourceManager::_hasCamera(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Cameras.count(n); }
bool epriv::ResourceManager::_hasShader(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_Shaders.count(n); }
bool epriv::ResourceManager::_hasSoundData(string n){ return Core::m_Engine->m_ResourceManager->m_i->m_SoundDatas.count(n); }
void epriv::ResourceManager::_addScene(Scene* s){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Scenes,s->name(),boost::shared_ptr<Scene>(s));
}
void epriv::ResourceManager::_addCamera(Camera* c){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Cameras,c->name(),boost::shared_ptr<Camera>(c));
}
void epriv::ResourceManager::_addFont(Font* f){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Fonts,f->name(),boost::shared_ptr<Font>(f));
}
void epriv::ResourceManager::_addShader(Shader* s){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Shaders,s->name(),boost::shared_ptr<Shader>(s));
}
void epriv::ResourceManager::_addTexture(Texture* t){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Textures,t->name(),boost::shared_ptr<Texture>(t));
}
void epriv::ResourceManager::_addMaterial(Material* m){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Materials,m->name(),boost::shared_ptr<Material>(m));
}
void epriv::ResourceManager::_addObject(Object* o){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Objects,o->name(),boost::shared_ptr<Object>(o));
}
void epriv::ResourceManager::_addMeshInstance(MeshInstance* m){
	_addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_MeshInstances,m->name(),boost::shared_ptr<MeshInstance>(m));
}
void epriv::ResourceManager::_addMesh(Mesh* m){
    _addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_Meshes,m->name(),boost::shared_ptr<Mesh>(m));
}
void epriv::ResourceManager::_addSoundData(SoundData* s){
    _addToContainer(Core::m_Engine->m_ResourceManager->m_i->m_SoundDatas,s->name(),boost::shared_ptr<SoundData>(s));
}
string epriv::ResourceManager::_buildMeshInstanceName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_MeshInstances,n);}
string epriv::ResourceManager::_buildObjectName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Objects,n);}
string epriv::ResourceManager::_buildTextureName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Textures,n);}
string epriv::ResourceManager::_buildFontName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Fonts,n);}
string epriv::ResourceManager::_buildSceneName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Scenes,n);}
string epriv::ResourceManager::_buildMeshName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n);}
string epriv::ResourceManager::_buildMaterialName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Materials,n);}
string epriv::ResourceManager::_buildCameraName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Cameras,n);}
string epriv::ResourceManager::_buildShaderName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_Shaders,n);}
string epriv::ResourceManager::_buildSoundDataName(string n){return _incrementName(Core::m_Engine->m_ResourceManager->m_i->m_SoundDatas,n);}

void epriv::ResourceManager::_remCamera(string n){_removeFromContainer(Core::m_Engine->m_ResourceManager->m_i->m_Cameras,n);}
void epriv::ResourceManager::_remObject(string n){_removeFromContainer(Core::m_Engine->m_ResourceManager->m_i->m_Objects,n);}

void epriv::ResourceManager::_resizeCameras(uint w,uint h){for(auto c:Core::m_Engine->m_ResourceManager->m_i->m_Cameras){c.second.get()->resize(w,h);}}
uint epriv::ResourceManager::_numScenes(){return Core::m_Engine->m_ResourceManager->m_i->m_Scenes.size();}

void Resources::Settings::enableDynamicMemory(bool b){ epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory = b; }
void Resources::Settings::disableDynamicMemory(){ epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory = false; }









Engine_Window* Resources::getWindow(){ return epriv::Core::m_Engine->m_ResourceManager->m_i->m_Window; }
glm::uvec2 Resources::getWindowSize(){ return epriv::Core::m_Engine->m_ResourceManager->m_i->m_Window->getSize(); }

boost::shared_ptr<Object>& Resources::getObjectPtr(string n){return epriv::Core::m_Engine->m_ResourceManager->m_i->m_Objects.at(n);}
boost::shared_ptr<Camera>& Resources::getCameraPtr(string n){return epriv::Core::m_Engine->m_ResourceManager->m_i->m_Cameras.at(n);}
boost::shared_ptr<Texture>& Resources::getTexturePtr(string n){return epriv::Core::m_Engine->m_ResourceManager->m_i->m_Textures.at(n);}

Scene* Resources::getScene(string n){return (Scene*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Scenes,n));}
Object* Resources::getObject(string n){return (Object*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Objects,n));}
Camera* Resources::getCamera(string n){return (Camera*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Cameras,n));}
Font* Resources::getFont(string n){return (Font*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Fonts,n));}
Texture* Resources::getTexture(string n){return (Texture*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Textures,n));}
Mesh* Resources::getMesh(string n){return (Mesh*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n));}
Material* Resources::getMaterial(string n){return (Material*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Materials,n));}
Shader* Resources::getShader(string n){return (Shader*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Shaders,n));}
ShaderP* Resources::getShaderProgram(string n){return (ShaderP*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_ShaderPrograms,n));}
MeshInstance* Resources::getMeshInstance(string n){return (MeshInstance*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_MeshInstances,n)); }
SoundData* Resources::getSoundData(string n){return (SoundData*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_SoundDatas,n)); }

void Resources::addMesh(string n,string f, CollisionType t, bool b,float threshhold){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,f,t,b,threshhold));
}
void Resources::addMesh(string n,float x,float y,float w,float h,float threshhold){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,x,y,w,h,threshhold));
}
void Resources::addMesh(string n,float w,float h,float threshhold){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,w,h,threshhold));
}
void Resources::addMesh(string f, CollisionType t,float threshhold){string n = f.substr(0, f.size()-4);Resources::addMesh(n,f,t,true,threshhold);}
void Resources::addMesh(string n, unordered_map<string,float>& g, uint w, uint l,float threshhold){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,g,w,l,threshhold));
}

void Resources::addMaterial(string n, string d, string nm , string g, string s,string program){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == "") program = "Deferred";
    Resources::getShaderProgram(program)->addMaterial(n);
}
void Resources::addMaterial(string n, Texture* d, Texture* nm, Texture* g, Texture* s,ShaderP* program){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == nullptr) program = Resources::getShaderProgram("Deferred");
    program->addMaterial(n);
}

void Resources::addShader(string n, string s, ShaderType::Type t, bool b){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Shaders,n,boost::make_shared<Shader>(n,s,t,b));
}
void Resources::addShaderProgram(string n, Shader* v, Shader* f, ShaderRenderPass::Pass s){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, string v, string f, ShaderRenderPass::Pass s){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, Shader* v, string f, ShaderRenderPass::Pass s){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, string v, Shader* f, ShaderRenderPass::Pass s){
    _addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addSoundData(string file,string n,bool music){
	if (n == ""){ n = file; }
	_addToContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_SoundDatas,n,boost::make_shared<SoundData>(file,music));
}

void Resources::removeMesh(string n){_removeFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Meshes,n);}
void Resources::removeMaterial(string n){_removeFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Materials,n);}

void Resources::setCurrentScene(Scene* scene){
	if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene == nullptr){
		epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene = scene;
		return;
	}
	if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene != scene){
        cout << "---- Scene Change started (" << epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene->name() << ") to (" << scene->name() << ") ----" << endl;
        if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory){
            //mark game object resources to minus use count
            for(auto obj:epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene->objects()){ obj.second->suspend(); }
            for(auto obj:epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene->lights()){ obj.second->suspend(); }
			for(auto obj:epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene->cameras()){ obj.second->suspend(); }
        }
		epriv::Core::m_Engine->m_ResourceManager->m_i->m_CurrentScene = scene;
        if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory){
            //mark game object resources to add use count
            for(auto obj:scene->objects()){ obj.second->resume(); }
            for(auto obj:scene->lights()){ obj.second->resume(); }
			for(auto obj:scene->cameras()){ obj.second->resume(); }
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(string s){Resources::setCurrentScene((Scene*)(_getFromContainer(epriv::Core::m_Engine->m_ResourceManager->m_i->m_Scenes,s)));}
