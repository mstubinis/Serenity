#include <boost/make_shared.hpp>
#include <core/engine/Engine.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/scene/Scene.h>

#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/resources/Handle.h>

#include <core/engine/sounds/SoundData.h>

#include <ecs/ECS.h>
#include <iostream>

using namespace Engine;
using namespace std;

epriv::ResourceManager* resourceManager;

epriv::ResourceManager::ResourceManager(const char* name, const uint& width, const uint& height){
    m_CurrentScene     = nullptr;
    m_Window           = nullptr;
    m_DynamicMemory    = false;
    m_Resources        = new ObjectPool<EngineResource>(32768);
    resourceManager    = this;
}
epriv::ResourceManager::~ResourceManager(){ 
    SAFE_DELETE(m_Resources);
    SAFE_DELETE(m_Window);
    SAFE_DELETE_VECTOR(m_Scenes);
}
void epriv::ResourceManager::_init(const char* name, const uint& width, const uint& height){
    m_Window = new Engine_Window(name, width, height);
}

string Engine::Data::reportTime(){
    return epriv::Core::m_Engine->m_DebugManager.reportTime();
}
const double Engine::Resources::dt(){ 
    return epriv::Core::m_Engine->m_DebugManager.dt(); 
}
Scene* Engine::Resources::getCurrentScene(){ 
    return resourceManager->m_CurrentScene; 
}

vector<Scene*>& epriv::ResourceManager::scenes() {
    return m_Scenes;
}

const bool epriv::ResourceManager::_hasScene(const string& n){
    for (auto& scene : m_Scenes) {
        if (scene->name() == n)
            return true;
    }
    return false;
}
Texture* epriv::ResourceManager::_hasTexture(const string& n){
    auto& resourcePool = *m_Resources;
    for(uint i = 0; i < resourcePool.maxEntries(); ++i){
        EngineResource* r = resourcePool.getAsFast<EngineResource>(i+1);
        if(r){ 
            Texture* t = dynamic_cast<Texture*>(r); 
            if(t && t->name() == n){ 
                return t; 
            } 
        }
    }
    return nullptr;
}
Material* epriv::ResourceManager::_hasMaterial(const string& n) {
    auto& resourcePool = *m_Resources;
    for (uint i = 0; i < resourcePool.maxEntries(); ++i) {
        EngineResource* r = resourcePool.getAsFast<EngineResource>(i + 1);
        if (r) {
            Material* m = dynamic_cast<Material*>(r);
            if (m && m->name() == n) {
                return m;
            }
        }
    }
    return nullptr;
}
void epriv::ResourceManager::onPostUpdate() {
    if (m_ScenesToBeDeleted.size() > 0) {
        for (size_t i = 0; i < m_ScenesToBeDeleted.size(); ++i) {
            SAFE_DELETE(m_ScenesToBeDeleted[i]);
            m_ScenesToBeDeleted[i] = nullptr; //redundant?
        }
        vector_clear(m_ScenesToBeDeleted);
    }   
}
Handle epriv::ResourceManager::_addTexture(Texture* t) {
    return resourceManager->m_Resources->add(t, ResourceType::Texture);
}
Scene& epriv::ResourceManager::_getSceneByID(const uint& id) {
    return *m_Scenes[id - 1];
}
const unsigned int epriv::ResourceManager::_addScene(Scene& s){
    for (size_t i = 0; i < m_Scenes.size(); ++i) {
        if (m_Scenes[i] == nullptr) {
            m_Scenes[i] = &s;
            return i + 1;
        }
    }
    m_Scenes.push_back(&s);
    return m_Scenes.size();
}
const size_t epriv::ResourceManager::_numScenes(){
    return m_Scenes.size();
}

void Resources::Settings::enableDynamicMemory(const bool b){ 
    resourceManager->m_DynamicMemory = b; 
}
void Resources::Settings::disableDynamicMemory(){ 
    resourceManager->m_DynamicMemory = false; 
}

Engine_Window& Resources::getWindow(){ 
    return *resourceManager->m_Window; 
}
glm::uvec2 Resources::getWindowSize(){ 
    return resourceManager->m_Window->getSize(); 
}

const bool Resources::deleteScene(const string& sceneName) {
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr->name() == sceneName) {
            return Resources::deleteScene(*scene_ptr);
        }
    }
    return false;
}
const bool Resources::deleteScene(Scene& scene) {
    for (auto& deleted_scene_ptr : resourceManager->m_ScenesToBeDeleted) {
        if (&scene == deleted_scene_ptr) {
            return false; //already flagged for deletion
        }
    }
    resourceManager->m_ScenesToBeDeleted.push_back(&scene);
    return true;
}


Scene* Resources::getScene(const string& sceneName){
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr->name() == sceneName) {
            return scene_ptr;
        }
    }
    return nullptr;
}

void Resources::getShader(Handle& h, Shader*& p) { resourceManager->m_Resources->getAs(h, p); }
Shader* Resources::getShader(Handle& h) { Shader* p; resourceManager->m_Resources->getAs(h, p); return p; }
void Resources::getSoundData(Handle& h, SoundData*& p) { resourceManager->m_Resources->getAs(h, p); }
SoundData* Resources::getSoundData(Handle& h) { SoundData* p; resourceManager->m_Resources->getAs(h, p); return p; }
void Resources::getCamera(Handle& h, Camera*& p) { resourceManager->m_Resources->getAs(h, p); }
Camera* Resources::getCamera(Handle& h) { Camera* p; resourceManager->m_Resources->getAs(h, p); return p; }
void Resources::getFont(Handle& h, Font*& p) { resourceManager->m_Resources->getAs(h, p); }
Font* Resources::getFont(Handle& h) { Font* p; resourceManager->m_Resources->getAs(h, p); return p; }
void Resources::getMesh(Handle& h, Mesh*& p) { resourceManager->m_Resources->getAs(h, p); }
Mesh* Resources::getMesh(Handle& h) { Mesh* p; resourceManager->m_Resources->getAs(h, p); return p; }
void Resources::getShaderProgram(Handle& h, ShaderProgram*& p) { resourceManager->m_Resources->getAs(h, p); }
ShaderProgram* Resources::getShaderProgram(Handle& h) { ShaderProgram* p; resourceManager->m_Resources->getAs(h, p); return p; }

void Resources::getTexture(Handle& h, Texture*& p) { 
    resourceManager->m_Resources->getAs(h, p); 
}
Texture* Resources::getTexture(Handle& h) { 
    Texture* p; 
    resourceManager->m_Resources->getAs(h, p); 
    return p; 
}
Texture* Resources::getTexture(const string& name) {
    return resourceManager->_hasTexture(name); 
}
Handle Resources::getTextureHandle(const string& name) {
    auto* texture = resourceManager->_hasTexture(name);
    if (texture) {
        return resourceManager->m_Resources->getHandle<Texture>(*texture);
    }
    return Handle();
}
Handle Resources::getTextureHandle(Texture* texture) {
    if (texture) {
        return resourceManager->m_Resources->getHandle<Texture>(*texture);
    }
    return Handle();
}
void Resources::getMaterial(Handle& h, Material*& p) { 
    resourceManager->m_Resources->getAs(h, p); 
}
Material* Resources::getMaterial(Handle& h) { 
    Material* p; 
    resourceManager->m_Resources->getAs(h, p); 
    return p; 
}
Handle Resources::getMaterialHandle(const string& name) {
    auto* material = resourceManager->_hasMaterial(name);
    if (material) {
        return resourceManager->m_Resources->getHandle<Material>(*material);
    }
    return Handle();
}
Handle Resources::getMaterialHandle(Material* material) {
    if (material) {
        return resourceManager->m_Resources->getHandle<Material>(*material);
    }
    return Handle();
}

Handle Resources::addFont(const string& filename){
    return resourceManager->m_Resources->add(new Font(filename),ResourceType::Font);
}


vector<Handle> Resources::loadMesh(const string& fileOrData, const float& threshhold) {
    MeshRequest request(fileOrData, threshhold);
    request.request();
    vector<Handle> handles;
    for (auto& part : request.parts) {
        handles.push_back(part.handle);
    }
    return handles;
}
vector<Handle> Resources::loadMeshAsync(const string& fileOrData, const float& threshhold) {
    auto* request = new MeshRequest(fileOrData, threshhold); //to extend the lifetime to the threads, we manually delete later
    request->requestAsync();
    vector<Handle> handles;
    for (auto& part : request->parts) {
        handles.push_back(part.handle);
    }
    return handles;
}
Handle Resources::loadTexture(const string& file, const ImageInternalFormat::Format& internalFormat, const bool& mipmaps) {
    auto* texture = resourceManager->_hasTexture(file);
    if (!texture) {
        TextureRequest request(file, mipmaps, internalFormat);
        request.request();
        return request.part.handle;
    }
    return Resources::getTextureHandle(texture);
}
Handle Resources::loadTextureAsync(const string& file, const ImageInternalFormat::Format& internalFormat, const bool& mipmaps) {
    auto* texture = resourceManager->_hasTexture(file);
    if (!texture) {
        auto* request = new TextureRequest(file, mipmaps, internalFormat); //to extend the lifetime to the threads, we manually delete later
        request->requestAsync();
        return request->part.handle;
    }
    return Resources::getTextureHandle(texture);
}

Handle Resources::loadMaterial(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness) {
    auto* material = resourceManager->_hasMaterial(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
        request.request();
        return request.part.handle;
    }
    return Resources::getMaterialHandle(material);
}
Handle Resources::loadMaterialAsync(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness) {
    if (!resourceManager->_hasMaterial(name)) {
        auto* request = new MaterialRequest(name, diffuse, normal, glow, specular, ao, metalness, smoothness); //to extend the lifetime to the threads, we manually delete later
        request->requestAsync();
        return request->part.handle;
    }
    return Handle();
}
Handle Resources::loadMaterial(const string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) {
    auto* material = resourceManager->_hasMaterial(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
        return request.part.handle;
    }
    return Resources::getMaterialHandle(material);
}

Handle Resources::addShader(const string& fileOrData, const ShaderType::Type& type, const bool& fromFile){
    Shader* shader = new Shader(fileOrData, type, fromFile);
    return resourceManager->m_Resources->add(shader, ResourceType::Shader);
}

Handle Resources::addShaderProgram(const string& n, Shader& v, Shader& f){
    ShaderProgram* program = new ShaderProgram(n, v, f);
    return resourceManager->m_Resources->add(program, ResourceType::ShaderProgram);
}
Handle Resources::addShaderProgram(const string& n, Handle& v, Handle& f){
    Shader* vertexShader = resourceManager->m_Resources->getAsFast<Shader>(v);
    Shader* fragmentShader = resourceManager->m_Resources->getAsFast<Shader>(f);
    ShaderProgram* program = new ShaderProgram(n, *vertexShader, *fragmentShader);
    return resourceManager->m_Resources->add(program, ResourceType::ShaderProgram);
}

Handle Resources::addSoundData(const string& file){
    SoundData* soundData = new SoundData(file);
    return resourceManager->m_Resources->add(soundData,ResourceType::SoundData);
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
        if(resourceManager->m_DynamicMemory){
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
