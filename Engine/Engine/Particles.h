#ifndef PARTICLES_H
#define PARTICLES_H

#include "Object.h"
#include "Light.h"
#include "Engine_Resources.h"


struct ParticleInfo{
	Material* material;

	glm::vec4 lightColor;

	//TODO: create a color animation framework that can be x number of colors and animating them at y speeds
	glm::vec4 startColor;
	glm::vec4 endColor;

	ParticleInfo(std::string _material,glm::vec4 _startColor = glm::vec4(1,1,1,1), glm::vec4 _endColor = glm::vec4(1,1,1,1)){
		material = Engine::Resources::getMaterial(_material);
		startColor = _startColor;
		endColor = _endColor;
	}
	ParticleInfo(Material* _material,glm::vec4 _startColor = glm::vec4(1,1,1,1), glm::vec4 _endColor = glm::vec4(1,1,1,1)){
		material = _material;
		startColor = _startColor;
		endColor = _endColor;
	}
	~ParticleInfo(){
	}
};

class Particle;

class ParticleEmitter: public Object{
	private:
		ParticleInfo* m_info;
		std::vector<Particle> m_Particles;
	public:
		ParticleEmitter(ParticleInfo*, glm::vec3 = glm::vec3(0,0,0), glm::vec3 = glm::vec3(1,1,1) ,std::string = "Particle Emitter", Scene* = nullptr);
		~ParticleEmitter();

		std::vector<Particle> getParticles(){ return m_Particles; }
		const ParticleInfo* getParticleInfo() const { return m_info; }

		void addParticle();
		void update(float);
		void render();
};

class Particle{
	private:
		SunLight* light;
		ParticleEmitter* emitter;

		glm::mat4 model;
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec2 scale;
		glm::vec2 scaleVelocity;

		glm::vec4 color;

		float zRot;
		float zRotVelocity;
	public:
		Particle(ParticleEmitter*,glm::vec3 pos,glm::vec4 col,glm::vec2 scl = glm::vec2(1,1) ,float rot = 0,glm::vec3 vel = glm::vec3(0,0,0),float rVel = 0, glm::vec2 sVel = glm::vec2(0,0));
		~Particle();
		void update(float);

		void setPosition(float,float,float); 
		void setPosition(glm::vec3);

		const glm::vec3& getPosition(){ 
			position = glm::vec3(model[3][0],model[3][1],model[3][2]);
			return position; 
		}

		void render(GLuint);
};
#endif