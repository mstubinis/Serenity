#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include "core/engine/Engine_Physics.h"
#include "core/engine/renderer/GLImageConstants.h"
#include "core/ShaderProgram.h"
#include "core/engine/Engine_ObjectPool.h"

typedef std::uint32_t uint;

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

                ResourceManager(const char* name,uint width,uint height);
                ~ResourceManager();

                void _init(const char* name,uint width,uint height);
 
                Handle _addTexture(Texture*);
                Scene& _getSceneByID(uint id);

                std::vector<Scene*>& scenes();

                bool _hasScene(std::string);
                void _addScene(Scene&);
                Texture* _hasTexture(std::string);
                uint _numScenes();
        };
    };
    namespace Resources{
        namespace Settings{
            void enableDynamicMemory(bool enable = true);
            void disableDynamicMemory();
        }

        Scene* getCurrentScene();
        void setCurrentScene(Scene* scene);
        void setCurrentScene(std::string sceneName);

        const double dt();

        Engine_Window& getWindow();
        glm::uvec2 getWindowSize();

        Scene* getScene(std::string sceneName);

        Font* getFont(std::string);

        void getShader(Handle& inHandle,Shader*& outPtr);         Shader* getShader(Handle& inHandle);
        void getSoundData(Handle& inHandle,SoundData*& outPtr);   SoundData* getSoundData(Handle& inHandle);
        void getCamera(Handle& inHandle,Camera*& outPtr);         Camera* getCamera(Handle& inHandle);
        void getFont(Handle& inHandle,Font*& outPtr);             Font* getFont(Handle& inHandle);
        void getTexture(Handle& inHandle,Texture*& outPtr);       Texture* getTexture(Handle& inHandle);
        void getMesh(Handle& inHandle,Mesh*& outPtr);             Mesh* getMesh(Handle& inHandle);
        void getMaterial(Handle& inHandle,Material*& outPtr);     Material* getMaterial(Handle& inHandle);
        void getShaderProgram(Handle& inHandle,ShaderP*& outPtr); ShaderP* getShaderProgram(Handle& inHandle);

        Handle addFont(std::string filename);

        std::vector<Handle> loadMesh(std::string fileOrData,  float threshhold = 0.0005f);
        std::vector<Handle> loadMeshAsync(std::string fileOrData,  float threshhold = 0.0005f);

        Handle addTexture(std::string file,ImageInternalFormat::Format = ImageInternalFormat::SRGB8_ALPHA8,bool mipmaps = false);

        Handle addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "");
        Handle addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr);

        Handle addShader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        Handle addSoundData(std::string file,std::string name = "",bool music = false);
        Handle addShaderProgram(std::string name, Shader& vertexShader, Shader& fragmentShader);
        Handle addShaderProgram(std::string name, Handle& vertexShader, Handle& fragmentShader);
    };
    namespace Data{
        std::string reportTime();
    };
};
#endif