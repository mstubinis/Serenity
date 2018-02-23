#include "Engine.h"
#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "Skybox.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;
using namespace std;

Scene::Scene(string name){
    m_Skybox = nullptr;
	m_ActiveCamera = nullptr;
    m_BackgroundColor = glm::vec3(0.0f);
	name = epriv::Core::m_Engine->m_ResourceManager->_buildSceneName(name);
    setName(name);

	epriv::Core::m_Engine->m_ResourceManager->_addScene(this);

    if(Resources::getCurrentScene() == nullptr){
		Resources::setCurrentScene(this);
    }
}
Camera* Scene::getActiveCamera(){ return m_ActiveCamera; }
void Scene::setActiveCamera(Camera* c){
	m_ActiveCamera = c;
}
void Scene::setActiveCamera(string s){
	m_ActiveCamera = Resources::getCamera(s);
}
void Scene::centerSceneToObject(Object* center){
    glm::vec3 offset = -(center->getPosition());
    for(auto object:m_Objects){
        Object* obj = object.second;
        if(obj != center && obj->getParent() == nullptr){
            obj->setPosition(obj->getPosition() + offset);
        }
    }
    if(center->getParent() == nullptr)
        center->setPosition(0.0f,0.0f,0.0f);
}
Scene::~Scene(){
    SAFE_DELETE(m_Skybox);
}
void Scene::update(float dt){
    for (auto it = m_Objects.cbegin(); it != m_Objects.cend();){
        if (it->second->isDestroyed()){
            epriv::Core::m_Engine->m_ResourceManager->_remObject(it->second->name());
            m_Objects.erase(it++);
        }
        else{
            it->second->update(dt); ++it;
        }
    }
    for (auto it = m_Cameras.cbegin(); it != m_Cameras.cend();){
        if (it->second->isDestroyed()){
			epriv::Core::m_Engine->m_ResourceManager->_remCamera(it->second->name());
        }
        else{
			it->second->update(dt); ++it;
        }
    }
    if(m_Skybox != nullptr) m_Skybox->update();
}
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_BackgroundColor,r,g,b); }
void Scene::renderSkybox(){ 
	if(m_Skybox != nullptr) 
		m_Skybox->draw(); 
	else{
		//render a fake skybox.
		Skybox::initMesh();
		ShaderP* p = Resources::getShaderProgram("Deferred_Skybox_Fake"); p->bind();
		Camera* c = Resources::getCurrentScene()->getActiveCamera();
		glm::mat4 view = c->getView();
		Math::removeMatrixPosition(view);
		Renderer::sendUniformMatrix4f("VP",c->getProjection() * view);
		Renderer::sendUniform4f("Color",m_BackgroundColor.r,m_BackgroundColor.g,m_BackgroundColor.b,1.0f);
		Skybox::bindMesh();
		Renderer::unbindTextureCubemap(0);//yes, this is needed.
		p->unbind();
	}
}
glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }
unordered_map<string,Object*>& Scene::objects() { return m_Objects; }
unordered_map<string,SunLight*>& Scene::lights() { return m_Lights; }
unordered_map<string,Camera*>& Scene::cameras() { return m_Cameras; }
unordered_map<string,LightProbe*>& Scene::lightProbes(){ return m_LightProbes; }
Object* Scene::getObject(string& name){ return m_Objects.at(name); }
SunLight* Scene::getLight(string& name){ return m_Lights.at(name); }
Camera* Scene::getCamera(string& name){ return m_Cameras.at(name); }
SkyboxEmpty* Scene::getSkybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
