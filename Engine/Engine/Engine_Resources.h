#ifndef ENGINE_RESOURCES_H
#define ENGINE_RESOURCES_H

#include <unordered_map>
#include <GL\glew.h>
#include <SFML\OpenGL.hpp>
#include <SFML\Window.hpp>
#include <Bullet\btBulletDynamicsCommon.h>

class Camera;
class Mesh;
class Material;
class ShaderP;
class Object;
class SunLight;

class ResourceManager{
	private:
		Camera* m_Current_Camera;

		std::unordered_map<std::string,Camera*> m_Cameras;
		std::unordered_map<std::string,Mesh*> m_Meshes;
		std::unordered_map<std::string,Material*> m_Materials;
		std::unordered_map<std::string,ShaderP*> m_ShaderPrograms;

	public:
		ResourceManager();
		~ResourceManager();

		void Set_Active_Camera(Camera*); void Set_Active_Camera(std::string);
		void Load_Texture_Into_GLuint(GLuint&, std::string filename);
		void Load_Cubemap_Texture_Into_GLuint(GLuint&, std::string filenames[]);

		void Add_Mesh(std::string file);
		void Add_Mesh(std::string name,std::string file);
		void Add_Material(std::string name,std::string diffuse, std::string normal, std::string glow);
		void Add_Shader_Program(std::string name,std::string vs, std::string ps);
		void Add_Camera(std::string,Camera*);

		float dt;
		std::vector<Object*> Objects;
		std::vector<SunLight*> Lights;
		Material* Default_Material(); Material* Get_Material(std::string);
		Mesh* Default_Mesh(); Mesh* Get_Mesh(std::string);
		Camera* Default_Camera(); Camera* Get_Camera(std::string); Camera* Current_Camera();
		ShaderP* Default_Shader_Program(); ShaderP* Get_Shader_Program(std::string);

		void INIT_Game_Resources();
};
extern sf::Window* Window;
extern sf::Mouse* Mouse;
extern ResourceManager* Resources;
#endif