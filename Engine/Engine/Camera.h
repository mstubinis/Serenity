#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Components.h"

class Scene;
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
        Camera(float angle,float aspectRatio,float nearPlane,float farPlane,Scene* = nullptr);
        Camera(float left,float right,float bottom,float top,float nearPlane,float farPlane,Scene* = nullptr);
        virtual ~Camera();

		glm::vec3 getPosition();
		glm::vec3 forward();
		glm::vec3 right();
		glm::vec3 up();
		glm::quat getOrientation();

		float getDistance(Entity*);
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

        bool sphereIntersectTest(Entity*);
        bool sphereIntersectTest(glm::vec3 pos, float radius);
		bool rayIntersectSphere(Entity*);
};
#endif
