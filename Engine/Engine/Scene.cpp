#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Skybox.h"
#include "Particles.h"


using namespace Engine;

Scene::Scene(std::string name,glm::vec4 ambientLightColor){
	if(Resources::getCurrentScene() == nullptr)
		Resources::Detail::ResourceManagement::m_CurrentScene = this;

	Resources::Detail::ResourceManagement::m_Scenes[name] = this;
	m_Name = name;
	m_AmbientLighting = ambientLightColor;
}
void Scene::centerSceneToObject(Object* center){
	glm::vec3 offset = -(center->getPosition());
	Scene* s =  Resources::getCurrentScene();
	for(auto obj:s->getObjects()){
		if(obj.second != center && obj.second->getParent() == nullptr){
			glm::vec3 objPos = obj.second->getPosition();
			obj.second->setPosition(objPos + offset);
		}
	}
	for(auto emitter:m_ParticleEmitters){
		for(auto particle:emitter.second->getParticles()){
			glm::vec3 objPos = particle.getPosition();
			particle.setPosition(objPos + offset);
		}
	}
	for(auto obj:s->getLights()){
		if(obj.second != center && obj.second->getParent() == nullptr){
			glm::vec3 objPos = obj.second->getPosition();
			obj.second->setPosition(objPos + offset);
		}
	}
	center->setPosition(0,0,0);
}
Scene::~Scene(){
	for(auto obj:m_Objects)              delete obj.second;
	for(auto light:m_Lights)             delete light.second;
	for(auto emitter:m_ParticleEmitters) delete emitter.second;
	for(auto cam:m_Cameras)              delete cam.second;
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
void Scene::update(float dt){
	for(auto object:m_Objects)
		object.second->update(dt);
	for(auto emitter:m_ParticleEmitters){
		for(auto particle:emitter.second->getParticles()){
			particle.update(dt);
		}
	}
	for(auto light:m_Lights)
		light.second->update(dt);
	m_Skybox->_updateMatrix();
}
void Scene::setAmbientLightColor(glm::vec4 c){ m_AmbientLighting = c; }
void Scene::setAmbientLightColor(float r,float g,float b,float a){ setAmbientLightColor(glm::vec4(r,g,b,a)); }
void Scene::renderSkybox(){ if(m_Skybox != nullptr) m_Skybox->render(); }