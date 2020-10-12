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
#include <core/engine/events/Event.h>

#include <ecs/ECS.h>

Engine::view_ptr<Engine::priv::ResourceManager> Engine::priv::ResourceManager::RESOURCE_MANAGER = nullptr;

Engine::priv::ResourceManager::ResourceManager(const EngineOptions& options) {
    RESOURCE_MANAGER = this;

    m_ResourceModule.registerResourceType<Texture>();
    m_ResourceModule.registerResourceType<Mesh>();
    m_ResourceModule.registerResourceType<Material>();
    m_ResourceModule.registerResourceType<Font>();
    m_ResourceModule.registerResourceType<Shader>();
    m_ResourceModule.registerResourceType<ShaderProgram>();
    m_ResourceModule.registerResourceType<SoundData>();
}
void Engine::priv::ResourceManager::init(const EngineOptions& options){
    auto& window = m_Windows.emplace_back(std::unique_ptr<Window>(NEW Window{}));
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
Engine::view_ptr<Scene> Engine::priv::ResourceManager::getSceneByID(uint32_t id) {
    uint32_t index = id - 1;
    return (index < m_Scenes.size()) ? m_Scenes[index].get() : nullptr;
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
Engine::view_ptr<Scene> Engine::Resources::getCurrentScene() {
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_CurrentScene;
}
Window& Engine::Resources::getWindow(){
    return *Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Windows[0];
}
glm::uvec2 Engine::Resources::getWindowSize(){
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Windows[0]->getSize();
}

Window& Engine::Resources::getWindow(unsigned int index) {
    return *Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Windows[index];
}
glm::uvec2 Engine::Resources::getWindowSize(unsigned int index) {
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Windows[index]->getSize();
}


bool Engine::Resources::deleteScene(std::string_view sceneName) {
    for (auto& scene_ptr : Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return Resources::deleteScene(*scene_ptr);
        }
    }
    return false;
}
bool Engine::Resources::deleteScene(Scene& scene) {
    for (auto& deleted_scene_ptr : Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ScenesToBeDeleted) {
        if (scene.name() == deleted_scene_ptr->name()) {
            return false; //already flagged for deletion
        }
    }
    Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ScenesToBeDeleted.emplace_back(&scene);
    return true;
}


Engine::view_ptr<Scene> Engine::Resources::getScene(std::string_view sceneName){
    for (auto& scene_ptr : Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return scene_ptr.get();
        }
    }
    return nullptr;
}

std::vector<Handle> Engine::Resources::loadMesh(const std::string& fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags) {
    MeshRequest request{ fileOrData, threshhold, flags, []() {} };
    request.request();
    std::vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
std::vector<Handle> Engine::Resources::loadMeshAsync(const std::string& fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags, std::function<void()> callback) {
    MeshRequest request{ fileOrData, threshhold, flags, std::move(callback) };
    request.request(true);
    std::vector<Handle> handles;
    handles.reserve(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
Handle Engine::Resources::loadTexture(const std::string& file, ImageInternalFormat internalFormat, bool mipmaps) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.first) {
        TextureRequest request{ file, mipmaps, internalFormat, TextureType::Texture2D,[]() {} };
        request.request();
        return request.m_Part.handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTexture(sf::Image& sfImage, const std::string& texture_name, ImageInternalFormat internalFormat, bool mipmaps) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.first) {
        TextureRequest request{ sfImage, texture_name, mipmaps, internalFormat, TextureType::Texture2D, []() {} };
        request.request();
        return request.m_Part.handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTextureAsync(const std::string& file, ImageInternalFormat internalFormat, bool mipmaps, std::function<void()> callback) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.first) {
        TextureRequest request{ file, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        return request.m_Part.handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTextureAsync(sf::Image& sfImage, const std::string& texture_name, ImageInternalFormat internalFormat, bool mipmaps, std::function<void()> callback) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.first) {
        TextureRequest request{ sfImage, texture_name, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        return request.m_Part.handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterial(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.first) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, []() {} };
        request.request();
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterialAsync(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness, std::function<void()> callback) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.first) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, std::move(callback) };
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterial(const std::string& name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.first) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, []() {} };
        //request.request(); //the above creates the material and is immediately available for use, no need to request
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::addShader(const std::string& fileOrData, ShaderType type, bool fromFile){
    return Engine::Resources::addResource<Shader>(fileOrData, type, fromFile);
}
Handle Engine::Resources::addShaderProgram(const std::string& n, Handle v, Handle f){
    auto vertexShader = Engine::Resources::getResource<Shader>(v);
    auto fragmentShader = Engine::Resources::getResource<Shader>(f);
    return Engine::Resources::addResource<ShaderProgram>(n, v, f);
}
bool Engine::Resources::setCurrentScene(Scene* newScene){
    Scene* oldScene = Engine::priv::ResourceManager::RESOURCE_MANAGER->m_CurrentScene;

    Event ev{ EventType::SceneChanged };
    ev.eventSceneChanged = Engine::priv::EventSceneChanged{ oldScene, newScene };
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    
    if(!oldScene){
        ENGINE_PRODUCTION_LOG("---- Initial scene set to: " << newScene->name())
        Engine::priv::ResourceManager::RESOURCE_MANAGER->m_CurrentScene = newScene;
        Engine::priv::InternalScenePublicInterface::GetECS(*newScene).onSceneEntered(*newScene);
        return false;
    }
    if(oldScene != newScene){
        ENGINE_PRODUCTION_LOG("---- Scene Change started (" << oldScene->name() << ") to (" << newScene->name() << ") ----")
        Engine::priv::InternalScenePublicInterface::GetECS(*oldScene).onSceneLeft(*oldScene);
        Engine::priv::ResourceManager::RESOURCE_MANAGER->m_CurrentScene = newScene;
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
