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
		std::vector<Particle*> m_Particles;
	public:
		ParticleEmitter(ParticleInfo*, glm::v3 = glm::v3(0), glm::vec3 = glm::vec3(1) ,std::string = "Particle Emitter", Scene* = nullptr);
		virtual ~ParticleEmitter();

		std::vector<Particle*> getParticles(){ return m_Particles; }
		const ParticleInfo* getParticleInfo() const { return m_info; }

		void addParticle();
		void deleteParticles();
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

		float lifetime;
		float lifetimeMax;
		bool toBeErased;

		float zRot;
		float zRotVelocity;
	public:
		Particle(ParticleEmitter*,glm::v3 pos,glm::vec4 col,glm::vec2 scl = glm::vec2(1,1) ,float rot = 0,glm::vec3 vel = glm::vec3(0,0,0),float rVel = 0, glm::vec2 sVel = glm::vec2(0,0));
		~Particle();
		void update(float);

		void setPosition(glm::nType,glm::nType,glm::nType); 
		void setPosition(glm::v3);

		const bool ToBeErased() const { return toBeErased; }

		const glm::vec3& getPosition(){ 
			position = glm::vec3(model[3][0],model[3][1],model[3][2]);
			return position; 
		}

		void render(GLuint);
};
#endif