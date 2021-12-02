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
class  SystemSceneChanging;

#include <serenity/renderer/GLImageConstants.h>
#include <serenity/resources/shader/ShaderIncludes.h>
#include <serenity/resources/ResourceModule.h>
#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/scene/Scene.h>
#include <serenity/types/Types.h>

namespace Engine::priv {
    class ResourceManager final{
        friend class  ::Scene;
        friend class  ::SystemSceneChanging;
        public:
            struct SceneSwapData {
                Scene*  oldScene  = nullptr;
                Scene*  newScene  = nullptr;
            };
            static Engine::view_ptr<ResourceManager> RESOURCE_MANAGER;
        public:
            ResourceModule                         m_ResourceModule;
            std::vector<std::unique_ptr<Window>>   m_Windows;
            std::vector<Scene*>                    m_Scenes;
            std::vector<Scene*>                    m_ScenesToBeDeleted;
            SceneSwapData                          m_SceneSwap;
            Scene*                                 m_CurrentScene       = nullptr;
        public:
            ResourceManager(const EngineOptions&);
            ~ResourceManager();

            void postUpdate();

            void init(const EngineOptions&);

            template<class TResource, class ... ARGS>
            [[nodiscard]] inline Handle addResource(ARGS&&... args) { return m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...); }

            [[nodiscard]] inline std::mutex& getMutex() const noexcept { return m_ResourceModule.m_Mutex; }

            [[nodiscard]] Engine::view_ptr<Scene> getSceneByID(uint32_t sceneID);

            [[nodiscard]] inline constexpr std::vector<Scene*>& scenes() noexcept { return m_Scenes; }

            template<class TResource>
            [[nodiscard]] inline std::list<Engine::view_ptr<TResource>> GetAllResourcesOfType() noexcept {
                return m_ResourceModule.getAllResourcesOfType<TResource>();
            }


    };
};
namespace Engine::Resources {
    [[nodiscard]] Engine::view_ptr<Scene> getCurrentScene();
    bool setCurrentScene(Scene* scene);
    bool setCurrentScene(std::string_view sceneName);

    template<class T, class ... ARGS>
    T& addScene(ARGS&&... args) {
        auto& mgr = *Engine::priv::ResourceManager::RESOURCE_MANAGER;
        for (uint32_t i = 0; i < mgr.m_Scenes.size(); ++i) {
            if (mgr.m_Scenes[i] == nullptr) {
                mgr.m_Scenes[i] = NEW T{ i, std::forward<ARGS>(args)... };
                return *static_cast<T*>(mgr.m_Scenes[i]);
            }
        }
        mgr.m_Scenes.emplace_back( NEW T{ uint32_t(mgr.m_Scenes.size()), std::forward<ARGS>(args)... } );
        return *static_cast<T*>(mgr.m_Scenes.back());
    }

    [[nodiscard]] std::mutex& getMutex() noexcept;

    [[nodiscard]] double dt();
    [[nodiscard]] double timeScale();
    [[nodiscard]] double applicationTime();

    [[nodiscard]] Window& getWindow();
    [[nodiscard]] glm::uvec2 getWindowSize();

    [[nodiscard]] Window& getWindow(uint32_t index);
    [[nodiscard]] glm::uvec2 getWindowSize(uint32_t index);
    [[nodiscard]] size_t getNumWindows();

    [[nodiscard]] Engine::view_ptr<Scene> getScene(std::string_view sceneName);
    bool deleteScene(std::string_view sceneName);
    bool deleteScene(Scene&);

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

    template<class TResource>
    [[nodiscard]] inline LoadedResource<TResource> getResource(std::string_view name) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(name);
    }
    template<class TResource>
    [[nodiscard]] inline Engine::view_ptr<TResource> getResource(Handle inHandle) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(inHandle);
    }

    template<class TResource>
    [[nodiscard]] inline std::list<Engine::view_ptr<TResource>> GetAllResourcesOfType() noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.getAllResourcesOfType<TResource>();
    }

    template<class TResource, class ... ARGS>
    [[nodiscard]] inline Handle addResource(ARGS&&... args) {
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
        std::string_view normal     = {},
        std::string_view glow       = {},
        std::string_view specular   = {},
        std::string_view ao         = {},
        std::string_view metalness  = {},
        std::string_view smoothness = {}
    );
    [[nodiscard]] Handle loadMaterialAsync(
        std::string_view name,
        std::string_view diffuse,
        std::string_view normal     = {},
        std::string_view glow       = {},
        std::string_view specular   = {},
        std::string_view ao         = {},
        std::string_view metalness  = {},
        std::string_view smoothness = {},
        Engine::ResourceCallback callback = [](Handle) {}
    );
    [[nodiscard]] Handle loadMaterial(std::string_view name, Handle diffuse,
        Handle normal     = {},
        Handle glow       = {},
        Handle specular   = {},
        Handle ao         = {},
        Handle metalness  = {},
        Handle smoothness = {}
    );

    [[nodiscard]] Handle addShader(std::string_view shaderFileOrData, ShaderType shaderType, bool fromFile = true);

    [[nodiscard]] Handle addShaderProgram(std::string_view name, Handle vertexShader, Handle fragmentShader);
};
namespace Engine::priv::lua::resources {

}


namespace Engine::Data {
    [[nodiscard]] std::string reportTime();
};

#endif