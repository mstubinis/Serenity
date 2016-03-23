#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include <map>
#include <string>
#include <glm/glm.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Object;
class Camera;
class SunLight;
class Skybox;
class ParticleEmitter;

class Scene{
	private:
		Skybox* m_Skybox;
	protected:
		std::map<std::string,Object*> m_Objects;
		std::map<std::string,ParticleEmitter*> m_ParticleEmitters;
		std::map<std::string,SunLight*> m_Lights;

		std::string m_Name;

		glm::vec3 m_AmbientLighting;
		glm::vec3 m_BackgroundColor;
	public:
		Scene(std::string name,glm::vec3 = glm::vec3(0.025f,0.025f,0.025f));
		virtual ~Scene();

		std::map<std::string,Object*>& getObjects(){ return m_Objects; }
		std::map<std::string,ParticleEmitter*>& getParticleEmitters(){ return m_ParticleEmitters; }
		std::map<std::string,SunLight*>& getLights(){ return m_Lights; }

		Object* getObject(std::string name){ return m_Objects[name]; }
		ParticleEmitter* getParticleEmitter(std::string name){ return m_ParticleEmitters[name]; }
		SunLight* getLight(std::string name){ return m_Lights[name]; }

		const std::string getName() const { return m_Name; }

		virtual void setName(std::string);

		virtual void update(float);
		glm::vec3 getAmbientLightColor(){ return m_AmbientLighting; }
		glm::vec3 getBackgroundColor(){ return m_BackgroundColor; }
		void setAmbientLightColor(glm::vec3);
		void setAmbientLightColor(float,float,float);
		void setBackgroundColor(float,float,float);

		Skybox* getSkybox() const { return m_Skybox; }
		void setSkybox(Skybox* s){ m_Skybox = s; }
		void centerSceneToObject(Object*);
		void renderSkybox();
};
#endif