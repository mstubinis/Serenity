#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Scene;
class Texture;
class Mesh;

struct SkyboxSunFlare{
	glm::vec3 position;
	glm::vec3 color;
	float scale;
};
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

		std::vector<SkyboxSunFlare> m_SunFlares;

		glm::vec2 getScreenCoordinates(glm::vec3);

	public:
		Skybox(std::string name,unsigned int numSunFlares = 0,Scene* = nullptr);
		~Skybox();

		void _updateMatrix();
		void render();
};
#endif