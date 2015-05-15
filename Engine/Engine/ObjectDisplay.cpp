#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"

using namespace Engine;

ObjectDisplay::ObjectDisplay(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name, bool notLight,Scene* scene):Object(pos,scl,name,notLight,scene){
	m_Radius = 0;
	m_BoundingBoxRadius = glm::vec3(0,0,0);
	setMesh(Resources::getMesh(mesh));
	setMaterial(Resources::getMaterial(mat));
	m_Color = glm::vec4(1,1,1,1);
}
ObjectDisplay::~ObjectDisplay()
{
}
void ObjectDisplay::render(Mesh* mesh,Material* material,bool debug)
{
	if(mesh == nullptr)
		return;
	Camera* activeCamera = Resources::getActiveCamera();
	if(!activeCamera->sphereIntersectTest(this))
		return;
	if(activeCamera->getDistance(this) > 850 * getRadius())
		return;

	GLuint shader = Resources::getShader("Deferred")->getShaderProgram();

	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
	glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(material->getShadeless()));

	for(auto component:material->getComponents())
		material->bindTexture(component.first,shader);
	mesh->render();
}
void ObjectDisplay::calculateRadius(){
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0);
		return;
	}
	m_BoundingBoxRadius = m_Mesh->getRadius() * m_Scale;
	m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
void ObjectDisplay::render(bool debug){ render(m_Mesh,m_Material,debug); }
void ObjectDisplay::setColor(float x, float y, float z,float a){ 
	m_Color.x = x; m_Color.y = y; m_Color.z = z; m_Color.w = a; 
}
void ObjectDisplay::setColor(glm::vec4 color){ setColor(color.x,color.y,color.z,color.w); }
void ObjectDisplay::setMesh(Mesh* mesh){ 
	m_Mesh = mesh; 
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0); 
		return; 
	} 
	m_BoundingBoxRadius = mesh->getRadiusBox();  
	calculateRadius();
}
void ObjectDisplay::setMaterial(Material* material){ m_Material = material; }
void ObjectDisplay::scale(float x, float y,float z){
	Object::scale(x,y,z);
	calculateRadius(); 
}
void ObjectDisplay::scale(glm::vec3 scl){ ObjectDisplay::scale(scl.x,scl.y,scl.z); }
void ObjectDisplay::_updateMatrix(){
	Object::_updateMatrix();
}