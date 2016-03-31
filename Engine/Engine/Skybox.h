#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

typedef unsigned int GLuint;

class Scene;
class Texture;
class Mesh;

struct SkyboxTextures final{
	std::string front;
	std::string back;
	std::string left;
	std::string right;
	std::string top;
	std::string bottom;
};
class Skybox{
	private:
		Texture* m_Texture;
		glm::mat4 m_Model;

		static GLuint m_Buffer;
		static std::vector<glm::vec3> m_Vertices;
	public:
		Skybox(std::string name,Scene* = nullptr);
		virtual ~Skybox();

		virtual void update();
		virtual void render();
};
#endif