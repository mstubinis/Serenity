#ifndef SCENE_H
#define SCENE_H

#include <unordered_map>
#include <string>

class SunLight;
class Object;
class Camera;

class Scene{
	protected:
		std::unordered_map<std::string,Camera*> m_Cameras;
		std::unordered_map<std::string,Object*> m_Objects;
		std::unordered_map<std::string,SunLight*> m_Lights;
	public:
		Scene(std::string name);
		~Scene();

		std::unordered_map<std::string,Object*>& Objects(){ return m_Objects; }
		std::unordered_map<std::string,SunLight*>& Lights(){ return m_Lights; }

		virtual void Update(float);
};
#endif