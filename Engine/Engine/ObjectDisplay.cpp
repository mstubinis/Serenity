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
        m_DisplayItems.push_back(new DisplayItem(Resources::getMesh(mesh),Resources::getMaterial(mat)));
		Resources::getMaterial(mat)->addObject(name());
	}
    m_Color = glm::vec4(1);
	m_GodsRaysColor = glm::vec3(0);
    calculateRadius();
}
ObjectDisplay::~ObjectDisplay(){
    for(auto item:m_DisplayItems) SAFE_DELETE(item);
}
void ObjectDisplay::render(GLuint shader,bool debug){
    //add to render queue
    if(shader == 0){
        shader = Resources::getShaderProgram("Deferred")->program();
    }
    Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void ObjectDisplay::setMaterial(std::string materialName, uint index){
	Material* current = this->m_DisplayItems[index]->material;
	Material* newMaterial = Resources::getMaterial(materialName);

	current->removeObject(name());
	newMaterial->addObject(name());
}
void ObjectDisplay::draw(GLuint shader, bool debug,bool godsRays){
	Engine::Renderer::Detail::drawObject(this,shader,debug,godsRays);
}
void ObjectDisplay::calculateRadius(){
    if(m_DisplayItems.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0);
        return;
    }
    float maxLength = 0;
    for(auto item:m_DisplayItems){
        float length = 0;
        glm::mat4 m = glm::mat4(1);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
        
        length = glm::length(localPosition) + item->mesh->getRadius() * glm::max(glm::abs(item->scale.z), glm::max(glm::abs(item->scale.x),glm::abs(item->scale.y)));

        if(length > maxLength){
            maxLength = length;
        }
    }
    m_BoundingBoxRadius = maxLength * m_Scale;
    m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
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