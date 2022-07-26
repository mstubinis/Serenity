
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

Engine::priv::ResourceManager::ResourceManager(const EngineOptions& options) 
{
    RESOURCE_MANAGER = this;

    m_ResourceModule.registerResource<Texture>();
    m_ResourceModule.registerResource<TextureCubemap>();
    m_ResourceModule.registerResource<Mesh>();
    m_ResourceModule.registerResource<Material>();
    m_ResourceModule.registerResource<Font>();
    m_ResourceModule.registerResource<Shader>();
    m_ResourceModule.registerResource<ShaderProgram>();
    m_ResourceModule.registerResource<UniformBufferObject>();
    m_ResourceModule.registerResource<SoundData>();
}
Engine::priv::ResourceManager::~ResourceManager() {
    SAFE_DELETE_VECTOR(m_Scenes);
}
void Engine::priv::ResourceManager::postUpdate() {
    if (m_ScenesToBeDeleted.size() > 0) {
        for (size_t i = 0; i < m_ScenesToBeDeleted.size(); ++i) {
            if (m_ScenesToBeDeleted[i]) {
                for (size_t j = 0; j < m_Scenes.size(); ++j) {
                    if (m_Scenes[j] && m_Scenes[j]->name() == m_ScenesToBeDeleted[i]->name()) {
                        SAFE_DELETE(m_Scenes[j]);
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
    return id < m_Scenes.size() ? m_Scenes[id] : nullptr;
}


std::string Engine::Data::reportTime() {
    return Engine::priv::Core::m_Engine->m_Modules->m_DebugManager.reportTime();
}
std::mutex& Engine::Resources::getMutex() noexcept {
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->getMutex();
}
double Engine::Resources::dt() {
    return Engine::priv::Core::m_Engine->m_Misc.m_Dt;
}
float Engine::Resources::dtSimulation() {
    return Engine::priv::Core::m_Engine->m_Misc.m_DtSimulation;
}
double Engine::Resources::timeScale(){
    return Engine::priv::Core::m_Engine->m_Modules->m_DebugManager.timeScale();
}
double Engine::Resources::applicationTime() {
    return Engine::priv::Core::m_Engine->m_Modules->m_DebugManager.totalTime();
}
Engine::view_ptr<Scene> Engine::Resources::getCurrentScene() {
    return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_CurrentScene;
}
bool Engine::Resources::deleteScene(std::string_view sceneName) {
    for (auto& scene_ptr : Engine::priv::ResourceManager::RESOURCE_MANAGER->m_Scenes) {
        if (scene_ptr && scene_ptr->name() == sceneName) {
            return Engine::Resources::deleteScene(*scene_ptr);
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
            return scene_ptr;
        }
    }
    return nullptr;
}
std::vector<Handle> Engine::Resources::loadMesh(std::string_view fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags) {
    MeshRequest request{ fileOrData, threshhold, flags, [](const std::vector<Handle>&) {} };
    request.request();
    auto handles = Engine::create_and_reserve<std::vector<Handle>>(request.m_Parts.size());
    for (auto& part : request.m_Parts) {
        handles.emplace_back(part.handle);
    }
    return handles;
}
std::vector<Handle> Engine::Resources::loadMeshAsync(std::string_view fileOrData, float threshhold, MeshCollisionLoadingFlag::Flag flags, MeshRequestCallback callback) {
    MeshRequest request{ fileOrData, threshhold, flags, std::move(callback) };
    request.request(true);
    auto handles = Engine::create_and_reserve<std::vector<Handle>>(request.m_Parts.size());
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
        texture.m_Handle = request.m_Part.m_Handle;
    }
    return texture.m_Handle;
}
Handle Engine::Resources::loadTexture(uint8_t* pixels, uint32_t width, uint32_t height, std::string_view texture_name, ImageInternalFormat internalFormat, bool mipmaps, bool dispatchEventLoaded) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.m_Resource) {
        texture.m_Handle = Engine::Resources::addResource<Texture>(pixels, width, height, texture_name, mipmaps, internalFormat, TextureType::Texture2D, dispatchEventLoaded);
    }
    return texture.m_Handle;
}
Handle Engine::Resources::loadTextureAsync(std::string_view file, ImageInternalFormat internalFormat, bool mipmaps, Engine::ResourceCallback callback) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.m_Resource) {
        TextureRequest request{ file, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        texture.m_Handle = request.m_Part.m_Handle;
    }
    return texture.m_Handle;
}
Handle Engine::Resources::loadTextureAsync(uint8_t* pixels, uint32_t width, uint32_t height, std::string_view texture_name, ImageInternalFormat internalFormat, bool mipmaps, Engine::ResourceCallback callback) {
    auto texture = Engine::Resources::getResource<Texture>(texture_name);
    if (!texture.m_Resource) {
        TextureRequest request{ pixels, width, height, texture_name, mipmaps, internalFormat, TextureType::Texture2D, std::move(callback) };
        request.request(true);
        texture.m_Handle = request.m_Part.m_Handle;
    }
    return texture.m_Handle;
}
Handle Engine::Resources::loadMaterial(std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, [](Handle) {} };
        request.request();
        material.m_Handle = request.m_Part.m_Handle;
    }
    return material.m_Handle;
}
Handle Engine::Resources::loadMaterialAsync(std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness, Engine::ResourceCallback callback) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, std::move(callback) };
        request.request(true);
        material.m_Handle = request.m_Part.m_Handle;
    }
    return material.m_Handle;
}
Handle Engine::Resources::loadMaterial(std::string_view name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness) {
    auto material = Engine::Resources::getResource<Material>(name);
    if (!material.m_Resource) {
        MaterialRequest request{ name, diffuse, normal, glow, specular, ao, metalness, smoothness, [](Handle) {} };
        //request.request(); //the above creates the material and is immediately available for use, no need to request
        material.m_Handle = request.m_Part.m_Handle;
    }
    return material.m_Handle;
}
Handle Engine::Resources::loadShader(std::string_view fileOrData, ShaderType type) {
    auto shader = Engine::Resources::getResource<Shader>(fileOrData);
    if (!shader.m_Resource) {
        shader.m_Handle = Engine::Resources::addResource<Shader>(fileOrData, type);;
    }
    return shader.m_Handle;
}
Handle Engine::Resources::addShaderProgram(std::string_view name, Handle vertexHandle, Handle fragmentHandle){
    return Engine::Resources::addResource<ShaderProgram>(name, vertexHandle, fragmentHandle);
}
Handle Engine::Resources::addShaderProgram(std::string_view name, std::string_view vertexFileOrData, std::string_view fragmentFileOrData) {
    return Engine::Resources::addResource<ShaderProgram>(name, vertexFileOrData, fragmentFileOrData);
}
bool Engine::Resources::setCurrentScene(Scene* newScene){
    auto& resourceMgr = *Engine::priv::ResourceManager::RESOURCE_MANAGER;
    Scene* oldScene   = resourceMgr.m_CurrentScene;
    if (resourceMgr.m_SceneSwap.newScene || oldScene == newScene || newScene == resourceMgr.m_SceneSwap.newScene) {
        return false;
    }
    resourceMgr.m_CurrentScene = newScene;
    resourceMgr.m_SceneSwap    = { oldScene, newScene };
    return true;
}
bool Engine::Resources::setCurrentScene(std::string_view sceneName){
    return Engine::Resources::setCurrentScene(Engine::Resources::getScene(sceneName));
}




Engine::priv::SceneLUABinder Engine::priv::lua::resources::getCurrentSceneLUA() {
    return Engine::Resources::getCurrentScene() != nullptr ? Engine::priv::SceneLUABinder{ *Engine::Resources::getCurrentScene() } : Engine::priv::SceneLUABinder{};
}
void Engine::priv::lua::resources::setCurrentSceneLUA(Engine::priv::SceneLUABinder scene) {
    Engine::Resources::setCurrentScene(&scene.getScene());
}
void Engine::priv::lua::resources::setCurrentSceneByNameLUA(const std::string& sceneName) {
    Engine::Resources::setCurrentScene(sceneName);
}

Handle Engine::priv::lua::resources::getResourceLUA(const std::string& resourceType, const std::string& resourceName) {
    if (resourceType == "Mesh") {
        return Engine::Resources::getResource<Mesh>(resourceName);
    } else if (resourceType == "Material") {
        return Engine::Resources::getResource<Material>(resourceName);
    } else if (resourceType == "Texture") {
        return Engine::Resources::getResource<Texture>(resourceName);
    } else if (resourceType == "TextureCubemap") {
        return Engine::Resources::getResource<TextureCubemap>(resourceName);
    } else if (resourceType == "Shader") {
        return Engine::Resources::getResource<Shader>(resourceName);
    } else if (resourceType == "ShaderProgram") {
        return Engine::Resources::getResource<ShaderProgram>(resourceName);
    } else if (resourceType == "SoundData") {
        return Engine::Resources::getResource<SoundData>(resourceName);
    } else if (resourceType == "Font") {
        return Engine::Resources::getResource<Font>(resourceName);
    }
    return Handle{};
}