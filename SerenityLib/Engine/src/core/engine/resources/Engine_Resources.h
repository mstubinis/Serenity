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

#include <core/engine/renderer/GLImageConstants.h>
#include <core/engine/shaders/ShaderIncludes.h>
#include <core/engine/resources/ResourceModule.h>

namespace Engine::priv {
    class ResourceManager final{
        friend class  Scene;
        public:
            static ResourceManager* RESOURCE_MANAGER;
        private:
            unsigned int AddScene(Scene& scene);
        public:
            ResourceModule m_ResourceModule;

            //http://gamesfromwithin.com/managing-data-relationships
            //ResourcePool<Resource>                m_ResourcePool;

            std::vector<std::unique_ptr<Window>>  m_Windows;
            std::vector<std::unique_ptr<Scene>>   m_Scenes;
            std::vector<Scene*>                   m_ScenesToBeDeleted;
            Scene*                                m_CurrentScene = nullptr;
        public:
            ResourceManager(const EngineOptions& engineOptions);

            void onPostUpdate();

            void init(const EngineOptions& engineOptions);

            template<typename TResource, typename ... ARGS>
            Handle addResource(ARGS&&... args) {
                return m_ResourceModule.emplace<TResource>(std::forward<ARGS>(args)...);
            }


            Scene& _getSceneByID(std::uint32_t sceneID);

            inline CONSTEXPR std::vector<std::unique_ptr<Scene>>& scenes() noexcept { return m_Scenes; }
            /*
            template<typename T> 
            T* HasResource(std::string_view resource_name) noexcept {
                for (size_t i = 0; i < m_ResourcePool.size(); ++i) {
                    Resource* r = m_ResourcePool.getAsFast<Resource>((unsigned int)i + 1U);
                    if (r) {
                        T* resource = dynamic_cast<T*>(r);
                        if (resource && resource->name() == resource_name) {
                            return resource;
                        }
                    }
                }
                return nullptr;
            }
            */
            template<typename T> 
            std::list<T*> GetAllResourcesOfType() noexcept {
                return m_ResourceModule.getAllResourcesOfType<T>();
                /*
                std::list<T*> ret;
                for (size_t i = 0; i < m_ResourcePool.size(); ++i) {
                    Resource* r = m_ResourcePool.getAsFast<Resource>((unsigned int)i + 1U);
                    if (r) {
                        T* resource = dynamic_cast<T*>(r);
                        if (resource) {
                            ret.emplace_back(resource);
                        }
                    }
                }
                return ret;
                */
            }


    };
};
namespace Engine::Resources {
    Scene* getCurrentScene();
    bool setCurrentScene(Scene* scene);
    bool setCurrentScene(std::string_view sceneName);

    float dt();
    double timeScale();
    double applicationTime();

    Window& getWindow();
    glm::uvec2 getWindowSize();

    Window& getWindow(unsigned int index);
    glm::uvec2 getWindowSize(unsigned int index);

    Scene* getScene(std::string_view sceneName);
    bool deleteScene(std::string_view sceneName);
    bool deleteScene(Scene& scene);

    std::vector<Handle> loadMesh(
        const std::string& fileOrData, 
        float threshhold = 0.005f
    );
    std::vector<Handle> loadMeshAsync(
        const std::string& fileOrData, 
        float threshhold = 0.005f,
        std::function<void()> callback = []() {}
    );

    template<typename TResource>
    std::pair<TResource*, Handle> getResource(std::string_view name) noexcept {
        return Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(name);
    }
    template<typename TResource>
    void getResource(Handle inHandle, TResource*& outPtr) noexcept {
        outPtr = Engine::priv::ResourceManager::RESOURCE_MANAGER->m_ResourceModule.get<TResource>(inHandle);
    }
    template<typename TResource>
    TResource* getResource(Handle inHandle) noexcept {
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