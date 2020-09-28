#include <core/engine/utils/PrecompiledHeader.h>

#include <core/engine/system/Engine.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/sounds/SoundData.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/system/window/Window.h>

#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/resources/Handle.h>

#include <ecs/ECS.h>


Engine::priv::ResourceManager* resourceManager = nullptr;

Engine::priv::ResourceManager::ResourceManager(const EngineOptions& options) 
    : m_ResourcePool(32768)
{
    resourceManager    = this;
}
Engine::priv::ResourceManager::~ResourceManager(){
    cleanup();
}
void Engine::priv::ResourceManager::cleanup() {
}
void Engine::priv::ResourceManager::init(const EngineOptions& options){
    auto& window = m_Windows.emplace_back(std::unique_ptr<Window>(NEW Window()));
    window->init(options);
    window->setJoystickProcessingActive(false);
}
void Engine::priv::ResourceManager::onPostUpdate() {
    if (m_ScenesToBeDeleted.size() > 0) {
        for (size_t i = 0; i < m_ScenesToBeDeleted.size(); ++i) {
            if (m_ScenesToBeDeleted[i]) {
                for (size_t j = 0; j < m_Scenes.size(); ++j) {
                    if (m_Scenes[j] && m_Scenes[j]->name() == m_ScenesToBeDeleted[i]->name()) {
                        m_Scenes[j].reset(nullptr);
                        m_ScenesToBeDeleted[i] = nullptr;
                        break;
                    }
                }
            }
        }
        m_ScenesToBeDeleted.clear();
    }   
}
Handle Engine::priv::ResourceManager::_addTexture(Texture* t) {
    return m_ResourcePool.add(t, (unsigned int)ResourceType::Texture);
}
Scene& Engine::priv::ResourceManager::_getSceneByID(std::uint32_t id) {
    return *m_Scenes[id - 1];
}
unsigned int Engine::priv::ResourceManager::AddScene(Scene& s){
    for (size_t i = 0; i < m_Scenes.size(); ++i) {
        if (m_Scenes[i] == nullptr) {
            m_Scenes[i].reset(&s);
            return (unsigned int)i + 1U;
        }
    }
    m_Scenes.emplace_back(std::unique_ptr<Scene>(&s));
    return (unsigned int)m_Scenes.size();
}
std::string Engine::Data::reportTime() {
    return priv::Core::m_Engine->m_DebugManager.reportTime();
}
float Engine::Resources::dt() {
    return priv::Core::m_Engine->m_DebugManager.dt();
}
double Engine::Resources::timeScale(){
    return priv::Core::m_Engine->m_DebugManager.timeScale();
}
double Engine::Resources::applicationTime() {
    return priv::Core::m_Engine->m_DebugManager.totalTime();
}
Scene* Engine::Resources::getCurrentScene() {
    return resourceManager->m_CurrentScene;
}
Window& Engine::Resources::getWindow(){
    return *resourceManager->m_Windows[0]; 
}
glm::uvec2 Engine::Resources::getWindowSize(){
    return resourceManager->m_Windows[0]->getSize(); 
}

Window& Engine::Resources::getWindow(unsigned int index) {
    return *resourceManager->m_Windows[index];
}
glm::uvec2 Engine::Resources::getWindowSize(unsigned int index) {
    return resourceManager->m_Windows[index]->getSize();
}


bool Engine::Resources::deleteScene(std::string_view sceneName) {
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return Resources::deleteScene(*scene_ptr);
        }
    }
    return false;
}
bool Engine::Resources::deleteScene(Scene& scene) {
    for (auto& deleted_scene_ptr : resourceManager->m_ScenesToBeDeleted) {
        if (scene.name() == deleted_scene_ptr->name()) {
            return false; //already flagged for deletion
        }
    }
    resourceManager->m_ScenesToBeDeleted.emplace_back(&scene);
    return true;
}


Scene* Engine::Resources::getScene(std::string_view sceneName){
    for (auto& scene_ptr : resourceManager->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return scene_ptr.get();
        }
    }
    return nullptr;
}

void Engine::Resources::getShader(Handle h, Shader*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Shader* Engine::Resources::getShader(Handle h) {
    Shader* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
void Engine::Resources::getSoundData(Handle h, SoundData*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
SoundData* Engine::Resources::getSoundData(Handle h) {
    SoundData* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
void Engine::Resources::getCamera(Handle h, Camera*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Camera* Engine::Resources::getCamera(Handle h) {
    Camera* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
void Engine::Resources::getFont(Handle h, Font*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Font* Engine::Resources::getFont(Handle h) {
    Font* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
Font* Engine::Resources::getFont(std::string_view name) {
    return resourceManager->HasResource<Font>(name);
}
void Engine::Resources::getMesh(Handle h, Mesh*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Mesh* Engine::Resources::getMesh(Handle h) {
    Mesh* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
Mesh* Engine::Resources::getMesh(std::string_view name) {
    return resourceManager->HasResource<Mesh>(name);
}
void Engine::Resources::getShaderProgram(Handle h, ShaderProgram*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
ShaderProgram* Engine::Resources::getShaderProgram(Handle h) {
    ShaderProgram* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
ShaderProgram* Engine::Resources::getShaderProgram(std::string_view name) {
    return resourceManager->HasResource<ShaderProgram>(name);
}

void Engine::Resources::getTexture(Handle h, Texture*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Texture* Engine::Resources::getTexture(Handle h) {
    Texture* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
Texture* Engine::Resources::getTexture(std::string_view name) {
    return resourceManager->HasResource<Texture>(name); 
}
void Engine::Resources::getMaterial(Handle h, Material*& p) {
    resourceManager->m_ResourcePool.getAs(h, p);
}
Material* Engine::Resources::getMaterial(Handle h) {
    Material* p; 
    resourceManager->m_ResourcePool.getAs(h, p);
    return p; 
}
Material* Engine::Resources::getMaterial(std::string_view name) {
    return resourceManager->HasResource<Material>(name);
}
Handle Engine::Resources::addFont(const std::string& filename, int height, int width, float line_height){
    return resourceManager->m_ResourcePool.add(NEW Font(filename, height, width, line_height), (unsigned int)ResourceType::Font);
}


std::vector<Handle> Engine::Resources::loadMesh(const std::string& fileOrData, float threshhold) {
    MeshRequest request(fileOrData, threshhold, []() {});
    request.request();
    std::vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
std::vector<Handle> Engine::Resources::loadMeshAsync(const std::string& fileOrData, float threshhold, std::function<void()> callback) {
    MeshRequest request(fileOrData, threshhold, std::move(callback));
    request.request(true);
    std::vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
Handle Engine::Resources::loadTexture(const std::string& file, ImageInternalFormat internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(file);
    if (!texture) {
        TextureRequest request(file, mipmaps, internalFormat, GL_TEXTURE_2D,[]() {});
        request.request(false);
        return request.part.handle;
    }
    return Handle();
}
Handle Engine::Resources::loadTexture(sf::Image& sfImage, const std::string& texture_name, ImageInternalFormat internalFormat, bool mipmaps) {
    auto* texture = resourceManager->HasResource<Texture>(texture_name);
    if (!texture) {
        TextureRequestFromMemory request(sfImage, texture_name, mipmaps, internalFormat, GL_TEXTURE_2D, []() {});
        request.request(false);
        return request.part.handle;
    }
    return Handle();
}
Handle Engine::Resources::loadTextureAsync(const std::string& file, ImageInternalFormat internalFormat, bool mipmaps, std::function<void()> callback) {
    auto* texture = resourceManager->HasResource<Texture>(file);
    if (!texture) {
        TextureRequest request(file, mipmaps, internalFormat, GL_TEXTURE_2D, std::move(callback));
        request.request(true);
        return request.part.handle;
    }
    return Handle();
}
Handle Engine::Resources::loadTextureAsync(sf::Image& sfImage, const std::string& texture_name, ImageInternalFormat internalFormat, bool mipmaps, std::function<void()> callback) {
    auto* texture = resourceManager->HasResource<Texture>(texture_name);
    if (!texture) {
        TextureRequestFromMemory request(sfImage, texture_name, mipmaps, internalFormat, GL_TEXTURE_2D, std::move(callback));
        request.request(true);
        return request.part.handle;
    }
    return Handle();
}

Handle Engine::Resources::loadMaterial(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness) {
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness, []() {});
        request.request(false);
        return request.m_Part.m_Handle;
    }
    return Handle();
}
Handle Engine::Resources::loadMaterialAsync(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness, std::function<void()> callback) {
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness, std::move(callback));
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle();
}
Handle Engine::Resources::loadMaterial(const std::string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) {
    auto* material = resourceManager->HasResource<Material>(name);
    if (!material) {
        MaterialRequest request(name, diffuse, normal, glow, specular, ao, metalness, smoothness, []() {});
        //request.request(false); //the above creates the material and is immediately available for use, no need to request
        return request.m_Part.m_Handle;
    }
    return Handle();
}

Handle Engine::Resources::addShader(const std::string& fileOrData, ShaderType type, bool fromFile){
    Shader* shader = NEW Shader(fileOrData, type, fromFile);
    return resourceManager->m_ResourcePool.add(shader, (unsigned int)ResourceType::Shader);
}

Handle Engine::Resources::addShaderProgram(const std::string& n, Shader& v, Shader& f){
    ShaderProgram* program = NEW ShaderProgram(n, v, f);
    return resourceManager->m_ResourcePool.add(program, (unsigned int)ResourceType::ShaderProgram);
}
Handle Engine::Resources::addShaderProgram(const std::string& n, Handle v, Handle f){
    Shader* vertexShader   = resourceManager->m_ResourcePool.getAsFast<Shader>(v);
    Shader* fragmentShader = resourceManager->m_ResourcePool.getAsFast<Shader>(f);
    ShaderProgram* program = NEW ShaderProgram(n, *vertexShader, *fragmentShader);
    return resourceManager->m_ResourcePool.add(program, (unsigned int)ResourceType::ShaderProgram);
}

Handle Engine::Resources::addSoundData(const std::string& file){
    SoundData* soundData = NEW SoundData(file);
    return resourceManager->m_ResourcePool.add(soundData, (unsigned int)ResourceType::SoundData);
}

bool Engine::Resources::setCurrentScene(Scene* newScene){
    Scene* oldScene = resourceManager->m_CurrentScene;

    Event ev(EventType::SceneChanged);
    ev.eventSceneChanged = Engine::priv::EventSceneChanged(oldScene, newScene);
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    
    if(!oldScene){
        ENGINE_PRODUCTION_LOG("---- Initial scene set to: " << newScene->name())
        resourceManager->m_CurrentScene = newScene; 
        Engine::priv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        return false;
    }
    if(oldScene != newScene){
        ENGINE_PRODUCTION_LOG("---- Scene Change started (" << oldScene->name() << ") to (" << newScene->name() << ") ----")
        Engine::priv::InternalScenePublicInterface::GetECS(*oldScene).onSceneLeft(*oldScene);
        resourceManager->m_CurrentScene = newScene;
        Engine::priv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);

        Engine::priv::InternalScenePublicInterface::SkipRenderThisFrame(*newScene, true);

        ENGINE_PRODUCTION_LOG("-------- Scene Change to (" << newScene->name() << ") ended --------")

        return true;
    }
    return false;
}
bool Engine::Resources::setCurrentScene(std::string_view sceneName){
    return Engine::Resources::setCurrentScene(Engine::Resources::getScene(sceneName));
}
