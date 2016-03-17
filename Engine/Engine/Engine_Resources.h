#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include <unordered_map>

#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <boost/shared_ptr.hpp>

class Font;
class Texture;
class Scene;
class Camera;
class Mesh;
class Material;
class ShaderP;
class Object;
class SunLight;
class SoundEffect;

struct ParticleInfo;

template<typename T> void SAFE_DELETE(T*& p){
    delete p;
    p = nullptr;
}
                              
namespace Engine{
	namespace Resources{
		namespace Detail{
			class ResourceManagement final{
				public:
					static std::unordered_map<std::string,Scene*> m_Scenes;
					static Scene* m_CurrentScene;

					static float m_DeltaTime;

					static sf::Window* m_Window;
					static std::string m_WindowName;
					static sf::Mouse* m_Mouse;

					static Camera* m_ActiveCamera;

					static std::unordered_map<std::string,boost::shared_ptr<SoundEffect>> m_Sounds;
					static std::unordered_map<std::string,Object*> m_Objects;
					static std::unordered_map<std::string,Camera*> m_Cameras;
					static std::unordered_map<std::string,boost::shared_ptr<Font>> m_Fonts;
					static std::unordered_map<std::string,boost::shared_ptr<Mesh>> m_Meshes;
					static std::unordered_map<std::string,boost::shared_ptr<Texture>> m_Textures;
					static std::unordered_map<std::string,boost::shared_ptr<Material>> m_Materials;
					static std::unordered_map<std::string,boost::shared_ptr<ShaderP>> m_Shaders;
					static std::unordered_map<std::string,boost::shared_ptr<ParticleInfo>> m_ParticleInfos;

					static void destruct();
			};
		};
		static Scene* getCurrentScene(){ return Detail::ResourceManagement::m_CurrentScene; }
		static void setCurrentScene(Scene* s){ Detail::ResourceManagement::m_CurrentScene; }
		static void setCurrentScene(std::string s){ Detail::ResourceManagement::m_CurrentScene = Detail::ResourceManagement::m_Scenes[s]; }

		static float getDeltaTime(){ return Detail::ResourceManagement::m_DeltaTime; }
		static float dt(){ return Detail::ResourceManagement::m_DeltaTime; }

		static sf::Window* getWindow(){ return Detail::ResourceManagement::m_Window; }
		static sf::Vector2u getWindowSize(){ return Detail::ResourceManagement::m_Window->getSize(); }
		static sf::Mouse* getMouse(){ return Detail::ResourceManagement::m_Mouse; }

		static Camera* getActiveCamera(){ return Detail::ResourceManagement::m_ActiveCamera; }
		static void setActiveCamera(Camera* c){ Detail::ResourceManagement::m_ActiveCamera = c; }
		static void setActiveCamera(std::string name){ Detail::ResourceManagement::m_ActiveCamera = Detail::ResourceManagement::m_Cameras[name]; }

		static SoundEffect* getSound(std::string name){ return Detail::ResourceManagement::m_Sounds[name].get(); }
		static Object* getObject(std::string name){ return Detail::ResourceManagement::m_Objects[name]; }
		static Camera* getCamera(std::string name){ return Detail::ResourceManagement::m_Cameras[name]; }
		static Font* getFont(std::string name){ return Detail::ResourceManagement::m_Fonts[name].get(); }
		static Texture* getTexture(std::string name){ return Detail::ResourceManagement::m_Textures[name].get(); }
		static Mesh* getMesh(std::string name){ return Detail::ResourceManagement::m_Meshes[name].get(); }
		static Material* getMaterial(std::string name){ return Detail::ResourceManagement::m_Materials[name].get(); }
		static ParticleInfo* getParticleInfo(std::string name){ return Detail::ResourceManagement::m_ParticleInfos[name].get(); }
		static ShaderP* getShader(std::string name){ return Detail::ResourceManagement::m_Shaders[name].get(); }

		void addMesh(std::string name,std::string file);
		void addMesh(std::string file);
		void removeMesh(std::string name);

		void addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "");
		void addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr);
		void removeMaterial(std::string name);

		void addShader(std::string name, std::string vertexShaderFile, std::string fragmentShaderFile);

		void addParticleInfo(std::string name, std::string material);
		void addParticleInfo(std::string name, Material* diffuse);
		void removeParticleInfo(std::string name);

		void addSound(std::string name, std::string file);
		void removeSound(std::string name);

		void initResources();
	};
	//TODO: Move this somewhere else
	template<typename T>
	static std::string convertNumToNumWithCommas(T n){
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