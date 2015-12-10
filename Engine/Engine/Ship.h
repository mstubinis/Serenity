#ifndef SHIP_H
#define SHIP_H

#include "ObjectDynamic.h"
class GameCamera;
class Ship: public ObjectDynamic{
	protected:
		bool m_IsWarping;
		float m_WarpFactor;
		Object* m_Target;
	public:
		Ship(
			std::string,                          //Mesh
			std::string,                          //Material
			std::string = "Ship",                 //Name
			glm::vec3 = glm::vec3(0,0,0),         //Position
			glm::vec3 = glm::vec3(1,1,1),         //Scale
			Engine::Physics::Collision* = nullptr,//Bullet Collision Shape
			Scene* = nullptr
		    );
		~Ship();

		void translateWarp(float);

		Object* getTarget() { return m_Target; }
		virtual void setTarget(Object*);
		virtual void update(float);
};

class PlayerShip: public Ship{
	private:
		GameCamera* m_Camera;
	public:
		PlayerShip(
			std::string,                          //Mesh
			std::string,                          //Material
			std::string = "Player Ship",          //Name
			glm::vec3 = glm::vec3(0,0,0),         //Position
			glm::vec3 = glm::vec3(1,1,1),         //Scale
			Engine::Physics::Collision* = nullptr,//Bullet Collision Shape
			Scene* = nullptr
		    );
		~PlayerShip();

		void update(float);
		void setTarget(Object*);
};

#endif