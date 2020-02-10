#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

namespace sf {
    class Image;
};

struct Handle;
struct EngineOptions;
class  EngineResource;
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
#include <core/engine/utils/Utils.h>

#include <glm/vec2.hpp>

namespace Engine{
    namespace priv{
        class ResourceManager final{
            friend class  Scene;
            private:
                const unsigned int AddScene(Scene&);
            public:
                //http://gamesfromwithin.com/managing-data-relationships
                ResourcePool<EngineResource>       m_Resources;
                std::vector<Window*>               m_Windows;
                Scene*                             m_CurrentScene;
                bool                               m_DynamicMemory;
                std::vector<Scene*>                m_Scenes;
                std::vector<Scene*>                m_ScenesToBeDeleted;
            public:

                ResourceManager(const EngineOptions& options);
                ~ResourceManager();

                void cleanup();

                void onPostUpdate();

                void _init(const EngineOptions& options);
 
                Handle _addTexture(Texture*);
                Scene& _getSceneByID(const uint& id);

                std::vector<Scene*>& scenes();

                template<typename T> T* HasResource(const std::string& resource_name) {
                    for (size_t i = 0; i < m_Resources.size(); ++i) {
                        EngineResource* r = m_Resources.getAsFast<EngineResource>(static_cast<unsigned int>(i) + 1U);
                        if (r) {
                            T* resource = dynamic_cast<T*>(r);
                            if (resource && resource->name() == resource_name) {
                                return resource;
                            }
                        }
                    }
                    return nullptr;
                }
        };
    };
    namespace Resources{
        namespace Settings{
            void enableDynamicMemory(const bool enable = true);
            void disableDynamicMemory();
        }

        Scene* getCurrentScene();
        const bool setCurrentScene(Scene* scene);
        const bool setCurrentScene(const std::string& sceneName);

        const double dt();
        const double timeScale();
        const double applicationTime();

        Window& getWindow();
        glm::uvec2 getWindowSize();

        Window& getWindow(const unsigned int& index);
        glm::uvec2 getWindowSize(const unsigned int& index);

        Scene* getScene(const std::string& sceneName);
        const bool deleteScene(const std::string& sceneName);
        const bool deleteScene(Scene& scene);

        void getShader(Handle& inHandle,Shader*& outPtr);                  Shader*    getShader(Handle& inHandle);
        void getSoundData(Handle& inHandle,SoundData*& outPtr);            SoundData* getSoundData(Handle& inHandle);
        void getCamera(Handle& inHandle,Camera*& outPtr);                  Camera*    getCamera(Handle& inHandle);
        void getFont(Handle& inHandle,Font*& outPtr);                      Font*      getFont(Handle& inHandle);

        void     getTexture(Handle& inHandle, Texture*& outPtr);
        Texture* getTexture(Handle& inHandle);
        Texture* getTexture(const std::string& name);

        void getMesh(Handle& inHandle,Mesh*& outPtr);                      Mesh*      getMesh(Handle& inHandle);

        void      getMaterial(Handle& inHandle,Material*& outPtr);
        Material* getMaterial(Handle& inHandle);

        void getShaderProgram(Handle& inHandle,ShaderProgram*& outPtr);    ShaderProgram*   getShaderProgram(Handle& inHandle);


        Handle addFont(const std::string& filename);

        std::vector<Handle> loadMesh(const std::string& fileOrData, const float& threshhold = 0.0005f);
        std::vector<Handle> loadMeshAsync(const std::string& fileOrData, const float& threshhold = 0.0005f);

        Handle loadTexture(
            const std::string& file,
            const ImageInternalFormat::Format& = ImageInternalFormat::SRGB8_ALPHA8,
            const bool& mipmaps = false
        );
        Handle loadTextureAsync(
            const std::string& file,
            const ImageInternalFormat::Format& = ImageInternalFormat::SRGB8_ALPHA8,
            const bool& mipmaps = false
        );
        Handle loadTextureAsync(
            sf::Image& sfImage,
            const std::string& texture_name,
            const ImageInternalFormat::Format& = ImageInternalFormat::SRGB8_ALPHA8,
            const bool& mipmaps = false
        );
        Handle loadMaterial(
            const std::string& name, 
            const std::string& diffuse, 
            const std::string& normal = "", 
            const std::string& glow = "", 
            const std::string& specular = "",
            const std::string& ao = "",
            const std::string& metalness = "",
            const std::string& smoothness = ""
        );
        Handle loadMaterialAsync(
            const std::string& name,
            const std::string& diffuse,
            const std::string& normal = "",
            const std::string& glow = "",
            const std::string& specular = "",
            const std::string& ao = "",
            const std::string& metalness = "",
            const std::string& smoothness = ""
        );
        Handle loadMaterial(
            const std::string& name,
            Texture* diffuse,
            Texture* normal = nullptr,
            Texture* glow = nullptr,
            Texture* specular = nullptr,
            Texture* ao = nullptr,
            Texture* metalness = nullptr,
            Texture* smoothness = nullptr
        );

        Handle addShader(const std::string& shaderFileOrData, const ShaderType::Type& shaderType, const bool& fromFile = true);
        Handle addSoundData(const std::string& file);
        Handle addShaderProgram(const std::string& name, Shader& vertexShader, Shader& fragmentShader);
        Handle addShaderProgram(const std::string& name, Handle& vertexShader, Handle& fragmentShader);
    };
    namespace Data{
        std::string reportTime();
    };
};
#endif