#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include <ecs/Components.h>

class Scene;
class LightProbe;
class Camera;

struct CameraType{enum Type {
    Perspective, 
    Orthographic,
};};

class Camera: public EntityWrapper{
    friend class  ::LightProbe;
    friend struct ::Engine::epriv::ComponentCameraFunctions;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Camera(float angle,float aspectRatio,float nearPlane,float farPlane,Scene* = nullptr);
        Camera(float left,float right,float bottom,float top,float nearPlane,float farPlane,Scene* = nullptr);
        virtual ~Camera();

        const glm::vec3 getPosition();
        const glm::vec3 forward();
        const glm::vec3 right();
        const glm::vec3 up();
        glm::quat getOrientation();

        float getDistance(Entity&);
        float getDistance(glm::vec3&);
        float getDistanceSquared(Entity&);
        float getDistanceSquared(glm::vec3&);

        const float getAngle();    void setAngle(float);
        const float getAspect();   void setAspect(float);
        const float getNear();     void setNear(float);
        const float getFar();      void setFar(float);

        const glm::mat4 getProjection();
        const glm::mat4 getProjectionInverse();
        const glm::vec3 getViewVector();

        const glm::mat4 getView();
        const glm::mat4 getViewInverse();
        const glm::mat4 getViewProjection();
        const glm::mat4 getViewProjectionInverse();

        uint sphereIntersectTest(glm::vec3& pos,float radius);
        uint pointIntersectTest(glm::vec3& pos);

        bool rayIntersectSphere(Entity&);
};
#endif
