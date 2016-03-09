#include "Lagrange.h"
#include "Planet.h"
#include "Camera.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Engine_Resources.h"
#include "Engine.h"
#include <algorithm>
#include <cctype>

using namespace Engine;

GLuint Lagrange::m_Buffer;

std::vector<glm::vec3> _genBuffer(){
	std::vector<glm::vec3> temp;
	    //68 verts
		temp.push_back(glm::vec3(0,-10.608f,-13.657f));
		temp.push_back(glm::vec3(0,-6.262f,-8.53f));
		temp.push_back(glm::vec3(4.06f,-9.8f,-13.657f));
		temp.push_back(glm::vec3(2.4f,-5.785f,-8.53f));
		temp.push_back(glm::vec3(7.5f,-7.5f,-13.657f));
		temp.push_back(glm::vec3(4.428f,-4.428f,-8.53f));
		temp.push_back(glm::vec3(9.8f,-4.059f,-13.657f));
		temp.push_back(glm::vec3(5.785f,-2.396f,-8.53f));
		temp.push_back(glm::vec3(10.608f,0,-13.657f));
		temp.push_back(glm::vec3(6.262f,0,-8.53f));
		temp.push_back(glm::vec3(9.8f,4.059f,-13.657f));
		temp.push_back(glm::vec3(5.785,2.396f,-8.53f));
		temp.push_back(glm::vec3(7.5f,7.5f,-13.657f));
		temp.push_back(glm::vec3(4.428f,4.428f,-8.53f));
		temp.push_back(glm::vec3(4.06f,9.8f,-13.657f));
		temp.push_back(glm::vec3(2.396f,5.785f,-8.53f));
		temp.push_back(glm::vec3(0,10.608f,-13.657f));
		temp.push_back(glm::vec3(0,6.262f,-8.53f));
		temp.push_back(glm::vec3(-4.06f,9.8f,-13.657f));
		temp.push_back(glm::vec3(-2.396f,5.785f,-8.53f));
		temp.push_back(glm::vec3(-7.5f,7.5f,-13.657f));
		temp.push_back(glm::vec3(-4.428f,4.428f,-8.53f));
		temp.push_back(glm::vec3(-9.8f,4.059f,-13.657f));
		temp.push_back(glm::vec3(-5.78f,2.396f,-8.53f));
		temp.push_back(glm::vec3(-10.6f,0,-13.657f));
		temp.push_back(glm::vec3(-6.262f,0,-8.53f));
		temp.push_back(glm::vec3(-9.8f,-4.059f,-13.657f));
		temp.push_back(glm::vec3(-5.785f,-2.396f,-8.53f));
		temp.push_back(glm::vec3(-7.5f,-7.5f,-13.657f));
		temp.push_back(glm::vec3(-4.428f,-4.428f,-8.53f));
		temp.push_back(glm::vec3(-4.06f,-9.8f,-13.657f));
		temp.push_back(glm::vec3(-2.396f,-5.785f,-8.53f));
		temp.push_back(glm::vec3(0,-10.608f,-13.657f));
		temp.push_back(glm::vec3(0,-6.262f,-8.53f));

		temp.push_back(glm::vec3(0,-6.262f,-8.53f));
		temp.push_back(glm::vec3(0,-3.964f,-3.46f));
		temp.push_back(glm::vec3(2.4f,-5.785f,-8.53f));
		temp.push_back(glm::vec3(1.517f,-3.662f,-3.46f));
		temp.push_back(glm::vec3(4.428f,-4.428f,-8.53f));
		temp.push_back(glm::vec3(2.803f,-2.803f,-3.46f));
		temp.push_back(glm::vec3(5.785f,-2.396f,-8.53f));
		temp.push_back(glm::vec3(3.662f,-1.517f,-3.46f));
		temp.push_back(glm::vec3(6.262f,0,-8.53f));
		temp.push_back(glm::vec3(3.964f,0,-3.46f));
		temp.push_back(glm::vec3(5.785,2.396f,-8.53f));
		temp.push_back(glm::vec3(3.662f,1.517f,-3.46f));
		temp.push_back(glm::vec3(4.428f,4.428f,-8.53f));
		temp.push_back(glm::vec3(2.803f,2.803f,-3.46f));
		temp.push_back(glm::vec3(2.396f,5.785f,-8.53f));
		temp.push_back(glm::vec3(1.517f,3.662f,-3.46f));
		temp.push_back(glm::vec3(0,6.262f,-8.53f));
		temp.push_back(glm::vec3(0,3.964f,-3.46f));
		temp.push_back(glm::vec3(-2.396f,5.785f,-8.53f));
		temp.push_back(glm::vec3(-1.517f,3.662f,-3.46f));
		temp.push_back(glm::vec3(-4.428f,4.428f,-8.53f));
		temp.push_back(glm::vec3(-2.803f,2.803f,-3.46f));
		temp.push_back(glm::vec3(-5.78f,2.396f,-8.53f));
		temp.push_back(glm::vec3(-3.662f,1.517f,-3.46f));
		temp.push_back(glm::vec3(-6.262f,0,-8.53f));
		temp.push_back(glm::vec3(-3.964f,0,-3.46f));
		temp.push_back(glm::vec3(-5.785f,-2.396f,-8.53f));
		temp.push_back(glm::vec3(-3.662f,-1.517f,-3.46f));
		temp.push_back(glm::vec3(-4.428f,-4.428f,-8.53f));
		temp.push_back(glm::vec3(-2.803f,-2.803f,-3.46f));
		temp.push_back(glm::vec3(-2.396f,-5.785f,-8.53f));
		temp.push_back(glm::vec3(-1.517f,-3.662f,-3.46f));
		temp.push_back(glm::vec3(0,-6.262f,-8.53f));
		temp.push_back(glm::vec3(0,-3.964f,-3.46f));

	std::vector<glm::vec3> temp2;

	for(auto point:temp)
		temp2.push_back(glm::vec3(point.x,point.y,-point.z));

	temp.insert(temp.end(),temp2.begin(),temp2.end());

	//middle ring (17 vertices)
	temp.push_back(glm::vec3(0,-2.668f,0));
	temp.push_back(glm::vec3(1.021f,-2.465f,0));
	temp.push_back(glm::vec3(1.887f,-1.887f,0));
	temp.push_back(glm::vec3(2.465f,-1.021f,0));
	temp.push_back(glm::vec3(2.668f,0,0));
	temp.push_back(glm::vec3(2.465f,1.021f,0));
	temp.push_back(glm::vec3(1.887f,1.887f,0));
	temp.push_back(glm::vec3(1.021f,2.465f,0));
	temp.push_back(glm::vec3(0,2.668f,0));
	temp.push_back(glm::vec3(-1.021f,2.465f,0));
	temp.push_back(glm::vec3(-1.887f,1.887f,0));
	temp.push_back(glm::vec3(-2.465f,1.021f,0));
	temp.push_back(glm::vec3(-2.668f,0,0));
	temp.push_back(glm::vec3(-2.465f,-1.021f,0));
	temp.push_back(glm::vec3(-1.887,-1.887f,0));
	temp.push_back(glm::vec3(-1.021f,-2.465f,0));
	temp.push_back(glm::vec3(0,-2.668f,0));


	return temp;
}
float _genRadius(std::vector<glm::vec3>& temp){
	float maxX = 0;
	float maxY = 0;
	float maxZ = 0;
	for(auto point:temp){
		float x = abs(point.x);
		float y = abs(point.y);
		float z = abs(point.z);

		if(x > maxX) maxX = x;
		if(y > maxY) maxY = y;
		if(z > maxZ) maxZ = z;
	}
	return glm::max(maxX, glm::max(maxY,maxZ));
}

std::vector<glm::vec3> Lagrange::m_Vertices = _genBuffer();
float Lagrange::radius = _genRadius(Lagrange::m_Vertices);

Lagrange::Lagrange(Planet* _planet1, Planet* _planet2, LAGRANGE_TYPE _type, std::string _name,Scene* _scene):ObjectBasic(glm::v3(0),glm::vec3(1),_name,_scene){
	_init(_planet1,_planet2,_type);
}
Lagrange::Lagrange(Planet* _planet1, Planet* _planet2, std::string _type, std::string _name,Scene* _scene):ObjectBasic(glm::v3(0),glm::vec3(1),_name,_scene){
	LAGRANGE_TYPE type;
	std::transform(_type.begin(), _type.end(), _type.begin(),std::tolower);
	if(_type == "l1")      type = LAGRANGE_TYPE_L1;
	else if(_type == "l2") type = LAGRANGE_TYPE_L2;
	else if(_type == "l3") type = LAGRANGE_TYPE_L3;
	else if(_type == "l4") type = LAGRANGE_TYPE_L4;
	else                   type = LAGRANGE_TYPE_L5;
	_init(_planet1,_planet2,type);
}
void Lagrange::_init(Planet* _planet1, Planet* _planet2, LAGRANGE_TYPE _type){
	m_Planet1 = _planet1;
	m_Planet2 = _planet2;
	m_Visible = true;
	m_Type = _type;

	_calculateLagrangePosition(_type);
	m_Scale = glm::vec3(2,2,2);

	m_Radius = Lagrange::radius * glm::max(glm::abs(m_Scale.x),glm::max(glm::abs(m_Scale.y),glm::abs(m_Scale.z)));
}
Lagrange::~Lagrange(){

}
void Lagrange::_calculateLagrangePosition(LAGRANGE_TYPE type){
	glm::v3 position = glm::v3(0);

	//first off we want the bigger body to be p1, the other is p2.
	Planet* p1 = m_Planet1; Planet* p2 = m_Planet2;
	if(p2->getRadius() >= p1->getRadius()){
		p1 = m_Planet2; p2 = m_Planet1;
	}

	float sizeRatio = p1->getRadius() / p2->getRadius();
	float planetaryDistanceToSubtractFromDistance = p1->getRadius() + p2->getRadius();
	glm::nType distanceFromP2AndP1 = glm::distance(p1->getPosition(),p2->getPosition());
	glm::nType actualDistance = distanceFromP2AndP1 - static_cast<glm::nType>(planetaryDistanceToSubtractFromDistance);

	glm::nType step1 = actualDistance / (sizeRatio + 1);
	glm::nType distanceOfL1FromP1 = (step1 * sizeRatio) + p1->getRadius();

	glm::v3 unitVector = glm::normalize(p2->getPosition() - p1->getPosition());
	this->alignTo(unitVector);

	unitVector *= distanceOfL1FromP1;
	position = p1->getPosition() + unitVector;
	setPosition(position);
}
void Lagrange::update(float dt){
	_calculateLagrangePosition(m_Type);
	Object::update(dt);
}
void Lagrange::render(GLuint shader, bool debug){
	//add to render queue
	if(shader == 0){
		shader = Resources::getShader("Deferred")->getShaderProgram();
	}
	Engine::Renderer::Detail::RenderManagement::getForegroundObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void Lagrange::draw(GLuint shader, bool debug){
	Camera* camera = Resources::getActiveCamera();
	if((m_Visible == false) || (!camera->sphereIntersectTest(this->getPosition(),this->getRadius())) || (camera->getDistance(this) > 1100 * getRadius()))
		return;	
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
	glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(glm::mat4(m_Model)));

	glUniform1i(glGetUniformLocation(shader, "DiffuseMapEnabled"),0);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),1);
	glUniform1f(glGetUniformLocation(shader, "BaseGlow"),1.0f);
	glUniform1f(glGetUniformLocation(shader, "Specularity"),0.0f);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
   	glBlendFunc(GL_ONE, GL_ONE);
	glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);

    //glDisable(GL_DEPTH_TEST);

	//first end
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),1,0,0,1);
	glBindBuffer( GL_ARRAY_BUFFER, m_Buffer );
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_QUAD_STRIP, 0, 68);
	glDisableVertexAttribArray(0);

	//second end
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),0,0,1,1);
	glBindBuffer( GL_ARRAY_BUFFER, m_Buffer );
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_QUAD_STRIP, 68, 68);
	glDisableVertexAttribArray(0);

	//third end
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),0,1,0,1);
	glBindBuffer( GL_ARRAY_BUFFER, m_Buffer );
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINE_STRIP, 136, 17);
	glDisableVertexAttribArray(0);

    //glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glUseProgram(0);
}