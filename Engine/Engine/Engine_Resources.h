#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include "Engine_ResourceHandle.h"
#include "Object.h"
#include "Engine_Physics.h"
#include "ShaderProgram.h"

#include <unordered_map>
#include <map>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

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
class MeshInstance;
class SoundData;

template <typename E> void vector_clear(std::vector<E>& t){ t.clear(); std::vector<E>().swap(t); t.shrink_to_fit(); }
template <typename E> std::string to_string(E t){ return boost::lexical_cast<std::string>(t); }

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for deleting a pointer

namespace Engine{
	namespace epriv{
		class ResourceManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ResourceManager(const char* name,uint width,uint height);
				~ResourceManager();

				void _init(const char* name,uint width,uint height);
		
				Handle _addTexture(Texture*);

				bool _hasObject(std::string);        void _addObject(Object*);              std::string _buildObjectName(std::string);
				bool _hasScene(std::string);         void _addScene(Scene*);                std::string _buildSceneName(std::string);
				bool _hasMeshInstance(std::string);  void _addMeshInstance(MeshInstance*);  std::string _buildMeshInstanceName(std::string);

				void _remObject(std::string);

				void _resizeCameras(uint w,uint h);
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

        float& dt();

        Engine_Window* getWindow();
        glm::uvec2 getWindowSize();

        Scene* getScene(std::string sceneName);

        boost::shared_ptr<Object>& getObjectPtr(std::string);

        Object* getObject(std::string);
        Font* getFont(std::string);
        MeshInstance* getMeshInstance(std::string);

        void getShader(Handle& inHandle,Shader*& outPtr);         Shader* getShader(Handle& inHandle);
        void getSoundData(Handle& inHandle,SoundData*& outPtr);   SoundData* getSoundData(Handle& inHandle);
		void getObject(Handle& inHandle,Object*& outPtr);         Object* getObject(Handle& inHandle);
        void getCamera(Handle& inHandle,Camera*& outPtr);         Camera* getCamera(Handle& inHandle);
        void getFont(Handle& inHandle,Font*& outPtr);             Font* getFont(Handle& inHandle);
        void getTexture(Handle& inHandle,Texture*& outPtr);       Texture* getTexture(Handle& inHandle);
        void getMesh(Handle& inHandle,Mesh*& outPtr);             Mesh* getMesh(Handle& inHandle);
        void getMaterial(Handle& inHandle,Material*& outPtr);     Material* getMaterial(Handle& inHandle);
        void getShaderProgram(Handle& inHandle,ShaderP*& outPtr); ShaderP* getShaderProgram(Handle& inHandle);

		Handle addFont(std::string filename);

        Handle addMesh(std::string file, CollisionType::Type = CollisionType::None,bool fromFile = true,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float x, float y, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length,float threshhold = 0.0005f);

        Handle addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "",Handle shaderHandle = Handle());
        Handle addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);

        Handle addShader(std::string name, std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
		Handle addSoundData(std::string file,std::string name = "",bool music = false);
        Handle addShaderProgram(std::string name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
		Handle addShaderProgram(std::string name, Handle& vertexShader, Handle& fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
    };
	namespace Data{
		std::string reportTime();
	};
    //TODO: Move this somewhere else
    template<typename T>
    static std::string convertNumToNumWithCommas(T n){
        std::string r = std::to_string(n);
        int insertPosition = r.length() - 3;
        while (insertPosition > 0) {
            r.insert(insertPosition, ",");
            insertPosition-=3;
        }
        return r;
    }
};
#endif