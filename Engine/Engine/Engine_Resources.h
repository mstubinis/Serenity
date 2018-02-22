#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

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

class Engine_Window;
class Scene;
class Font;
class Texture;
class Camera;
class Mesh;
class Material;
class Object;
class SunLight;
class MeshInstance;

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for deleting a pointer

template <typename E> void vector_clear(std::vector<E>& t){ t.clear(); std::vector<E>().swap(t); t.shrink_to_fit(); }
template <typename E> std::string to_string(E t){ return boost::lexical_cast<std::string>(t); }

namespace Engine{
	namespace impl{
		class ResourceManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ResourceManager(const char* name,uint width,uint height);
				~ResourceManager();

				bool _hasMaterial(std::string);      void _addMaterial(Material*);          std::string _buildMaterialName(std::string);
				bool _hasMesh(std::string);          void _addMesh(Mesh*);                  std::string _buildMeshName(std::string);
				bool _hasTexture(std::string);       void _addTexture(Texture*);            std::string _buildTextureName(std::string);
				bool _hasObject(std::string);        void _addObject(Object*);              std::string _buildObjectName(std::string);
				bool _hasFont(std::string);          void _addFont(Font*);                  std::string _buildFontName(std::string);
				bool _hasScene(std::string);         void _addScene(Scene*);                std::string _buildSceneName(std::string);
				bool _hasMeshInstance(std::string);  void _addMeshInstance(MeshInstance*);  std::string _buildMeshInstanceName(std::string);
				bool _hasCamera(std::string);        void _addCamera(Camera*);              std::string _buildCameraName(std::string);
				bool _hasShader(std::string);        void _addShader(Shader*);              std::string _buildShaderName(std::string);

				void _remCamera(std::string);
				void _remObject(std::string);


				void _resizeCameras(uint w,uint h);
				uint _numScenes();
		};
	};
	namespace Data{
		std::string reportTime();
		std::string reportTimeRendering();
	};






    namespace Resources{
        namespace Settings{
            void enableDynamicMemory(bool b = true);
            void disableDynamicMemory();
        }

        Scene* getCurrentScene();
        void setCurrentScene(Scene* s);
        void setCurrentScene(std::string s);

        float dt();
        float applicationTime();

        Engine_Window* getWindow();
        glm::uvec2 getWindowSize();

        Camera* getActiveCamera();
        boost::weak_ptr<Camera>& getActiveCameraPtr();
        void setActiveCamera(Camera* c);
        void setActiveCamera(std::string name);

        Scene* getScene(std::string n);

        boost::shared_ptr<Object>& getObjectPtr(std::string);
        boost::shared_ptr<Camera>& getCameraPtr(std::string);
        boost::shared_ptr<Texture>& getTexturePtr(std::string);

        Object* getObject(std::string n);
        Camera* getCamera(std::string n);
        Font* getFont(std::string n);
        Texture* getTexture(std::string n);
        Mesh* getMesh(std::string n);
        Material* getMaterial(std::string n);
        Shader* getShader(std::string n);
        ShaderP* getShaderProgram(std::string n);
        MeshInstance* getMeshInstance(std::string n);

        void addMesh(std::string name,std::string file, CollisionType = CollisionType::None,bool fromFile = true,float threshhold = 0.0005f);
        void addMesh(std::string file, CollisionType = CollisionType::None,float threshhold = 0.0005f);
        void addMesh(std::string name, float x, float y, float w, float h,float threshhold = 0.0005f);
        void addMesh(std::string name, float w, float h,float threshhold = 0.0005f);
        void addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length,float threshhold = 0.0005f);
        void removeMesh(std::string name);

        void addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "",std::string shader = "");
        void addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
        void removeMaterial(std::string name);

        void addShader(std::string name, std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);

		void addSound(std::string file,std::string name = "");

        void addShaderProgram(std::string name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        void addShaderProgram(std::string name, std::string vertexShader, std::string fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        void addShaderProgram(std::string name, Shader* vertexShader, std::string fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        void addShaderProgram(std::string name, std::string vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);

        void initResources(const char* name,uint width,uint height);
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