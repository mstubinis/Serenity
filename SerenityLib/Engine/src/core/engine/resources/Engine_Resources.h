#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/renderer/GLImageConstants.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/Engine_ObjectPool.h>
#include <core/engine/utils/Utils.h>

#include <glm/vec2.hpp>

struct Handle;
class  EngineResource;
class  Engine_Window;
class  Scene;
class  Font;
class  Texture;
class  Camera;
class  Mesh;
class  Material;
class  SunLight;
class  SoundData;

namespace Engine{
    namespace epriv{
        class ResourceManager final{
            public:
                //http://gamesfromwithin.com/managing-data-relationships
                ObjectPool<EngineResource>*                    m_Resources;
                Engine_Window*                                 m_Window;
                Scene*                                         m_CurrentScene;
                bool                                           m_DynamicMemory;
                std::vector<Scene*>                            m_Scenes;
                std::vector<Scene*>                            m_ScenesToBeDeleted;

                ResourceManager(const char* name, const uint& width, const uint& height);
                ~ResourceManager();

                void onPostUpdate();

                void _init(const char* name, const uint& width, const uint& height);
 
                Handle _addTexture(Texture*);
                Scene& _getSceneByID(const uint& id);

                std::vector<Scene*>& scenes();

                bool _hasScene(const std::string&);
                void _addScene(Scene&);
                Texture* _hasTexture(const std::string&);
                const size_t _numScenes();
        };
    };
    namespace Resources{
        namespace Settings{
            void enableDynamicMemory(const bool enable = true);
            void disableDynamicMemory();
        }

        Scene* getCurrentScene();
        void setCurrentScene(Scene* scene);
        void setCurrentScene(const std::string& sceneName);

        const double dt();

        Engine_Window& getWindow();
        glm::uvec2 getWindowSize();

        Scene* getScene(const std::string& sceneName);
        const bool deleteScene(const std::string& sceneName);

        void getShader(Handle& inHandle,Shader*& outPtr);           Shader*    getShader(Handle& inHandle);
        void getSoundData(Handle& inHandle,SoundData*& outPtr);     SoundData* getSoundData(Handle& inHandle);
        void getCamera(Handle& inHandle,Camera*& outPtr);           Camera*    getCamera(Handle& inHandle);
        void getFont(Handle& inHandle,Font*& outPtr);               Font*      getFont(Handle& inHandle);
        void getTexture(Handle& inHandle,Texture*& outPtr);         Texture*   getTexture(Handle& inHandle);
        void getMesh(Handle& inHandle,Mesh*& outPtr);               Mesh*      getMesh(Handle& inHandle);
        void getMaterial(Handle& inHandle,Material*& outPtr);       Material*  getMaterial(Handle& inHandle);
        void getShaderProgram(Handle& inHandle,ShaderProgram*& outPtr);   ShaderProgram*   getShaderProgram(Handle& inHandle);

        Handle addFont(const std::string& filename);

        std::vector<Handle> loadMesh(const std::string& fileOrData, const float& threshhold = 0.0005f);
        std::vector<Handle> loadMeshAsync(const std::string& fileOrData, const float& threshhold = 0.0005f);

        Handle loadTexture(const std::string& file, const ImageInternalFormat::Format & = ImageInternalFormat::SRGB8_ALPHA8, const bool& mipmaps = false);
        Handle loadTextureAsync(const std::string& file, const ImageInternalFormat::Format & = ImageInternalFormat::SRGB8_ALPHA8, const bool& mipmaps = false);

        Handle addTexture(const std::string& file, const ImageInternalFormat::Format& = ImageInternalFormat::SRGB8_ALPHA8, const bool& mipmaps = false);

        Handle addMaterial(
            const std::string& name, 
            const std::string& diffuse, 
            const std::string& normal = "", 
            const std::string& glow = "", 
            const std::string& specular = "",
            const std::string& ao = "",
            const std::string& metalness = "",
            const std::string& smoothness = ""
        );
        Handle addMaterial(
            const std::string& name,
            Texture* diffuse,
            Texture* normal = nullptr,
            Texture* glow = nullptr,
            Texture* specular = nullptr,
            Texture * ao = nullptr,
            Texture * metalness = nullptr,
            Texture * smoothness = nullptr
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
            const std::string & ao = "",
            const std::string & metalness = "",
            const std::string & smoothness = ""
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