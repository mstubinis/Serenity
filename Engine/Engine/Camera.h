#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Object.h"

class Scene;
class ObjectDisplay;
class ObjectDynamic;
class LightProbe;
class CameraType{public: enum Type { 
    Perspective, 
    Orthographic,
};};

class Camera: public ObjectBasic{
    friend class LightProbe;
    private:
        void _constructFrustrum();
        glm::vec4 m_Planes[6];
        Scene* m_Scene;
    protected:
        CameraType::Type m_Type;
        float m_Angle;
        float m_AspectRatio;
        float m_Near, m_Far;
        glm::mat4 m_View, m_Projection;
    public:
        Camera(std::string,float,float,float,float,Scene* = nullptr);             // Perspective camera Constructor
        Camera(std::string,float,float,float,float,float,float,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~Camera();

        virtual void resize(unsigned int w, unsigned int h);
        virtual void setPerspectiveProjection();
		virtual void setPerspectiveProjection(float angle,float aspectRatio,float near,float far);
        virtual void setOrthoProjection(float,float,float,float);

        virtual void update(float);

        virtual void lookAt(glm::vec3);  
        virtual void lookAt(glm::vec3,glm::vec3); 
        virtual void lookAt(glm::vec3,glm::vec3,glm::vec3); 
        virtual void lookAt(Object*,bool targetUp = false);

        virtual void setAspectRatio(float);

        const float getAngle() const;
        const float getAspectRatio() const;
        const float getNear() const;
        const float getFar() const;

        Scene* getScene(){ return m_Scene; }

        glm::quat& getOrientation(){ return m_Orientation; }

        virtual glm::mat4 getViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
        glm::mat4 getProjection();
        glm::mat4 getView();
        glm::mat4 getViewProjection();
        glm::vec3 getViewVector();
        const CameraType::Type getType() const{ return m_Type; }

        virtual bool sphereIntersectTest(Object*);
        virtual bool sphereIntersectTest(glm::vec3 pos, float radius);

        //ray tests
        virtual bool rayIntersectSphere(Object*);
};
#endif
