
#include <serenity/system/Engine.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/font/Font.h>
#include <serenity/resources/sound/SoundData.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/system/window/Window.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/texture/TextureRequest.h>
#include <serenity/resources/material/MaterialRequest.h>
#include <serenity/events/Event.h>

#include <serenity/ecs/ECS.h>

Engine::view_ptr<Engine::priv::ResourceManager> Engine::priv::ResourceManager::RESOURCE_MANAGER = nullptr;

Engine::priv::ResourceManager::ResourceManager(const EngineOptions& options) {
    RESOURCE_MANAGER = this;

    m_ResourceModule.registerResourceTypeID<Texture>();
    m_ResourceModule.registerResourceTypeID<TextureCubemap>();
    m_ResourceModule.registerResourceTypeID<Mesh>();
    m_ResourceModule.registerResourceTypeID<Material>();
    m_ResourceModule.registerResourceTypeID<Font>();
    m_ResourceModule.registerResourceTypeID<Shader>();
    m_ResourceModule.registerResourceTypeID<ShaderProgram>();
    m_ResourceModule.registerResourceTypeID<UniformBufferObject>();
    m_ResourceModule.registerResourceTypeID<SoundData>();
}
void Engine::priv::ResourceManager::init(const EngineOptions& options){
    auto& window = m_Windows.emplace_back(std::unique_ptr<Window>(NEW Window{}));
    window->init(options);
    window->setJoystickProcessingActive(false);
}
void Engine::priv::ResourceManager::postUpdate() {
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


std::string Engine::Data::reportTime() {
    return priv::Core::m_Engine->m_DebugManager.reportTime();
}
std::mutex& Engine::Resources::getMutex() noexcept {
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->getMutex();
}
double Engine::Resources::dt() {
    return priv::Core::m_Engine->m_Misc.m_Dt;
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

Window& Engine::Resources::getWindow(uint32_t index) {
    return *Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Windows[index];
}
glm::uvec2 Engine::Resources::getWindowSize(uint32_t index) {
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
std::vector<Handle> Engine::Resources::loadMesh(std::string_view fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags) {
    MeshRequest request{ fileOrData, threshhold, flags, [](const std::vector<Handle>&) {} };
    request.request();
    auto handles = Engine::create_and_reserve<std::vector<Handle>>((uint32_t)request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
std::vector<Handle> Engine::Resources::loadMeshAsync(std::string_view fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags, MeshRequestCallback callback) {
    MeshRequest request{ fileOrData, threshhold, flags, std::move(callback) };
    request.request(true);
    auto handles = Engine::create_and_reserve<std::vector<Handle>>((uint32_t)request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
Handle Engine::Resources::loadTexture(std::string_view file, ImageInternalFormat internalFormat, bool mipmaps) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.m_Resource) {
        TextureRequest request{ file, mipmaps, internalFormat, TextureType::Texture2D, [](Handle) {} };
        request.request();
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTexture(sf::Image& sfImage, std::string_view texture_name, ImageInternalFormat internalFormat, bool mipmaps) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.m_Resource) {
        TextureRequest request{ sfImage, texture_name, mipmaps, internalFormat, TextureType::Texture2D, [](Handle) {} };
        request.request();
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTextureAsync(std::string_view file, ImageInternalFormat internalFormat, bool mipmaps, Engine::ResourceCallback callback) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.m_Resource) {
        TextureRequest request{ file, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadTextureAsync(sf::Image& sfImage, std::string_view texture_name, ImageInternalFormat internalFormat, bool mipmaps, Engine::ResourceCallback callback) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.m_Resource) {
        TextureRequest request{ sfImage, texture_name, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterial(std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, [](Handle) {} };
        request.request();
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterialAsync(std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness, Engine::ResourceCallback callback) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, std::move(callback) };
        request.request(true);
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::loadMaterial(std::string_view name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, [](Handle) {} };
        //request.request(); //the above creates the material and is immediately available for use, no need to request
        return request.m_Part.m_Handle;
    }
    return Handle{};
}
Handle Engine::Resources::addShader(std::string_view fileOrData, ShaderType type, bool fromFile){
    return Engine::Resources::addResource<Shader>(fileOrData, type, fromFile);
}
Handle Engine::Resources::addShaderProgram(std::string_view n, Handle v, Handle f){
    auto vertexShader   = Engine::Resources::getResource<Shader>(v);
    auto fragmentShader = Engine::Resources::getResource<Shader>(f);
    return Engine::Resources::addResource<ShaderProgram>(n, v, f);
}
bool Engine::Resources::setCurrentScene(Scene* newScene){
    auto& resourceMgr = *Engine::priv::ResourceManager::RESOURCE_MANAGER;
    Scene* oldScene   = resourceMgr.m_CurrentScene;
    if (oldScene == newScene || std::get<2>(resourceMgr.m_SceneSwap) == true) {
        return false;
    }
    if (newScene == std::get<1>(resourceMgr.m_SceneSwap)) {
        return false;
    }   
    resourceMgr.m_CurrentScene = newScene;
    resourceMgr.m_SceneSwap = { oldScene, newScene, true };
    return true;
}
bool Engine::Resources::setCurrentScene(std::string_view sceneName){
    return Engine::Resources::setCurrentScene(Engine::Resources::getScene(sceneName));
}
