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

#include <serenity/renderer/GLImageConstants.h>
#include <serenity/resources/shader/ShaderIncludes.h>
#include <serenity/resources/ResourceModule.h>
#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/types/Types.h>

namespace Engine::priv {
    class ResourceManager final{
        friend class  Scene;
        public:
            static Engine::view_ptr<ResourceManager> RESOURCE_MANAGER;
        private:
            uint32_t AddScene(Scene& scene);
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
            [[nodiscard]] Handle addResource(ARGS&&... args) {
                return m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...);
            }


            [[nodiscard]] Engine::view_ptr<Scene> getSceneByID(uint32_t sceneID);

            [[nodiscard]] inline constexpr std::vector<std::unique_ptr<Scene>>& scenes() noexcept { return m_Scenes; }

            template<typename T> 
            [[nodiscard]] std::list<Engine::view_ptr<T>> GetAllResourcesOfType() noexcept {
                return m_ResourceModule.getAllResourcesOfType<T>();
            }


    };
};
namespace Engine::Resources {
    [[nodiscard]] Engine::view_ptr<Scene> getCurrentScene();
    bool setCurrentScene(Scene* scene);
    bool setCurrentScene(std::string_view sceneName);

    [[nodiscard]] double dt();
    [[nodiscard]] double timeScale();
    [[nodiscard]] double applicationTime();

    [[nodiscard]] Window& getWindow();
    [[nodiscard]] glm::uvec2 getWindowSize();

    [[nodiscard]] Window& getWindow(uint32_t index);
    [[nodiscard]] glm::uvec2 getWindowSize(uint32_t index);

    [[nodiscard]] Engine::view_ptr<Scene> getScene(std::string_view sceneName);
    bool deleteScene(std::string_view sceneName);
    bool deleteScene(Scene& scene);

    [[nodiscard]] std::vector<Handle> loadMesh(
        std::string_view fileOrData,
        float threshhold = MESH_DEFAULT_THRESHOLD,
        MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG
    );


    [[nodiscard]] std::vector<Handle> loadMeshAsync(
        std::string_view fileOrData,
        float threshhold = MESH_DEFAULT_THRESHOLD,
        MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG,
        MeshRequestCallback callback = [](const std::vector<Handle>&) {}
    );

    template<typename TResource>
    [[nodiscard]] LoadedResource<TResource> getResource(std::string_view name) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(name);
    }
    template<typename TResource>
    [[nodiscard]] Engine::view_ptr<TResource> getResource(Handle inHandle) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(inHandle);
    }

    template<typename TResource, typename ... ARGS>
    [[nodiscard]] Handle addResource(ARGS&&... args) {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...);
    }

    [[nodiscard]] Handle loadTexture(
        std::string_view file,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false
    );
    [[nodiscard]] Handle loadTexture(
        sf::Image& sfImage,
        std::string_view texture_name,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false
    );
    [[nodiscard]] Handle loadTextureAsync(
        std::string_view file,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false,
        Engine::ResourceCallback callback = [](Handle) {}
    );
    [[nodiscard]] Handle loadTextureAsync(
        sf::Image& sfImage,
        std::string_view texture_name,
        ImageInternalFormat = ImageInternalFormat::SRGB8_ALPHA8,
        bool mipmaps        = false,
        Engine::ResourceCallback callback = [](Handle) {}
    );
    [[nodiscard]] Handle loadMaterial(
        std::string_view name,
        std::string_view diffuse,
        std::string_view normal     = "",
        std::string_view glow       = "",
        std::string_view specular   = "",
        std::string_view ao         = "",
        std::string_view metalness  = "",
        std::string_view smoothness = ""
    );
    [[nodiscard]] Handle loadMaterialAsync(
        std::string_view name,
        std::string_view diffuse,
        std::string_view normal     = "",
        std::string_view glow       = "",
        std::string_view specular   = "",
        std::string_view ao         = "",
        std::string_view metalness  = "",
        std::string_view smoothness = "",
        Engine::ResourceCallback callback = [](Handle) {}
    );
    [[nodiscard]] Handle loadMaterial(
        std::string_view name,
        Handle diffuse,
        Handle normal     = Handle{},
        Handle glow       = Handle{},
        Handle specular   = Handle{},
        Handle ao         = Handle{},
        Handle metalness  = Handle{},
        Handle smoothness = Handle{}
    );

    [[nodiscard]] Handle addShader(std::string_view shaderFileOrData, ShaderType shaderType, bool fromFile = true);

    [[nodiscard]] Handle addShaderProgram(std::string_view name, Handle vertexShader, Handle fragmentShader);
};
namespace Engine::Data{
    [[nodiscard]] std::string reportTime();
};

#endif