#pragma once
#ifndef ENGINE_RESOURCES_H
#define ENGINE_RESOURCES_H

namespace sf {
    class Image;
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
#include <core/engine/resources/ResourcePool.h>

namespace Engine::priv {
    class ResourceManager final{
        friend class  Scene;
        private:
            unsigned int AddScene(Scene& scene);
        public:
            //http://gamesfromwithin.com/managing-data-relationships
            ResourcePool<Resource>                m_ResourcePool;
            std::vector<std::unique_ptr<Window>>  m_Windows;
            std::vector<std::unique_ptr<Scene>>   m_Scenes;
            std::vector<Scene*>                   m_ScenesToBeDeleted;
            Scene*                                m_CurrentScene = nullptr;
        public:
            ResourceManager(const EngineOptions& engineOptions);
            ~ResourceManager();

            void cleanup();

            void onPostUpdate();

            void init(const EngineOptions& engineOptions);
 
            Handle _addTexture(Texture* texture);
            Scene& _getSceneByID(std::uint32_t sceneID);

            inline CONSTEXPR std::vector<std::unique_ptr<Scene>>& scenes() noexcept { return m_Scenes; }

            template<typename T> T* HasResource(std::string_view resource_name) noexcept {
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

            template<typename T> std::list<T*> GetAllResourcesOfType() noexcept {
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

    void     getShader(Handle inHandle, Shader*& outPtr);                  
    Shader*  getShader(Handle inHandle);

    void       getSoundData(Handle inHandle, SoundData*& outPtr);            
    SoundData* getSoundData(Handle inHandle);

    void     getCamera(Handle inHandle, Camera*& outPtr);                  
    Camera*  getCamera(Handle inHandle);

    void  getFont(Handle inHandle, Font*& outPtr);                      
    Font* getFont(Handle inHandle);
    Font* getFont(std::string_view name);

    void     getTexture(Handle inHandle, Texture*& outPtr);
    Texture* getTexture(Handle inHandle);
    Texture* getTexture(std::string_view name);

    void  getMesh(Handle inHandle, Mesh*& outPtr);
    Mesh* getMesh(Handle inHandle);
    Mesh* getMesh(std::string_view name);

    void      getMaterial(Handle inHandle, Material*& outPtr);
    Material* getMaterial(Handle inHandle);
    Material* getMaterial(std::string_view name);

    void           getShaderProgram(Handle inHandle, ShaderProgram*& outPtr);
    ShaderProgram* getShaderProgram(Handle inHandle);
    ShaderProgram* getShaderProgram(std::string_view name);

    Handle addFont(const std::string& filename, int height, int width = 0, float line_height = 8.0f);

    std::vector<Handle> loadMesh(
        const std::string& fileOrData, 
        float threshhold = 0.005f
    );
    std::vector<Handle> loadMeshAsync(
        const std::string& fileOrData, 
        float threshhold = 0.005f,
        std::function<void()> callback = []() {}
    );

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
        Texture* diffuse,
        Texture* normal     = nullptr,
        Texture* glow       = nullptr,
        Texture* specular   = nullptr,
        Texture* ao         = nullptr,
        Texture* metalness  = nullptr,
        Texture* smoothness = nullptr
    );

    Handle addShader(const std::string& shaderFileOrData, ShaderType shaderType, bool fromFile = true);
    Handle addSoundData(const std::string& file);
    Handle addShaderProgram(const std::string& name, Shader& vertexShader, Shader& fragmentShader);
    Handle addShaderProgram(const std::string& name, Handle vertexShader, Handle fragmentShader);
};
namespace Engine::Data{
    std::string reportTime();
};

#endif