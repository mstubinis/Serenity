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

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/make_shared.hpp>

#include <SFML/Graphics.hpp>

using namespace Engine::Resources;

ENGINE_RENDERING_API Detail::ResourceManagement::m_RenderingAPI;
float Detail::ResourceManagement::m_DeltaTime = 1;
Engine_Window* Detail::ResourceManagement::m_Window;
Scene* Detail::ResourceManagement::m_CurrentScene;
boost::weak_ptr<Camera> Detail::ResourceManagement::m_ActiveCamera;

std::unordered_map<std::string,boost::shared_ptr<Object>> Detail::ResourceManagement::m_Objects;
std::unordered_map<std::string,boost::shared_ptr<Camera>> Detail::ResourceManagement::m_Cameras;
std::unordered_map<std::string,boost::shared_ptr<Font>> Detail::ResourceManagement::m_Fonts;
std::unordered_map<std::string,boost::shared_ptr<Texture>> Detail::ResourceManagement::m_Textures;
std::unordered_map<std::string,boost::shared_ptr<Scene>> Detail::ResourceManagement::m_Scenes;
std::unordered_map<std::string,boost::shared_ptr<Mesh>> Detail::ResourceManagement::m_Meshes;
std::unordered_map<std::string,boost::shared_ptr<Material>> Detail::ResourceManagement::m_Materials;
std::unordered_map<std::string,boost::shared_ptr<ShaderP>> Detail::ResourceManagement::m_Shaders;
std::unordered_map<std::string,boost::shared_ptr<SoundEffectBasic>> Detail::ResourceManagement::m_Sounds;

void Engine::Resources::Detail::ResourceManagement::destruct(){
    for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )               
        it->second.reset();
    for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )           
        it->second.reset();
    for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                 
        it->second.reset();
    for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )         
        it->second.reset();
    for (auto it = m_Shaders.begin();it != m_Shaders.end(); ++it )             
        it->second.reset();
    for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )             
        it->second.reset();
    for (auto it = m_Cameras.begin();it != m_Cameras.end(); ++it )             
        it->second.reset();
    for (auto it = m_Sounds.begin();it != m_Sounds.end(); ++it )               
        it->second.reset();
    for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )               
        it->second.reset();
    SAFE_DELETE(Detail::ResourceManagement::m_Window);
}
Engine_Window* Engine::Resources::getWindow(){ return Detail::ResourceManagement::m_Window; }
sf::Vector2u Engine::Resources::getWindowSize(){ return Detail::ResourceManagement::m_Window->getSize(); }
Camera* Engine::Resources::getActiveCamera(){ return Detail::ResourceManagement::m_ActiveCamera.lock().get(); }
boost::weak_ptr<Camera>& Engine::Resources::getActiveCameraPtr(){ return Detail::ResourceManagement::m_ActiveCamera; }
void Engine::Resources::setActiveCamera(Camera* c){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras[c->getName()]; }
void Engine::Resources::setActiveCamera(std::string name){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras[name]; }

void Engine::Resources::addMesh(std::string name,std::string file, COLLISION_TYPE type, bool fromFile){
    if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name))
        return;
    Detail::ResourceManagement::m_Meshes[name] = boost::make_shared<Mesh>(file,type,fromFile);
}
void Engine::Resources::addMesh(std::string name,float x,float y,float w,float h){
    if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name))
        return;
    Detail::ResourceManagement::m_Meshes[name] = boost::make_shared<Mesh>(x,y,w,h);
}
void Engine::Resources::addMesh(std::string file, COLLISION_TYPE type){
    std::string name = file.substr(0, file.size()-4);
    Engine::Resources::addMesh(name,file,type);
}
void Engine::Resources::addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length){
    if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name))
        return;
    Detail::ResourceManagement::m_Meshes[name] = boost::make_shared<Mesh>(grid,width,length);
}

void Engine::Resources::addMaterial(std::string name, std::string diffuse, std::string normal , std::string glow, std::string specular){
    if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
        return;
    Detail::ResourceManagement::m_Materials[name] = boost::make_shared<Material>(diffuse,normal,glow,specular);
}
void Engine::Resources::addMaterial(std::string name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular){
    if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
        return;
    Detail::ResourceManagement::m_Materials[name] = boost::make_shared<Material>(diffuse,normal,glow,specular);
}

void Engine::Resources::addShader(std::string name, std::string vShader, std::string fShader, bool fromFile){
    if (Detail::ResourceManagement::m_Shaders.size() > 0 && Detail::ResourceManagement::m_Shaders.count(name))
        return;
    Detail::ResourceManagement::m_Shaders[name] = boost::make_shared<ShaderP>(name,vShader,fShader,fromFile);
}

void Engine::Resources::addSound(std::string name, std::string file, bool asEffect){
    if (Detail::ResourceManagement::m_Sounds.size() > 0 && Detail::ResourceManagement::m_Sounds.count(name))
        return;
    if(asEffect) Detail::ResourceManagement::m_Sounds[name] = boost::make_shared<SoundEffect>(file);
    else         Detail::ResourceManagement::m_Sounds[name] = boost::make_shared<SoundMusic>(file);
}
void Engine::Resources::addSoundAsEffect(std::string name, std::string file){ addSound(name,file,true); }
void Engine::Resources::addSoundAsMusic(std::string name, std::string file){ addSound(name,file,false); }

SoundMusic* Engine::Resources::getSoundAsMusic(std::string n){ 
    if(!Detail::ResourceManagement::m_Sounds.count(n))
        return nullptr;
    return static_cast<SoundMusic*>(Detail::ResourceManagement::m_Sounds[n].get()); 
}
SoundEffect* Engine::Resources::getSoundAsEffect(std::string n){ 
    if(!Detail::ResourceManagement::m_Sounds.count(n))
        return nullptr;
    return static_cast<SoundEffect*>(Detail::ResourceManagement::m_Sounds[n].get()); 
}

void Engine::Resources::removeMesh(std::string name){
    if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name)){
        Detail::ResourceManagement::m_Meshes[name].reset();
        Detail::ResourceManagement::m_Meshes.erase(name);
    }
}
void Engine::Resources::removeMaterial(std::string name){
    if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name)){
        Detail::ResourceManagement::m_Materials[name].reset();
        Detail::ResourceManagement::m_Materials.erase(name);
    }
}
void Engine::Resources::removeSound(std::string name){
    if (Detail::ResourceManagement::m_Sounds.size() > 0 && Detail::ResourceManagement::m_Sounds.count(name)){
        Detail::ResourceManagement::m_Sounds[name].reset();
        Detail::ResourceManagement::m_Sounds.erase(name);
    }
}


void Engine::Resources::initResources(){
	//add a basic cube mesh
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
	//addMesh("Cube",cubeMesh,COLLISION_TYPE_NONE,false);

    addShader("Deferred","data/Shaders/vert.glsl","data/Shaders/deferred_frag.glsl");
    addShader("Deferred_HUD","data/Shaders/vert_HUD.glsl","data/Shaders/deferred_frag_HUD.glsl");
    addShader("Deferred_GodsRays","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_rays_frag.glsl");
    addShader("Deferred_Blur","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_blur_frag.glsl");
    addShader("Deferred_HDR","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_hdr_frag.glsl");
    addShader("Deferred_SSAO","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_ssao_frag.glsl");
    addShader("Deferred_Edge","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_edge_frag.glsl");
    addShader("Deferred_Final","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_final_frag.glsl");
    addShader("Deferred_Skybox","data/Shaders/vert_skybox.glsl","data/Shaders/deferred_frag_skybox.glsl");
    addShader("Copy_Depth","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/copy_depth_frag.glsl");
    addShader("Deferred_Light","data/Shaders/vert_fullscreenQuad.glsl","data/Shaders/deferred_lighting_frag.glsl");

	addMaterial("Default",nullptr,nullptr,nullptr);

    Resources::Detail::ResourceManagement::m_Meshes["Plane"] = boost::make_shared<Mesh>(1.0f,1.0f);
}
void Engine::Resources::initRenderingContexts(unsigned int api){
    for(auto mesh:Detail::ResourceManagement::m_Meshes)
        mesh.second.get()->initRenderingContext(api);
    for(auto shader:Detail::ResourceManagement::m_Shaders)
        shader.second.get()->initRenderingContext(api);
}
void Engine::Resources::cleanupRenderingContexts(unsigned int api){
    for(auto mesh:Detail::ResourceManagement::m_Meshes)
        mesh.second.get()->cleanupRenderingContext(api);
    for(auto shader:Detail::ResourceManagement::m_Shaders)
        shader.second.get()->cleanupRenderingContext(api);
}
void Engine::Resources::setCurrentScene(Scene* s){ 
    if(Detail::ResourceManagement::m_CurrentScene == s) return;

    Scene* previousScene = Detail::ResourceManagement::m_CurrentScene;
    for(auto obj:previousScene->getObjects()){
        ObjectDynamic* dynamicObj = dynamic_cast<ObjectDynamic*>(obj.second);
        if(dynamicObj != NULL){
            Engine::Physics::removeRigidBody(dynamicObj);
        }
    }

    Detail::ResourceManagement::m_CurrentScene = s;

    for(auto obj:s->getObjects()){
        ObjectDynamic* dynamicObj = dynamic_cast<ObjectDynamic*>(obj.second);
        if(dynamicObj != NULL){
            Engine::Physics::addRigidBody(dynamicObj);
        }
    }
}
void Engine::Resources::setCurrentScene(std::string s){ Engine::Resources::setCurrentScene(Detail::ResourceManagement::m_Scenes[s].get()); }