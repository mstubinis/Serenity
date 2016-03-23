#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Skybox.h"
#include "Particles.h"
#include <boost/shared_ptr.hpp>

using namespace Engine;
using namespace boost;

Scene::Scene(std::string name,glm::vec3 ambientLightColor){
	if(Resources::getCurrentScene() == nullptr){
		Resources::Detail::ResourceManagement::m_CurrentScene = this;
	}
	m_Name = name;
	m_AmbientLighting = ambientLightColor;
	m_BackgroundColor = glm::vec3(0,0,0);

	if(!exists(Resources::getActiveCameraPtr())){
		new Camera("Default_" + m_Name,45.0f,1.0f,0.1f,100.0f,this);
		Resources::setActiveCamera("Default_" + m_Name);
	}

	Resources::Detail::ResourceManagement::m_Scenes[m_Name] = shared_ptr<Scene>(this);
}
void Scene::centerSceneToObject(Object* center){
	glm::v3 offset = -(center->getPosition());
	Scene* s =  Resources::getCurrentScene();
	for(auto object:s->getObjects()){
		Object* obj = object.second;
		if(obj != center && obj->getParent() == nullptr){
			obj->setPosition(obj->getPosition() + offset);
		}
	}
	for(auto emitter:m_ParticleEmitters){
		glm::v3 objPos = emitter.second->getPosition();
		emitter.second->setPosition(objPos + offset);
		for(auto particle:emitter.second->getParticles()){
			objPos = particle->getPosition();
			particle->setPosition(objPos + offset);
		}
	}
	for(auto object:s->getLights()){
		Object* obj = object.second;
		if(obj != center && obj->getParent() == nullptr){
			obj->setPosition(obj->getPosition() + offset);
		}
	}
	if(center->getParent() == nullptr)
		center->setPosition(0,0,0);
}
Scene::~Scene(){
}
void Scene::setBackgroundColor(float r, float g, float b){ m_BackgroundColor.x = r; m_BackgroundColor.y = g; m_BackgroundColor.z = b; }
void Scene::setName(std::string name){
	if(name == m_Name) return;

	std::string oldName = m_Name; m_Name = name;
	Resources::Detail::ResourceManagement::m_Scenes[name] = boost::shared_ptr<Scene>(this);

	if(Resources::Detail::ResourceManagement::m_Scenes.count(oldName)){
		Resources::Detail::ResourceManagement::m_Scenes[oldName].reset();
		Resources::Detail::ResourceManagement::m_Scenes.erase(oldName);
	}
}
void Scene::update(float dt){
	for (auto it = m_Objects.cbegin(); it != m_Objects.cend();){
		if (it->second->isDestroyed()){
			std::string name = it->second->getName();
			Resources::getObjectPtr(name).reset();
			Resources::Detail::ResourceManagement::m_Objects.erase(name);
			m_Objects.erase(it++);
	    }
	    else{
			it->second->update(dt);
			++it;
	    }
	}
	for (auto it = Resources::Detail::ResourceManagement::m_Cameras.cbegin(); it != Resources::Detail::ResourceManagement::m_Cameras.cend();){
		if (it->second->isDestroyed()){
			std::string name = it->second->getName();
			Resources::getCameraPtr(name).reset();
			Resources::Detail::ResourceManagement::m_Cameras.erase(it++);
	    }
	    else{
			if(it->second->getScene() == this){
				it->second->update(dt);
			}
			++it;
	    }
	}

	for(auto emitter:m_ParticleEmitters){
		for(auto particle:emitter.second->getParticles()){
			particle->update(dt);
		}
	}
	if(m_Skybox != nullptr) m_Skybox->update();
}
void Scene::setAmbientLightColor(glm::vec3 c){ m_AmbientLighting = c; }
void Scene::setAmbientLightColor(float r,float g,float b){ setAmbientLightColor(glm::vec3(r,g,b)); }
void Scene::renderSkybox(){ if(m_Skybox != nullptr) m_Skybox->render(); }