#pragma once
#ifndef ENGINE_RESOURCES_H
#define ENGINE_RESOURCES_H

namespace sf {
    class Image;
};
namespace Engine::priv {
    class ResourceManager;
};

class  Handle;
struct EngineOptions;
class  Resource;
class  Window;
class  Scene;
class  Font;
class  Texture;
class  Camera;
class  Mesh;
class  Material;
class  SunLight;
class  SoundData;
class  Shader;
class  ShaderProgram;

#include <serenity/core/engine/renderer/GLImageConstants.h>
#include <serenity/core/engine/shaders/ShaderIncludes.h>
#include <serenity/core/engine/resources/ResourceModule.h>
#include <serenity/core/engine/mesh/MeshIncludes.h>
#include <serenity/core/engine/mesh/MeshRequest.h>

namespace Engine::priv {
    class ResourceManager final{
        friend class  Scene;
        public:
            static Engine::view_ptr<ResourceManager> RESOURCE_MANAGER;
        private:
            unsigned int AddScene(Scene& scene);
        public:
            ResourceModule                        m_ResourceModule;
            std::vector<std::unique_ptr<Window>>  m_Windows;
            std::vector<std::unique_ptr<Scene>>   m_Scenes;
            std::vector<Scene*>                   m_ScenesToBeDeleted;
            Scene*                                m_CurrentScene = nullptr;
        public:
            ResourceManager(const EngineOptions& engineOptions);

            void postUpdate();

            void init(const EngineOptions& engineOptions);

            template<typename TResource, typename ... ARGS>
            Handle addResource(ARGS&&... args) {
                return m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...);
            }


            Engine::view_ptr<Scene> getSceneByID(uint32_t sceneID);

            inline constexpr std::vector<std::unique_ptr<Scene>>& scenes() noexcept { return m_Scenes; }

            template<typename T> 
            std::list<Engine::view_ptr<T>> GetAllResourcesOfType() noexcept {
                return m_ResourceModule.getAllResourcesOfType<T>();
            }


    };
};
namespace Engine::Resources {
    Engine::view_ptr<Scene> getCurrentScene();
    bool setCurrentScene(Scene* scene);
    bool setCurrentScene(std::string_view sceneName);

    float dt();
    double timeScale();
    double applicationTime();

    Window& getWindow();
    glm::uvec2 getWindowSize();

    Window& getWindow(unsigned int index);
    glm::uvec2 getWindowSize(unsigned int index);

    Engine::view_ptr<Scene> getScene(std::string_view sceneName);
    bool deleteScene(std::string_view sceneName);
    bool deleteScene(Scene& scene);

    std::vector<Handle> loadMesh(
        const std::string& fileOrData, 
        float threshhold = MESH_DEFAULT_THRESHOLD,
        MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG
    );


    std::vector<Handle> loadMeshAsync(
        const std::string& fileOrData,
        float threshhold = MESH_DEFAULT_THRESHOLD,
        MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG,
        MeshRequestCallback callback = [](const std::vector<Handle>&) {}
    );

    template<typename TResource>
    std::pair<Engine::view_ptr<TResource>, Handle> getResource(std::string_view name) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(name);
    }
    template<typename TResource>
    Engine::view_ptr<TResource> getResource(Handle inHandle) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(inHandle);
    }

    template<typename TResource, typename ... ARGS>
    Handle addResource(ARGS&&... args) {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...);
    }

    Handle loadTexture(
        const std::string& file,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false
    );
    Handle loadTexture(
        sf::Image& sfImage,
        const std::string& texture_name,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false
    );
    Handle loadTextureAsync(
        const std::string& file,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false,
        std::function<void()> callback = []() {}
    );
    Handle loadTextureAsync(
        sf::Image& sfImage,
        const std::string& texture_name,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false,
        std::function<void()> callback = []() {}
    );
    Handle loadMaterial(
        const std::string& name, 
        const std::string& diffuse, 
        const std::string& normal     = "", 
        const std::string& glow       = "", 
        const std::string& specular   = "",
        const std::string& ao         = "",
        const std::string& metalness  = "",
        const std::string& smoothness = ""
    );
    Handle loadMaterialAsync(
        const std::string& name,
        const std::string& diffuse,
        const std::string& normal     = "",
        const std::string& glow       = "",
        const std::string& specular   = "",
        const std::string& ao         = "",
        const std::string& metalness  = "",
        const std::string& smoothness = "",
        std::function<void()> callback = []() {}
    );
    Handle loadMaterial(
        const std::string& name,
        Handle diffuse,
        Handle normal     = Handle{},
        Handle glow       = Handle{},
        Handle specular   = Handle{},
        Handle ao         = Handle{},
        Handle metalness  = Handle{},
        Handle smoothness = Handle{}
    );

    Handle addShader(const std::string& shaderFileOrData, ShaderType shaderType, bool fromFile = true);

    Handle addShaderProgram(const std::string& name, Handle vertexShader, Handle fragmentShader);
};
namespace Engine::Data{
    std::string reportTime();
};

#endif