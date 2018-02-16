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
using namespace Engine::Resources;
using namespace std;

EngineTime Detail::ResourceManagement::m_Time = EngineTime();

Engine_Window* Detail::ResourceManagement::m_Window;
Scene* Detail::ResourceManagement::m_CurrentScene;
boost::weak_ptr<Camera> Detail::ResourceManagement::m_ActiveCamera;
bool Detail::ResourceManagement::m_DynamicMemory = false;

unordered_map<string,boost::shared_ptr<MeshInstance>> Detail::ResourceManagement::m_MeshInstances;
unordered_map<string,boost::shared_ptr<Scene>> Detail::ResourceManagement::m_Scenes;
unordered_map<string,boost::shared_ptr<Object>> Detail::ResourceManagement::m_Objects;
unordered_map<string,boost::shared_ptr<Camera>> Detail::ResourceManagement::m_Cameras;
unordered_map<string,boost::shared_ptr<Font>> Detail::ResourceManagement::m_Fonts;
unordered_map<string,boost::shared_ptr<Mesh>> Detail::ResourceManagement::m_Meshes;
unordered_map<string,boost::shared_ptr<Texture>> Detail::ResourceManagement::m_Textures;
unordered_map<string,boost::shared_ptr<Material>> Detail::ResourceManagement::m_Materials;
unordered_map<string,boost::shared_ptr<Shader>> Detail::ResourceManagement::m_Shaders;
unordered_map<string,boost::shared_ptr<ShaderP>> Detail::ResourceManagement::m_ShaderPrograms;

void Resources::Detail::ResourceManagement::destruct(){
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
    SAFE_DELETE(Detail::ResourceManagement::m_Window);
}
Engine_Window* Resources::getWindow(){ return Detail::ResourceManagement::m_Window; }
sf::Vector2u Resources::getWindowSize(){ return Detail::ResourceManagement::m_Window->getSize(); }
Camera* Resources::getActiveCamera(){ return Detail::ResourceManagement::m_ActiveCamera.lock().get(); }
boost::weak_ptr<Camera>& Resources::getActiveCameraPtr(){ return Detail::ResourceManagement::m_ActiveCamera; }
void Resources::setActiveCamera(Camera* c){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras.at(c->name()); }
void Resources::setActiveCamera(string name){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras.at(name); }

boost::shared_ptr<Object>& Resources::getObjectPtr(string n){return Detail::ResourceManagement::m_Objects.at(n);}
boost::shared_ptr<Camera>& Resources::getCameraPtr(string n){return Detail::ResourceManagement::m_Cameras.at(n);}
boost::shared_ptr<Texture>& Resources::getTexturePtr(string n){return Detail::ResourceManagement::m_Textures.at(n);}

Scene* Resources::getScene(string n){return static_cast<Scene*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Scenes,n));}
Object* Resources::getObject(string n){return static_cast<Object*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Objects,n));}
Camera* Resources::getCamera(string n){return static_cast<Camera*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Cameras,n));}
Font* Resources::getFont(string n){return static_cast<Font*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Fonts,n));}
Texture* Resources::getTexture(string n){return static_cast<Texture*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Textures,n));}
Mesh* Resources::getMesh(string n){return static_cast<Mesh*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Meshes,n));}
Material* Resources::getMaterial(string n){return static_cast<Material*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Materials,n));}
Shader* Resources::getShader(string n){return static_cast<Shader*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Shaders,n));}
ShaderP* Resources::getShaderProgram(string n){return static_cast<ShaderP*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_ShaderPrograms,n));}
MeshInstance* Resources::getMeshInstance(string n){return static_cast<MeshInstance*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_MeshInstances,n)); }

void Resources::addMesh(string n,string f, CollisionType t, bool b,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,f,t,b,threshhold));
}
void Resources::addMesh(string n,float x,float y,float w,float h,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,x,y,w,h,threshhold));
}
void Resources::addMesh(string n,float w,float h,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,w,h,threshhold));
}
void Resources::addMesh(string f, CollisionType t,float threshhold){string n = f.substr(0, f.size()-4);Resources::addMesh(n,f,t,true,threshhold);}
void Resources::addMesh(string n, unordered_map<string,float>& g, uint w, uint l,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,g,w,l,threshhold));
}

void Resources::addMaterial(string n, string d, string nm , string g, string s,string program){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == "") program = "Deferred";
    Resources::getShaderProgram(program)->addMaterial(n);
}
void Resources::addMaterial(string n, Texture* d, Texture* nm, Texture* g, Texture* s,ShaderP* program){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == nullptr) program = Resources::getShaderProgram("Deferred");
    program->addMaterial(n);
}

void Resources::addShader(string n, string s, ShaderType::Type t, bool b){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Shaders,n,boost::make_shared<Shader>(n,s,t,b));
}
void Resources::addShaderProgram(string n, Shader* v, Shader* f, ShaderRenderPass::Pass s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, string v, string f, ShaderRenderPass::Pass s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, Shader* v, string f, ShaderRenderPass::Pass s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, string v, Shader* f, ShaderRenderPass::Pass s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}


void Resources::removeMesh(string n){Detail::ResourceManagement::_removeFromContainer(Detail::ResourceManagement::m_Meshes,n);}
void Resources::removeMaterial(string n){Detail::ResourceManagement::_removeFromContainer(Detail::ResourceManagement::m_Materials,n);}

void Resources::initResources(){
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
    addMesh("Cube",cubeMesh,CollisionType::None,false);

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


    addShaderProgram("Deferred",vertexBasic,deferredFrag,ShaderRenderPass::Geometry);
    addShaderProgram("Deferred_HUD",vertexHUD,deferredFragHUD,ShaderRenderPass::Geometry);
    addShaderProgram("Deferred_GodsRays",fullscreenVertexShader,godrays,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Blur",fullscreenVertexShader,blur,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_HDR",fullscreenVertexShader,hdr,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_SSAO",fullscreenVertexShader,ssao,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Final",fullscreenVertexShader,finalFrag,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_FXAA",fullscreenVertexShader,fxaa,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Skybox",vertexSkybox,deferredFragSkybox,ShaderRenderPass::Geometry);
	addShaderProgram("Deferred_Skybox_Fake",vertexSkybox,deferredFragSkyboxFake,ShaderRenderPass::Geometry);
    addShaderProgram("Copy_Depth",fullscreenVertexShader,copyDepth,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Light",fullscreenVertexShader,lightingFrag,ShaderRenderPass::Lighting);
    addShaderProgram("Deferred_Light_GI",fullscreenVertexShader,lightingFragGI,ShaderRenderPass::Lighting);
    addShaderProgram("Cubemap_Convolude",vertexSkybox,cubemapConvolude,ShaderRenderPass::Postprocess);
    addShaderProgram("Cubemap_Prefilter_Env",vertexSkybox,cubemapPrefilterEnv,ShaderRenderPass::Postprocess);
    addShaderProgram("BRDF_Precompute_CookTorrance",fullscreenVertexShader,brdfPrecompute,ShaderRenderPass::Postprocess);
    addShaderProgram("Greyscale_Frag",fullscreenVertexShader,greyscale,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Edge_Canny_Blur",fullscreenVertexShader,edgeCannyBlur,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_Edge_Canny",fullscreenVertexShader,edgeCannyFrag,ShaderRenderPass::Postprocess);
    addShaderProgram("Stencil_Pass",fullscreenVertexShader,stencilPass,ShaderRenderPass::Postprocess);

	addShaderProgram("Deferred_SMAA_1_Stencil",smaa_vert_1,smaa_frag_1_stencil,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_SMAA_1",smaa_vert_1,smaa_frag_1,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_SMAA_2",smaa_vert_2,smaa_frag_2,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_SMAA_3",smaa_vert_3,smaa_frag_3,ShaderRenderPass::Postprocess);
    addShaderProgram("Deferred_SMAA_4",smaa_vert_4,smaa_frag_4,ShaderRenderPass::Postprocess);

    Texture* brdfCook = new Texture("BRDFCookTorrance",512,512,ImageInternalFormat::RG16F,ImagePixelFormat::RG,ImagePixelType::FLOAT,GL_TEXTURE_2D,1.0f);
    brdfCook->setWrapping(TextureWrap::ClampToEdge);

    addMaterial("Default","","","","","Deferred");

    addMesh("Plane",1.0f,1.0f);
}
void Resources::setCurrentScene(Scene* scene){ 
    if(Detail::ResourceManagement::m_CurrentScene != scene){
        cout << "---- Scene Change started (" << Detail::ResourceManagement::m_CurrentScene->name() << ") to (" << scene->name() << ") ----" << endl;
        if(Resources::Detail::ResourceManagement::m_DynamicMemory){
            //mark game object resources to minus use count
            for(auto obj:Detail::ResourceManagement::m_CurrentScene->objects()){ obj.second->suspend(); }
            for(auto obj:Detail::ResourceManagement::m_CurrentScene->lights()){ obj.second->suspend(); }
			//suspend cameras too?
        }
        Detail::ResourceManagement::m_CurrentScene = scene;
        if(Resources::Detail::ResourceManagement::m_DynamicMemory){
            //mark game object resources to add use count
            for(auto obj:scene->objects()){ obj.second->resume(); }
            for(auto obj:scene->lights()){ obj.second->resume(); }
			//resume cameras too?
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(string s){
    Resources::setCurrentScene(static_cast<Scene*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Scenes,s))); 
}

void Resources::addSound(string file,string name){
	if (name == ""){
		if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
			Sound::Detail::SoundManagement::addSoundDataFromFile(file,file,false);
		}
	}
	else{
		if(!Sound::Detail::SoundManagement::m_SoundData.count(name)){
			Sound::Detail::SoundManagement::addSoundDataFromFile(name,file,false);
		}
	}
}