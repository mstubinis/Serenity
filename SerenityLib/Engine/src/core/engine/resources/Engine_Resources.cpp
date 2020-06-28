#include <boost/make_shared.hpp>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/window/Window.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/resources/Handle.h>

#include <core/engine/sounds/SoundData.h>

#include <ecs/ECS.h>
#include <iostream>

using namespace Engine;
using namespace std;

priv::ResourceManager* resourceManager = nullptr;

priv::ResourceManager::ResourceManager(const EngineOptions& options) : m_Resources(32768){
    resourceManager    = this;
}
priv::ResourceManager::~ResourceManager(){ 
    cleanup();
}
void priv::ResourceManager::cleanup() {
    SAFE_DELETE_VECTOR(m_Windows);
    SAFE_DELETE_VECTOR(m_Scenes);
}
void priv::ResourceManager::_init(const EngineOptions& options){
    auto* window = NEW Window(options);
    window->setJoystickProcessingActive(false);
    m_Windows.push_back(window);
}
vector<Scene*>& priv::ResourceManager::scenes() {
    return m_Scenes;
}
void priv::ResourceManager::onPostUpdate() {
    if (m_ScenesToBeDeleted.size() > 0) {
        for (size_t i = 0; i < m_ScenesToBeDeleted.size(); ++i) {
            size_t index = 0;
            for (size_t j = 0; j < m_Scenes.size(); ++j) {
                if (m_Scenes[j] && m_Scenes[j]->name() == m_ScenesToBeDeleted[i]->name()) {
                    index = j;
                    break;
                }
            }
            SAFE_DELETE(m_ScenesToBeDeleted[i]);
            m_Scenes[index] = nullptr;
        }
        m_ScenesToBeDeleted.clear();
    }   
}
Handle priv::ResourceManager::_addTexture(Texture* t) {
    return m_Resources.add(t, ResourceType::Texture);
}
Scene& priv::ResourceManager::_getSceneByID(std::uint32_t id) {
    return *m_Scenes[id - 1];
}
unsigned int priv::ResourceManager::AddScene(Scene& s){
    for (size_t i = 0; i < m_Scenes.size(); ++i) {
        if (m_Scenes[i] == nullptr) {
            m_Scenes[i] = &s;
            unsigned int res = static_cast<unsigned int>(i) + 1U;
            return res;
        }
    }
    m_Scenes.push_back(&s);
    return static_cast<unsigned int>(m_Scenes.size());
}
string Engine::Data::reportTime() {
    return priv::Core::m_Engine->m_DebugManager.reportTime();
}
float Resources::dt() {
    return priv::Core::m_Engine->m_DebugManager.dt();
}
double Resources::timeScale(){
    return priv::Core::m_Engine->m_DebugManager.timeScale();
}
double Resources::applicationTime() {
    return priv::Core::m_Engine->m_DebugManager.totalTime();
}
Scene* Resources::getCurrentScene() {
    return resourceManager->m_CurrentScene;
}
void Resources::Settings::enableDynamicMemory(bool dynamicMemory){ 
    resourceManager->m_DynamicMemory = dynamicMemory;
}
void Resources::Settings::disableDynamicMemory(){ 
    resourceManager->m_DynamicMemory = false; 
}
Window& Resources::getWindow(){
    return *resourceManager->m_Windows[0]; 
}
glm::uvec2 Resources::getWindowSize(){ 
    return resourceManager->m_Windows[0]->getSize(); 
}

Window& Resources::getWindow(unsigned int index) {
    return *resourceManager->m_Windows[index];
}
glm::uvec2 Resources::getWindowSize(unsigned int index) {
    return resourceManager->m_Windows[index]->getSize();
}


bool Resources::deleteScene(string_view sceneName) {
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return Resources::deleteScene(*scene_ptr);
        }
    }
    return false;
}
bool Resources::deleteScene(Scene& scene) {
    for (auto& deleted_scene_ptr : resourceManager->m_ScenesToBeDeleted) {
        if (scene.name() == deleted_scene_ptr->name()) {
            return false; //already flagged for deletion
        }
    }
    resourceManager->m_ScenesToBeDeleted.push_back(&scene);
    return true;
}


Scene* Resources::getScene(string_view sceneName){
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return scene_ptr;
        }
    }
    return nullptr;
}

void Resources::getShader(Handle h, Shader*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
Shader* Resources::getShader(Handle h) { 
    Shader* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
void Resources::getSoundData(Handle h, SoundData*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
SoundData* Resources::getSoundData(Handle h) { 
    SoundData* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
void Resources::getCamera(Handle h, Camera*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
Camera* Resources::getCamera(Handle h) { 
    Camera* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
void Resources::getFont(Handle h, Font*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
Font* Resources::getFont(Handle h) { 
    Font* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
void Resources::getMesh(Handle h, Mesh*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
Mesh* Resources::getMesh(Handle h) { 
    Mesh* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
void Resources::getShaderProgram(Handle h, ShaderProgram*& p) { 
    resourceManager->m_Resources.getAs(h, p); 
}
ShaderProgram* Resources::getShaderProgram(Handle h) { 
    ShaderProgram* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}

void Resources::getTexture(Handle h, Texture*& p) {
    resourceManager->m_Resources.getAs(h, p); 
}
Texture* Resources::getTexture(Handle h) {
    Texture* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
Texture* Resources::getTexture(string_view name) {
    return resourceManager->HasResource<Texture>(name); 
}
void Resources::getMaterial(Handle h, Material*& p) {
    resourceManager->m_Resources.getAs(h, p); 
}
Material* Resources::getMaterial(Handle h) {
    Material* p; 
    resourceManager->m_Resources.getAs(h, p); 
    return p; 
}
Handle Resources::addFont(const string& filename, int height, int width, float line_height){
    return resourceManager->m_Resources.add(NEW Font(filename, height, width, line_height), ResourceType::Font);
}


vector<Handle> Resources::loadMesh(const string& fileOrData, float threshhold) {
    MeshRequest request(fileOrData, threshhold);
    request.request();
    vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.push_back(part.handle);
    }
    return handles;
}
vector<Handle> Resources::loadMeshAsync(const string& fileOrData, float threshhold) {
    MeshRequest request(fileOrData, threshhold);
    request.request(true);
    vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.push_back(part.handle);
    }
    return handles;
}
Handle Resources::loadTexture(const string& file, ImageInternalFormat::Format internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(file);
    if (!texture) {
        TextureRequest request(file, mipmaps, internalFormat);
        request.request(false);
        return request.part.handle;
    }
    return Handle();
}
Handle Resources::loadTexture(sf::Image& sfImage, const string& texture_name, ImageInternalFormat::Format internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(texture_name);
    if (!texture) {
        TextureRequestFromMemory request(sfImage, texture_name, mipmaps, internalFormat);
        request.request(false);
        return request.part.handle;
    }
    return Handle();
}
Handle Resources::loadTextureAsync(const string& file, ImageInternalFormat::Format internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(file);
    if (!texture) {
        TextureRequest request(file, mipmaps, internalFormat);
        request.request(true);
        return request.part.handle;
    }
    return Handle();
}
Handle Resources::loadTextureAsync(sf::Image& sfImage, const string& texture_name, ImageInternalFormat::Format internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(texture_name);
    if (!texture) {
        TextureRequestFromMemory request(sfImage, texture_name, mipmaps, internalFormat);
        request.request(true);
        return request.part.handle;
    }
    return Handle();
}

Handle Resources::loadMaterial(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness) {
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
        request.request(false);
        return request.m_Part.m_Handle;
    }
    return Handle();
}
Handle Resources::loadMaterialAsync(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness) {   
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle();
}
Handle Resources::loadMaterial(const string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) {
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
        request.request(false);
        return request.m_Part.m_Handle;
    }
    return Handle();
}

Handle Resources::addShader(const string& fileOrData, ShaderType::Type type, bool fromFile){
    Shader* shader = NEW Shader(fileOrData, type, fromFile);
    return resourceManager->m_Resources.add(shader, ResourceType::Shader);
}

Handle Resources::addShaderProgram(const string& n, Shader& v, Shader& f){
    ShaderProgram* program = NEW ShaderProgram(n, v, f);
    return resourceManager->m_Resources.add(program, ResourceType::ShaderProgram);
}
Handle Resources::addShaderProgram(const string& n, Handle v, Handle f){
    Shader* vertexShader   = resourceManager->m_Resources.getAsFast<Shader>(v);
    Shader* fragmentShader = resourceManager->m_Resources.getAsFast<Shader>(f);
    ShaderProgram* program = NEW ShaderProgram(n, *vertexShader, *fragmentShader);
    return resourceManager->m_Resources.add(program, ResourceType::ShaderProgram);
}

Handle Resources::addSoundData(const string& file){
    SoundData* soundData = NEW SoundData(file);
    return resourceManager->m_Resources.add(soundData,ResourceType::SoundData);
}

bool Resources::setCurrentScene(Scene* newScene){
    Scene* oldScene = resourceManager->m_CurrentScene;

    priv::EventSceneChanged e(oldScene, newScene);
    Event ev(EventType::SceneChanged);
    ev.eventSceneChanged = e;
    priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    
    if(!oldScene){
        cout << "---- Initial scene set to: " << newScene->name() << endl;
        resourceManager->m_CurrentScene = newScene; 
        priv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        return false;
    }
    if(oldScene != newScene){
        cout << "---- Scene Change started (" << oldScene->name() << ") to (" << newScene->name() << ") ----" << endl;
        if(resourceManager->m_DynamicMemory){
            //mark game object resources to minus use count
        }
        priv::InternalScenePublicInterface::GetECS(*oldScene).onSceneLeft(*oldScene);
        resourceManager->m_CurrentScene = newScene;
        priv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        if(resourceManager->m_DynamicMemory){
            //mark game object resources to add use count
        }
        cout << "-------- Scene Change ended --------" << endl;
        return true;
    }
    return false;
}
bool Resources::setCurrentScene(string_view s){ 
    return Resources::setCurrentScene(Resources::getScene(s)); 
}
