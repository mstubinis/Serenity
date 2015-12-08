#include "Particles.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "Scene.h"
#include "Engine.h"

#include <boost/lexical_cast.hpp>

ParticleEmitter::ParticleEmitter(ParticleInfo* info, glm::vec3 pos, glm::vec3 scl,std::string name, Scene* scene):Object(pos,scl,name,true,scene){
	m_info = info;
	unsigned int count = 0;
	if(scene == nullptr){
		scene = Engine::Resources::getCurrentScene();
	}
	if (scene->getParticleEmitters().size() > 0){
		while(scene->getParticleEmitters().count(m_Name)){
			m_Name = name + " " + boost::lexical_cast<std::string>(count);
			count++;
		}
	}
	scene->getParticleEmitters()[m_Name] = this;
}
ParticleEmitter::~ParticleEmitter(){
	for(auto particle:m_Particles) delete particle;
}
void ParticleEmitter::addParticle(){
	float rot = rand() % 360;

	glm::vec3 velocity = glm::vec3(0,0.05f,0);
	float rotationalVelocity = 0.05f;
	m_Particles.push_back(new Particle(this,this->getPosition(),m_info->startColor,glm::vec2(1,1),rot,velocity,rotationalVelocity,glm::vec2(0,0)));
}
void ParticleEmitter::update(float dt)
{
	Object::update(dt);
	for(auto particle:m_Particles)
		particle->update(dt);
}
void ParticleEmitter::render(){
	return;
}

Particle::Particle(ParticleEmitter* _emitter,glm::vec3 pos,glm::vec4 col,glm::vec2 scl ,float rot,glm::vec3 vel,float rVel, glm::vec2 sVel){
	emitter = _emitter;
	position = pos;
	color = col;
	scale = scl;
	zRot = rot;
	velocity = vel;
	zRotVelocity = rVel;
	scaleVelocity = sVel;

	model = glm::mat4(1);
	model = glm::translate(model,position);
	model = glm::rotate(model, zRot,glm::vec3(0,0,1));
	model = glm::scale(model, glm::vec3(scale.x,scale.y,1));
}
Particle::~Particle(){
	//delete light; 
}
void Particle::setPosition(float x,float y,float z){
	position.x = x;
	position.y = y;
	position.z = z;

	model[3][0] = position.x;
	model[3][1] = position.y;
	model[3][2] = position.z;
}
void Particle::setPosition(glm::vec3 pos){ setPosition(pos.x,pos.y,pos.z); }
void Particle::update(float dt){
	position += velocity*dt;
	zRot += zRotVelocity*dt;
	scale += scaleVelocity*dt;

	model = glm::mat4(1);
	model = glm::translate(model,position);
	model = glm::rotate(model, zRot,glm::vec3(0,0,1));
	model = glm::scale(model, glm::vec3(scale.x,scale.y,1));
}
void Particle::render(GLuint shader){


}