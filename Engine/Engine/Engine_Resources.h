#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include "Engine_Physics.h"
#include "ShaderProgram.h"
#include "GLImageConstants.h"

typedef unsigned int uint;
typedef std::uint32_t uint32;

class Engine_Window;
class Scene;
class Font;
class Texture;
class Camera;
class Mesh;
class Material;
class SunLight;
class SoundData;

class ResourceType final {public: enum Type {
    Empty, //do NOT move this specific enum
    Texture,
    Mesh,
    Material,
    Sound,
    Object,
    Font,
    Camera,
    Shader,
    ShaderProgram,
    SoundData,
    Scene,
_TOTAL};};

struct Handle final {
    uint32 index : 12;
    uint32 counter : 15;
    uint32 type : 5;
    Handle();
    Handle(uint32 _index, uint32 _counter, uint32 _type);
    inline operator uint32() const;
    const bool null() const;
    const EngineResource* get() const;
    inline const EngineResource* operator ->() const;
};

namespace Engine{
    namespace epriv{
        class ResourceManager final: Engine::epriv::noncopyable{
            public:
                class impl; std::unique_ptr<impl> m_i;

                ResourceManager(const char* name,uint width,uint height);
                ~ResourceManager();

                void _init(const char* name,uint width,uint height);
 
                Handle _addTexture(Texture*);

                bool _hasScene(std::string);
                void _addScene(Scene*);
                std::string _buildSceneName(std::string);
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

        Engine_Window* getWindow();
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

        Handle addMesh(std::string file, bool fromFile = true,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float x, float y, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length,float threshhold = 0.0005f);
        Handle addMeshAsync(std::string file,bool fromFile = true,float threshhold = 0.0005f);

        Handle addTexture(std::string file,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8,bool mipmaps = false);

        Handle addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "");
        Handle addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr);

        Handle addShader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        Handle addSoundData(std::string file,std::string name = "",bool music = false);
        Handle addShaderProgram(std::string name, Shader* vertexShader, Shader* fragmentShader);
        Handle addShaderProgram(std::string name, Handle& vertexShader, Handle& fragmentShader);
    };
    namespace Data{
        std::string reportTime();
    };
};
#endif