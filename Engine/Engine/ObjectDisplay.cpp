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

ObjectDisplay::ObjectDisplay(std::string mesh, std::string mat, glm::v3 pos, glm::vec3 scl, std::string name,Scene* scene):ObjectBasic(pos,scl,name,scene){
    m_Radius = 0;
    m_Visible = true;
    m_Shadeless = false;
    m_BoundingBoxRadius = glm::vec3(0);
    if(mesh != "" && mat != "")
        m_DisplayItems.push_back(new DisplayItem(Resources::getMesh(mesh),Resources::getMaterial(mat)));
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
        shader = Resources::getShader("Deferred")->program();
    }
    Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void ObjectDisplay::draw(GLuint shader, bool debug,bool godsRays){
    Camera* camera = Resources::getActiveCamera();
    if((m_DisplayItems.size() == 0 || m_Visible == false) || (!camera->sphereIntersectTest(this)) || (camera->getDistance(this) > 1100 * getRadius()))
        return;	
    glUseProgram(shader);


    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
    glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
    glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);
	glUniform3f(glGetUniformLocation(shader, "Gods_Rays_Color"),m_GodsRaysColor.x,m_GodsRaysColor.y,m_GodsRaysColor.z);

	glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
	glUniform3f(glGetUniformLocation(shader,"CameraPosition"),camPos.x,camPos.y,camPos.z);

    if(m_Shadeless)
        glUniform1i(glGetUniformLocation(shader, "Shadeless"),1);
	if(godsRays)
		glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),1);
	else
		glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),0);

    for(auto item:m_DisplayItems){
        glm::mat4 m = glm::mat4(m_Model);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        if(!m_Shadeless)
            glUniform1i(glGetUniformLocation(shader, "Shadeless"),int(item->material->shadeless()));

        glUniform1f(glGetUniformLocation(shader, "BaseGlow"),item->material->glow());
		glUniform1f(glGetUniformLocation(shader, "matID"),float(float(item->material->id())/255.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m));

		item->material->bind(shader,Resources::getAPI());
        item->mesh->render();
    }
    glUseProgram(0);
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
    return c->rayIntersectSphere(this);
}
bool ObjectDisplay::rayIntersectSphere(glm::v3 A, glm::vec3 rayVector){
    glm::vec3 a1 = glm::vec3(A);
    glm::vec3 B = a1 + rayVector;

    glm::vec3 C = glm::vec3(getPosition());
    float r = getRadius();
    if(r <= 0)
        return false;

    //check if point is behind
    float dot = glm::dot(rayVector,C-a1);
    if(dot >= 0)
        return false;

    glm::num a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
    glm::num b = 2* ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
    glm::num c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);

    glm::num Delta = (b*b) - (4*a*c);

    if(Delta < 0)
        return false;
    return true;
}