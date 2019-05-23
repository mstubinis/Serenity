#include <boost/make_shared.hpp>
#include <core/engine/Engine.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/Skybox.h>
#include <core/engine/mesh/Mesh.h>
#include <core/MeshInstance.h>
#include <core/Material.h>
#include <core/Camera.h>
#include <core/engine/textures/Texture.h>
#include <core/Font.h>
#include <core/Scene.h>

#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/resources/Handle.h>

#include <ecs/ECS.h>

#include <iostream>

using namespace Engine;
using namespace std;

epriv::ResourceManager* resourceManager;

epriv::ResourceManager::ResourceManager(const char* name,uint width,uint height){
    m_CurrentScene  = nullptr;
    m_Window        = nullptr;
    m_DynamicMemory = false;
    m_Resources     = new ObjectPool<EngineResource>(32768);
    resourceManager = this;
}
epriv::ResourceManager::~ResourceManager(){ 
    SAFE_DELETE(m_Resources);
    SAFE_DELETE(m_Window);
    SAFE_DELETE_VECTOR(m_Scenes);
}
void epriv::ResourceManager::_init(const char* n,uint w,uint h){ 
    m_Window = new Engine_Window(n, w, h);
}

string Engine::Data::reportTime(){
    return epriv::Core::m_Engine->m_DebugManager.reportTime();
}
const double Engine::Resources::dt(){ return epriv::Core::m_Engine->m_DebugManager.dt(); }
Scene* Engine::Resources::getCurrentScene(){ return resourceManager->m_CurrentScene; }

vector<Scene*>& epriv::ResourceManager::scenes() {
    return m_Scenes;
}

bool epriv::ResourceManager::_hasScene(string n){ 
    for (auto& scene : m_Scenes) {
        if (scene->name() == n)
            return true;
    }
    return false;
}
Texture* epriv::ResourceManager::_hasTexture(string n){
    auto& resourcePool = *(m_Resources);
    for(uint i = 0; i < resourcePool.maxEntries(); ++i){
        EngineResource* r = resourcePool.getAsFast<EngineResource>(i+1);
        if(r){ 
            Texture* t = dynamic_cast<Texture*>(r); 
            if(t && t->name() == n){ 
                return t; 
            } 
        }
    }
    return 0;
}
Handle epriv::ResourceManager::_addTexture(Texture* t) {
    return resourceManager->m_Resources->add(t, ResourceType::Texture);
}
Scene& epriv::ResourceManager::_getSceneByID(uint id) {
    return *(m_Scenes[id-1]);
}
void epriv::ResourceManager::_addScene(Scene& s){
    m_Scenes.push_back(&s);
}
uint epriv::ResourceManager::_numScenes(){
    return m_Scenes.size();
}

void Resources::Settings::enableDynamicMemory(bool b){ resourceManager->m_DynamicMemory = b; }
void Resources::Settings::disableDynamicMemory(){ resourceManager->m_DynamicMemory = false; }

Engine_Window& Resources::getWindow(){ return *resourceManager->m_Window; }
glm::uvec2 Resources::getWindowSize(){ return resourceManager->m_Window->getSize(); }

Scene* Resources::getScene(const string& n){ 
    for (auto& scene : resourceManager->m_Scenes) {
        if (scene->name() == n) {
            return scene;
        }
    }
    return nullptr;
}

void Resources::getShader(Handle& h,Shader*& p){ resourceManager->m_Resources->getAs(h,p); }
Shader* Resources::getShader(Handle& h){ Shader* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getSoundData(Handle& h,SoundData*& p){ resourceManager->m_Resources->getAs(h,p); }
SoundData* Resources::getSoundData(Handle& h){ SoundData* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getCamera(Handle& h,Camera*& p){ resourceManager->m_Resources->getAs(h,p); }
Camera* Resources::getCamera(Handle& h){ Camera* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getFont(Handle& h,Font*& p){ resourceManager->m_Resources->getAs(h,p); }
Font* Resources::getFont(Handle& h){ Font* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getTexture(Handle& h,Texture*& p){ resourceManager->m_Resources->getAs(h,p); }
Texture* Resources::getTexture(Handle& h){ Texture* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getMesh(Handle& h,Mesh*& p){ resourceManager->m_Resources->getAs(h,p); }
Mesh* Resources::getMesh(Handle& h){ Mesh* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getMaterial(Handle& h,Material*& p){ resourceManager->m_Resources->getAs(h,p); }
Material* Resources::getMaterial(Handle& h){ Material* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getShaderProgram(Handle& h,ShaderP*& p){ resourceManager->m_Resources->getAs(h,p); }
ShaderP* Resources::getShaderProgram(Handle& h){ ShaderP* p; resourceManager->m_Resources->getAs(h,p); return p; }

Handle Resources::addFont(const string& filename){
    return resourceManager->m_Resources->add(new Font(filename),ResourceType::Font);
}


vector<Handle> Resources::loadMesh(string fileOrData, float threshhold) {
    MeshRequest request = MeshRequest(fileOrData, threshhold);
    request.request();
    vector<Handle> handles;
    for (auto& part : request.parts) {
        handles.push_back(part.handle);
    }
    return handles;
}
vector<Handle> Resources::loadMeshAsync(string fileOrData, float threshhold) {
    MeshRequest* request = new MeshRequest(fileOrData, threshhold); //to extend the lifetime to the threads, we manually delete later
    request->requestAsync();
    vector<Handle> handles;
    for (auto& part : request->parts) {
        handles.push_back(part.handle);
    }
    return handles;
}



Handle Resources::addTexture(string file,ImageInternalFormat::Format internFormat,bool mipmaps){
    return resourceManager->m_Resources->add(new Texture(file,mipmaps,internFormat),ResourceType::Texture);
}

Handle Resources::addMaterial(string name, string diffuse, string normal,string glow, string specular){
    Material* material = new Material(name,diffuse,normal,glow,specular);
    return resourceManager->m_Resources->add(material,ResourceType::Material);
}
Handle Resources::addMaterial(string name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular){
    Material* material = new Material(name,diffuse,normal,glow,specular);
    return resourceManager->m_Resources->add(material,ResourceType::Material);
}

Handle Resources::addShader(string fileOrData, ShaderType::Type type, bool fromFile){
    return resourceManager->m_Resources->add(new Shader(fileOrData,type,fromFile),ResourceType::Shader);
}

Handle Resources::addShaderProgram(string n, Shader& v, Shader& f){
    return resourceManager->m_Resources->add(new ShaderP(n,v,f),ResourceType::ShaderProgram);
}
Handle Resources::addShaderProgram(string n, Handle& v, Handle& f){
    Shader* vS = resourceManager->m_Resources->getAsFast<Shader>(v);
    Shader* fS = resourceManager->m_Resources->getAsFast<Shader>(f);
    return resourceManager->m_Resources->add(new ShaderP(n,*vS,*fS),ResourceType::ShaderProgram);
}

Handle Resources::addSoundData(string file,string n,bool music){
    return resourceManager->m_Resources->add(new SoundData(file,music),ResourceType::SoundData);
}

void Resources::setCurrentScene(Scene* newScene){
    Scene* oldScene = resourceManager->m_CurrentScene;

    epriv::EventSceneChanged e(oldScene, newScene);
    Event ev;
    ev.eventSceneChanged = e;
    ev.type = EventType::SceneChanged;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    
    if(!oldScene){
        cout << "---- Initial scene set to: " << newScene->name() << endl;
        resourceManager->m_CurrentScene = newScene; 
        epriv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        return;
    }
    if(oldScene != newScene){
        cout << "---- Scene Change started (" << oldScene->name() << ") to (" << newScene->name() << ") ----" << endl;
        if(epriv::Core::m_Engine->m_ResourceManager.m_DynamicMemory){
            //mark game object resources to minus use count
        }
        epriv::InternalScenePublicInterface::GetECS(*oldScene).onSceneLeft(*oldScene);
        resourceManager->m_CurrentScene = newScene;    
        epriv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        if(resourceManager->m_DynamicMemory){
            //mark game object resources to add use count
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(const string& s){ Resources::setCurrentScene(Resources::getScene(s)); }
