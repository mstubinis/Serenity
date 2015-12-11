#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"

using namespace Engine;

ObjectDisplay::ObjectDisplay(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name, bool notLight,Scene* scene):Object(pos,scl,name,notLight,scene){
	m_Radius = 0;
	m_Visible = true;
	m_BoundingBoxRadius = glm::vec3(0);
	setMesh(Resources::getMesh(mesh));
	setMaterial(Resources::getMaterial(mat));
	m_Color = glm::vec4(1);
}
ObjectDisplay::~ObjectDisplay()
{
}
void ObjectDisplay::render(Mesh* mesh,Material* material,GLuint shader,bool debug){
	//add to render queue
	if(shader == 0){
		shader = Resources::getShader("Deferred")->getShaderProgram();
	}
	Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,mesh,material,shader));
}
void ObjectDisplay::draw(Mesh* mesh, Material* material, GLuint shader, bool debug){
	Camera* camera = Resources::getActiveCamera();
	if((mesh == nullptr || m_Visible == false) || (!camera->sphereIntersectTest(this)) || (camera->getDistance(this) > 1100 * getRadius()))
		return;	
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
	glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);

	glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(material->getShadeless()));
	glUniform1f(glGetUniformLocation(shader, "BaseGlow"),material->getBaseGlow());

	for(auto component:material->getComponents())
		material->bindTexture(component.first,shader);
	mesh->render();
	glUseProgram(0);
}
void ObjectDisplay::calculateRadius(){
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0);
		return;
	}
	m_BoundingBoxRadius = m_Mesh->getRadius() * m_Scale;
	m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
void ObjectDisplay::render(GLuint shader,bool debug){ render(m_Mesh,m_Material,shader,debug); }
void ObjectDisplay::setColor(float x, float y, float z,float a){ 
	m_Color.x = x; m_Color.y = y; m_Color.z = z; m_Color.w = a; 
}
void ObjectDisplay::setColor(glm::vec4 color){ setColor(color.x,color.y,color.z,color.w); }
void ObjectDisplay::setVisible(bool vis){ m_Visible = vis; }
void ObjectDisplay::setMesh(Mesh* mesh){ 
	m_Mesh = mesh; 
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0); 
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
bool ObjectDisplay::rayIntersectSphere(Camera* cam){
	return cam->rayIntersectSphere(this);
}
bool ObjectDisplay::rayIntersectSphere(glm::vec3 A, glm::vec3 rayVector){
	glm::vec3 B = A + rayVector;

	glm::vec3 C = getPosition();
	float r = getRadius();

	//check if point is behind
	float dot = glm::dot(rayVector,C-A);
	if(dot >= 0)
		return false;

	float a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
	float b = 2* ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
	float c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);

	float Delta = (b*b) - (4*a*c);

	if(Delta < 0)
		return false;
	return true;
}