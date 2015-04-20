#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"

using namespace Engine;

Scene::Scene(std::string name){
	if(Resources::getCurrentScene() == nullptr)
		Resources::Detail::ResourceManagement::m_CurrentScene = this;

	Resources::Detail::ResourceManagement::m_Scenes[name] = this;
	m_Name = name;
}
Scene::~Scene(){
	for(auto obj:m_Objects)
		delete obj.second;
	for(auto light:m_Lights)
		delete light.second;
	for(auto cam:m_Cameras)
		delete cam.second;
}
void Scene::setName(std::string name){
	std::string oldName = m_Name;
	m_Name = name;
	Resources::Detail::ResourceManagement::m_Scenes[name] = this;

	for(auto it = begin(Resources::Detail::ResourceManagement::m_Scenes); it != end(Resources::Detail::ResourceManagement::m_Scenes);){
		if (it->first == oldName)it = Resources::Detail::ResourceManagement::m_Scenes.erase(it);
	    else++it;
	}
}
void Scene::update(float dt)
{
}