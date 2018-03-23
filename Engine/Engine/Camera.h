#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Components.h"

#include "Object.h"

class Scene;
class ObjectDisplay;
class LightProbe;
class CameraType{public: enum Type { 
    Perspective, 
    Orthographic,
};};

class Camera: public Entity{
    friend class LightProbe;
    private:
		class impl; std::unique_ptr<impl> m_i;
    protected:
		ComponentBasicBody* m_BasicBody;
		ComponentCamera* m_Camera;
    public:
        Camera(float,float,float,float,Scene* = nullptr);             // Perspective camera Constructor
        Camera(float,float,float,float,float,float,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~Camera();

		glm::vec3 getPosition();
		glm::vec3 forward();
		glm::vec3 right();
		glm::vec3 up();
		glm::quat getOrientation();

		float getDistance(Object*);
		float getDistance(glm::vec3);

        float getNear();
        float getFar();

        glm::mat4 getViewProjectionInverse();
        glm::mat4 getProjection();
        glm::mat4 getView();
		glm::mat4 getViewInverse();
		glm::mat4 getProjectionInverse();
        glm::mat4 getViewProjection();
        glm::vec3 getViewVector();

        bool sphereIntersectTest(Object*);
        bool sphereIntersectTest(glm::vec3 pos, float radius);
        bool rayIntersectSphere(Object*);
		bool rayIntersectSphere(Entity*);
};
#endif
