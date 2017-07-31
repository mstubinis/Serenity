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

float Detail::ResourceManagement::m_DeltaTime = 1;
float Detail::ResourceManagement::m_ApplicationTime = 0;
Engine_Window* Detail::ResourceManagement::m_Window;
Scene* Detail::ResourceManagement::m_CurrentScene;
boost::weak_ptr<Camera> Detail::ResourceManagement::m_ActiveCamera;
bool Detail::ResourceManagement::m_DynamicMemory = false;

std::unordered_map<std::string,boost::shared_ptr<MeshInstance>> Detail::ResourceManagement::m_RenderedItems;
std::unordered_map<std::string,boost::shared_ptr<Scene>> Detail::ResourceManagement::m_Scenes;
std::unordered_map<std::string,boost::shared_ptr<SoundEffectBasic>> Detail::ResourceManagement::m_Sounds;
std::unordered_map<std::string,boost::shared_ptr<Object>> Detail::ResourceManagement::m_Objects;
std::unordered_map<std::string,boost::shared_ptr<Camera>> Detail::ResourceManagement::m_Cameras;
std::unordered_map<std::string,boost::shared_ptr<Font>> Detail::ResourceManagement::m_Fonts;
std::unordered_map<std::string,boost::shared_ptr<Mesh>> Detail::ResourceManagement::m_Meshes;
std::unordered_map<std::string,boost::shared_ptr<Texture>> Detail::ResourceManagement::m_Textures;
std::unordered_map<std::string,boost::shared_ptr<Material>> Detail::ResourceManagement::m_Materials;
std::unordered_map<std::string,boost::shared_ptr<Shader>> Detail::ResourceManagement::m_Shaders;
std::unordered_map<std::string,boost::shared_ptr<ShaderP>> Detail::ResourceManagement::m_ShaderPrograms;

void Resources::Detail::ResourceManagement::destruct(){
    for (auto it = m_RenderedItems.begin();it != m_RenderedItems.end(); ++it )   it->second.reset();
    for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )                 it->second.reset();
    for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )             it->second.reset();
    for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                   it->second.reset();
    for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )           it->second.reset();
    for (auto it = m_ShaderPrograms.begin();it != m_ShaderPrograms.end(); ++it ) it->second.reset();
    for (auto it = m_Shaders.begin();it != m_Shaders.end(); ++it )               it->second.reset();
    for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )               it->second.reset();
    for (auto it = m_Cameras.begin();it != m_Cameras.end(); ++it )               it->second.reset();
    for (auto it = m_Sounds.begin();it != m_Sounds.end(); ++it )                 it->second.reset();
    for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )                 it->second.reset();
    SAFE_DELETE(Detail::ResourceManagement::m_Window);
}
Engine_Window* Resources::getWindow(){ return Detail::ResourceManagement::m_Window; }
sf::Vector2u Resources::getWindowSize(){ return Detail::ResourceManagement::m_Window->getSize(); }
Camera* Resources::getActiveCamera(){ return Detail::ResourceManagement::m_ActiveCamera.lock().get(); }
boost::weak_ptr<Camera>& Resources::getActiveCameraPtr(){ return Detail::ResourceManagement::m_ActiveCamera; }
void Resources::setActiveCamera(Camera* c){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras.at(c->name()); }
void Resources::setActiveCamera(std::string name){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras.at(name); }

boost::shared_ptr<Object>& Resources::getObjectPtr(std::string n){return Detail::ResourceManagement::m_Objects.at(n);}
boost::shared_ptr<Camera>& Resources::getCameraPtr(std::string n){return Detail::ResourceManagement::m_Cameras.at(n);}
boost::shared_ptr<Texture>& Resources::getTexturePtr(std::string n){return Detail::ResourceManagement::m_Textures.at(n);}

Scene* Resources::getScene(std::string n){return static_cast<Scene*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Scenes,n));}
SoundEffectBasic* Resources::getSound(std::string n){return static_cast<SoundEffectBasic*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Sounds,n));}
Object* Resources::getObject(std::string n){return static_cast<Object*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Objects,n));}
Camera* Resources::getCamera(std::string n){return static_cast<Camera*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Cameras,n));}
Font* Resources::getFont(std::string n){return static_cast<Font*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Fonts,n));}
Texture* Resources::getTexture(std::string n){return static_cast<Texture*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Textures,n));}
Mesh* Resources::getMesh(std::string n){return static_cast<Mesh*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Meshes,n));}
Material* Resources::getMaterial(std::string n){return static_cast<Material*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Materials,n));}
Shader* Resources::getShader(std::string n){return static_cast<Shader*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Shaders,n));}
ShaderP* Resources::getShaderProgram(std::string n){return static_cast<ShaderP*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_ShaderPrograms,n));}
MeshInstance* Resources::getRenderedItem(std::string n){return static_cast<MeshInstance*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_RenderedItems,n)); }

void Resources::addMesh(std::string n,std::string f, CollisionType t, bool b,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,f,t,b,threshhold));
}
void Resources::addMesh(std::string n,float x,float y,float w,float h,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,x,y,w,h,threshhold));
}
void Resources::addMesh(std::string n,float w,float h,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,w,h,threshhold));
}
void Resources::addMesh(std::string f, CollisionType t,float threshhold){std::string n = f.substr(0, f.size()-4);Resources::addMesh(n,f,t,true,threshhold);}
void Resources::addMesh(std::string n, std::unordered_map<std::string,float>& g, uint w, uint l,float threshhold){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Meshes,n,boost::make_shared<Mesh>(n,g,w,l,threshhold));
}

void Resources::addMaterial(std::string n, std::string d, std::string nm , std::string g, std::string s,std::string program){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == "") 
		program = "Deferred";
    Resources::getShaderProgram(program)->addMaterial(n);
}
void Resources::addMaterial(std::string n, Texture* d, Texture* nm, Texture* g, Texture* s,ShaderP* program){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == nullptr) 
		program = Resources::getShaderProgram("Deferred");
    program->addMaterial(n);
}

void Resources::addShader(std::string n, std::string s, SHADER_TYPE t, bool b){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Shaders,n,boost::make_shared<Shader>(n,s,t,b));
}
void Resources::addShaderProgram(std::string n, Shader* v, Shader* f, SHADER_PIPELINE_STAGE s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(std::string n, std::string v, std::string f, SHADER_PIPELINE_STAGE s){
    Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(std::string n, Shader* v, std::string f, SHADER_PIPELINE_STAGE s){
	Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(std::string n, std::string v, Shader* f, SHADER_PIPELINE_STAGE s){
	Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}


void Resources::addSound(std::string n, std::string f, bool b){
    if(b){Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Sounds,n,boost::make_shared<SoundEffect>(n,f));}
    else{Detail::ResourceManagement::_addToContainer(Detail::ResourceManagement::m_Sounds,n,boost::make_shared<SoundMusic>(n,f));}
}
void Resources::addSoundAsEffect(std::string n, std::string f){ addSound(n,f,true); }
void Resources::addSoundAsMusic(std::string n, std::string f){ addSound(n,f,false); }

SoundMusic* Resources::getSoundAsMusic(std::string n){return static_cast<SoundMusic*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Sounds,n));}
SoundEffect* Resources::getSoundAsEffect(std::string n){ return static_cast<SoundEffect*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Sounds,n));}

void Resources::removeMesh(std::string n){Detail::ResourceManagement::_removeFromContainer(Detail::ResourceManagement::m_Meshes,n);}
void Resources::removeMaterial(std::string n){Detail::ResourceManagement::_removeFromContainer(Detail::ResourceManagement::m_Materials,n);}
void Resources::removeSound(std::string n){Detail::ResourceManagement::_removeFromContainer(Detail::ResourceManagement::m_Sounds,n);}

void Resources::initResources(){
    //add a basic cube mesh
    #pragma region MeshData
    std::string cubeMesh =  "v -1.0000 -1.0000 1.0000\n"
                            "v -1.0000 1.0000 1.0000\n"
                            "v -1.0000 -1.0000 -1.0000\n"
                            "v -1.0000 1.0000 -1.0000\n"
                            "v 1.0000 -1.0000 1.0000\n"
                            "v 1.0000 1.0000 1.0000\n"
                            "v 1.0000 -1.0000 -1.0000\n"
                            "v 1.0000 1.0000 -1.0000\n"
                            "vn -1.0000 0.0000 0.0000\n"
                            "vn 0.0000 0.0000 -1.0000\n"
                            "vn 1.0000 0.0000 0.0000\n"
                            "vn 0.0000 0.0000 1.0000\n"
                            "vn 0.0000 -1.0000 0.0000\n"
                            "vn 0.0000 1.0000 0.0000\n"
                            "f 4/1 1/1 2/1\n"
                            "f 8/2 3/2 4/2\n"
                            "f 6/3 7/3 8/3\n"
                            "f 2/4 5/4 6/4\n"
                            "f 3/5 5/5 1/5\n"
                            "f 8/6 2/6 6/6\n"
                            "f 4/1 3/1 1/1\n"
                            "f 8/2 7/2 3/2\n"
                            "f 6/3 5/3 7/3\n"
                            "f 2/4 1/4 5/4\n"
                            "f 3/5 7/5 5/5\n"
                            "f 8/6 4/6 2/6\n";
    #pragma endregion
    //addMesh("Cube",cubeMesh,COLLISION_TYPE_NONE,false);


	Shader* fullscreenVertexShader = new Shader("vert_fullscreenQuad",Engine::Shaders::Detail::ShadersManagement::fullscreen_quad_vertex,SHADER_TYPE_VERTEX,false);
	Shader* fxaa = new Shader("frag_fxaa",Engine::Shaders::Detail::ShadersManagement::fxaa_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* vertexBasic = new Shader("vert_basic",Engine::Shaders::Detail::ShadersManagement::vertex_basic,SHADER_TYPE_VERTEX,false);
	Shader* vertexHUD = new Shader("vert_hud",Engine::Shaders::Detail::ShadersManagement::vertex_hud,SHADER_TYPE_VERTEX,false);
	Shader* vertexSkybox = new Shader("vert_skybox",Engine::Shaders::Detail::ShadersManagement::vertex_skybox,SHADER_TYPE_VERTEX,false);
	Shader* deferredFrag = new Shader("deferred_frag",Engine::Shaders::Detail::ShadersManagement::deferred_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* deferredFragHUD = new Shader("deferred_frag_hud",Engine::Shaders::Detail::ShadersManagement::deferred_frag_hud,SHADER_TYPE_FRAGMENT,false);
	Shader* deferredFragSkybox = new Shader("deferred_frag_skybox",Engine::Shaders::Detail::ShadersManagement::deferred_frag_skybox,SHADER_TYPE_FRAGMENT,false);
	Shader* copyDepth = new Shader("copy_depth_frag",Engine::Shaders::Detail::ShadersManagement::copy_depth_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* ssao = new Shader("ssao_frag",Engine::Shaders::Detail::ShadersManagement::ssao_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* hdr = new Shader("hdr_frag",Engine::Shaders::Detail::ShadersManagement::hdr_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* blur = new Shader("blur_frag",Engine::Shaders::Detail::ShadersManagement::blur_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* godrays = new Shader("godrays_frag",Engine::Shaders::Detail::ShadersManagement::godRays_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* edge = new Shader("edge_frag",Engine::Shaders::Detail::ShadersManagement::edge_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* finalFrag = new Shader("final_frag",Engine::Shaders::Detail::ShadersManagement::final_frag,SHADER_TYPE_FRAGMENT,false);
	Shader* lightingFrag = new Shader("lighting_frag",Engine::Shaders::Detail::ShadersManagement::lighting_frag,SHADER_TYPE_FRAGMENT,false);

    addShaderProgram("Deferred",vertexBasic,deferredFrag,SHADER_PIPELINE_STAGE_GEOMETRY);
    addShaderProgram("Deferred_HUD",vertexHUD,deferredFragHUD,SHADER_PIPELINE_STAGE_GEOMETRY);
    addShaderProgram("Deferred_GodsRays",fullscreenVertexShader,godrays,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_Blur",fullscreenVertexShader,blur,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_HDR",fullscreenVertexShader,hdr,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_SSAO",fullscreenVertexShader,ssao,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_Edge",fullscreenVertexShader,edge,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_Final",fullscreenVertexShader,finalFrag,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_FXAA",fullscreenVertexShader,fxaa,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_Skybox",vertexSkybox,deferredFragSkybox,SHADER_PIPELINE_STAGE_GEOMETRY);
    addShaderProgram("Copy_Depth",fullscreenVertexShader,copyDepth,SHADER_PIPELINE_STAGE_POSTPROCESSING);
    addShaderProgram("Deferred_Light",fullscreenVertexShader,lightingFrag,SHADER_PIPELINE_STAGE_LIGHTING);

    addMaterial("Default","","","","","Deferred");

    addMesh("Plane",1.0f,1.0f);
}
void Resources::initRenderingContexts(){
    for(auto shaderProgram:Detail::ResourceManagement::m_ShaderPrograms) shaderProgram.second.get()->initRenderingContext();
}
void Resources::cleanupRenderingContexts(){
    for(auto shaderProgram:Detail::ResourceManagement::m_ShaderPrograms) shaderProgram.second.get()->cleanupRenderingContext();
}
void Resources::setCurrentScene(Scene* scene){ 
    if(Detail::ResourceManagement::m_CurrentScene != scene){
        std::cout << "---- Scene Change started (" << Detail::ResourceManagement::m_CurrentScene->name() << ") to (" << scene->name() << ") ----" << std::endl;
        if(Resources::Detail::ResourceManagement::m_DynamicMemory){
            //mark game object resources to minus use count
            for(auto obj:Detail::ResourceManagement::m_CurrentScene->objects()){
                obj.second->suspend();
            }
            for(auto obj:Detail::ResourceManagement::m_CurrentScene->lights()){
                obj.second->suspend();
            }
        }
        Detail::ResourceManagement::m_CurrentScene = scene;
        if(Resources::Detail::ResourceManagement::m_DynamicMemory){
            //mark game object resources to add use count
            for(auto obj:scene->objects()){
                obj.second->resume();
            }
            for(auto obj:scene->lights()){
                obj.second->resume();
            }
        }
        std::cout << "-------- Scene Change ended --------" << std::endl;
    }
}
void Resources::setCurrentScene(std::string s){
    Resources::setCurrentScene(static_cast<Scene*>(Detail::ResourceManagement::_getFromContainer(Detail::ResourceManagement::m_Scenes,s))); 
}