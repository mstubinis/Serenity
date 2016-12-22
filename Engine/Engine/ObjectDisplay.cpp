#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;

ObjectDisplay::ObjectDisplay(std::string mesh, std::string mat, glm::v3 pos, glm::vec3 scl, std::string n,Scene* scene):ObjectBasic(pos,scl,n,scene){
    m_Radius = 0;
    m_Visible = true;
    m_Shadeless = false;
    m_BoundingBoxRadius = glm::vec3(0);
    if(mesh != "" && mat != ""){
		RenderedItem* item = new RenderedItem(namePtr(),mesh,mat);
        m_DisplayItems.push_back(item);
	}
    m_Color = glm::vec4(1);
	m_GodsRaysColor = glm::vec3(0);
    calculateRadius();
}
ObjectDisplay::~ObjectDisplay(){
    //for(auto item:m_DisplayItems) SAFE_DELETE(item);
}
void ObjectDisplay::bind(){
	Renderer::sendUniform4f("Object_Color",m_Color.x,m_Color.y,m_Color.z,m_Color.w);
	Renderer::sendUniform3f("Gods_Rays_Color",m_GodsRaysColor.x,m_GodsRaysColor.y,m_GodsRaysColor.z);
}
void ObjectDisplay::update(float dt){
	ObjectBasic::update(dt);
	for(auto renderedItem:m_DisplayItems){
		renderedItem->update(dt);
	}
	Camera* c = Resources::getActiveCamera();
	m_PassedRenderCheck = true;
	if(!m_Visible || !c->sphereIntersectTest(this) || c->getDistance(this) > m_Radius * 1100.0f){
		m_PassedRenderCheck = false;
	}
}
void ObjectDisplay::render(GLuint shader,bool debug){
    //add to render queue
    if(shader == 0){
        shader = Resources::getShaderProgram("Deferred")->program();
    }
    Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void ObjectDisplay::draw(GLuint shader, bool debug,bool godsRays){

}
void ObjectDisplay::calculateRadius(){
    if(m_DisplayItems.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0);
		m_Radius = 0;
        return;
    }
    float maxLength = 0;
    for(auto item:m_DisplayItems){
        float length = 0;
		glm::mat4 m = item->model();
        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
		length = glm::length(localPosition) + item->mesh()->getRadius() * Engine::Math::Max(item->getScale());
        if(length > maxLength){
            maxLength = length;
        }
    }
    m_BoundingBoxRadius = maxLength * m_Scale;
	m_Radius = Engine::Math::Max(m_BoundingBoxRadius);
}
void ObjectDisplay::setColor(float r, float g, float b,float a){ Engine::Math::setColor(m_Color,r,g,b,a); }
void ObjectDisplay::setColor(glm::vec4 c){ setColor(c.x,c.y,c.z,c.w); }
void ObjectDisplay::setGodsRaysColor(float r, float g, float b){ Engine::Math::setColor(m_GodsRaysColor,r,g,b); }
void ObjectDisplay::setGodsRaysColor(glm::vec3 c){ setGodsRaysColor(c.x,c.y,c.z); }
void ObjectDisplay::setVisible(bool v){ m_Visible = v; }

void ObjectDisplay::setScale(float x,float y,float z){
    ObjectBasic::setScale(x,y,z);
    calculateRadius();
}
void ObjectDisplay::setScale(glm::vec3 s){ ObjectDisplay::setScale(s.x,s.y,s.z); }

void ObjectDisplay::scale(float x, float y,float z){
    ObjectBasic::scale(x,y,z);
    calculateRadius(); 
}
void ObjectDisplay::scale(glm::vec3 s){ ObjectDisplay::scale(s.x,s.y,s.z); }
bool ObjectDisplay::rayIntersectSphere(Camera* c){
	if(c == nullptr) c = Resources::getActiveCamera();
    return c->rayIntersectSphere(this);
}
bool ObjectDisplay::rayIntersectSphere(glm::v3 A, glm::vec3 rayVector){
	return Engine::Math::rayIntersectSphere(glm::vec3(getPosition()),getRadius(),A,rayVector);
}