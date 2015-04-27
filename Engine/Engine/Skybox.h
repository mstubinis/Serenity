#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <glm/glm.hpp>

class Scene;
class Texture;
class Mesh;

struct SkyboxTextures{
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
		Mesh* m_Mesh;
	public:
		Skybox(std::string name,Scene* = nullptr);
		~Skybox();

		void update();
		void render();
};
#endif