#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"

using namespace Engine;

Scene::Scene(std::string name){
	if(Resources::getCurrentScene() == nullptr)
		Resources::Detail::ResourceManagement::m_CurrentScene = this;
	Resources::Detail::ResourceManagement::m_Scenes[name] = this;
}
Scene::~Scene(){
	for(auto obj:m_Objects)
		delete obj.second;
	for(auto light:m_Lights)
		delete light.second;
	for(auto cam:m_Cameras)
		delete cam.second;
}
void Scene::Update(float dt)
{
}