#ifndef ENGINE_RESOURCES_H
#define ENGINE_RESOURCES_H

#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <bullet/btBulletDynamicsCommon.h>

class Font;
class Camera;
class Mesh;
class Material;
class ShaderP;
class Object;
class SunLight;
class Scene;
class Texture;

namespace Engine{
	namespace Resources{
		namespace Detail{
			class ResourceManagement{
				public:
					static std::unordered_map<std::string,Scene*> m_Scenes;
					static Scene* m_CurrentScene;

					static float m_DeltaTime;

					static sf::Window* m_Window;
					static sf::Mouse* m_Mouse;

					static Camera* m_ActiveCamera;

					static std::unordered_map<std::string,Font*> m_Fonts;
					static std::unordered_map<std::string,Camera*> m_Cameras;
					static std::unordered_map<std::string,Mesh*> m_Meshes;
					static std::unordered_map<std::string,Texture*> m_Textures;
					static std::unordered_map<std::string,Material*> m_Materials;
					static std::unordered_map<std::string,ShaderP*> m_Shaders;

					static void destruct();
			};
		};
		static Scene* getCurrentScene(){ return Detail::ResourceManagement::m_CurrentScene; }
		static void setCurrentScene(Scene* s){ Detail::ResourceManagement::m_CurrentScene = s; }

		static float getDeltaTime(){ return Detail::ResourceManagement::m_DeltaTime; }
		static float dt(){ return Detail::ResourceManagement::m_DeltaTime; }

		static sf::Window* getWindow(){ return Detail::ResourceManagement::m_Window; }
		static sf::Mouse* getMouse(){ return Detail::ResourceManagement::m_Mouse; }

		static Camera* getActiveCamera(){ return Detail::ResourceManagement::m_ActiveCamera; }
		static Camera* getCamera(std::string name){ return Detail::ResourceManagement::m_Cameras[name]; }
		static void setActiveCamera(Camera* c){ Detail::ResourceManagement::m_ActiveCamera = c; }
		static void setActiveCamera(std::string name){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras[name]; }

		static Font* getFont(std::string name){ return Detail::ResourceManagement::m_Fonts[name]; }
		static Texture* getTexture(std::string name){ return Detail::ResourceManagement::m_Textures[name]; }
		static Mesh* getMesh(std::string name){ return Detail::ResourceManagement::m_Meshes[name]; }
		static Material* getMaterial(std::string name){ return Detail::ResourceManagement::m_Materials[name]; }
		static ShaderP* getShader(std::string name){ return Detail::ResourceManagement::m_Shaders[name]; }


		void addMesh(std::string name,std::string file);
		void addMesh(std::string file);
		void addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "");
		void addShader(std::string name, std::string vertexShaderFile, std::string fragmentShaderFile);

		void initResources();
	};
	static std::string convertNumberToStringCommas(unsigned long long n){
		std::string numWithCommas = std::to_string(n);
		int insertPosition = numWithCommas.length() - 3;
		while (insertPosition > 0) {
			numWithCommas.insert(insertPosition, ",");
			insertPosition-=3;
		}
		return numWithCommas;
	}
};
#endif