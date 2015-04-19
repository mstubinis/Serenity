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

		std::string m_Name;

	public:
		Scene(std::string name);
		~Scene();

		std::unordered_map<std::string,Object*>& getObjects(){ return m_Objects; }
		std::unordered_map<std::string,SunLight*>& getLights(){ return m_Lights; }

		const std::string getName() const { return m_Name; }

		virtual void setName(std::string);

		virtual void update(float);
};
#endif