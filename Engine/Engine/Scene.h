#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <glm/glm.hpp>

class SunLight;
class Object;
class Camera;
class Skybox;
class ParticleEmitter;

class Scene{
	private:
		Skybox* m_Skybox;
	protected:
		std::map<std::string,Camera*> m_Cameras;
		std::map<std::string,Object*> m_Objects;
		std::map<std::string,ParticleEmitter*> m_ParticleEmitters;
		std::map<std::string,SunLight*> m_Lights;

		std::string m_Name;

		glm::vec4 m_AmbientLighting;
	public:
		Scene(std::string name,glm::vec4 = glm::vec4(0.05f,0.05f,0.05f,0.05f));
		~Scene();

		std::map<std::string,Object*>& getObjects(){ return m_Objects; }
		std::map<std::string,Camera*>& getCameras(){ return m_Cameras; }
		std::map<std::string,ParticleEmitter*>& getParticleEmitters(){ return m_ParticleEmitters; }
		std::map<std::string,SunLight*>& getLights(){ return m_Lights; }

		const std::string getName() const { return m_Name; }

		virtual void setName(std::string);

		virtual void update(float);
		glm::vec4 getAmbientLightColor(){ return m_AmbientLighting; }
		void setAmbientLightColor(glm::vec4);
		void setAmbientLightColor(float,float,float,float);

		Skybox* getSkybox() const { return m_Skybox; }
		void setSkybox(Skybox* s){ m_Skybox = s; }
		void centerSceneToObject(Object*);
		void renderSkybox();
};
#endif