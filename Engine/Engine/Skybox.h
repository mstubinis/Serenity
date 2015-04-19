#ifndef SKYBOX_H
#define SKYBOX_H

#include "Object.h"

class Scene;

struct SkyboxTextures{
	std::string front;
	std::string back;
	std::string left;
	std::string right;
	std::string top;
	std::string bottom;
};
class Skybox: public Object{
	private:
		GLuint m_Shader;
		GLuint m_Texture;
	public:
		Skybox(std::string name,Scene* = nullptr);
		~Skybox();

		void update(float);
		void render(bool=false);
};
#endif