#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Object.h"

class Scene;
class ObjectDisplay;
class ObjectDynamic;
enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };

class Camera: public ObjectBasic{
    private:
        void _constructFrustrum();
        glm::vec4 m_Planes[6];
        Scene* m_Scene;
    protected:
        CAMERA_TYPE m_Type;
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
        virtual void setOrthoProjection(float,float,float,float);

        virtual void update(float);

        virtual void lookAt(glm::v3);  
        virtual void lookAt(glm::v3,glm::v3); 
        virtual void lookAt(glm::v3,glm::v3,glm::v3); 
        virtual void lookAt(Object*,bool targetUp = false);

        virtual void setAspectRatio(float);

        const float getAngle() const { return m_Angle; }
        const float getAspectRatio() const { return m_AspectRatio; }
        const float getNear() const { return m_Near; }
        const float getFar() const { return m_Far; }

        Scene* getScene(){ return m_Scene; }

        glm::quat getOrientation(){ return m_Orientation; }

        virtual glm::mat4 calculateProjection(glm::mat4 model){ return m_Projection * m_View * model; }
        virtual glm::mat4 calculateModelView(glm::mat4 model){ return m_View * model; }
        virtual glm::mat4 calculateViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
        glm::mat4 getProjection(){ return m_Projection; }
        glm::mat4 getView(){ return m_View; }
        glm::mat4 getViewProjection(){ return m_Projection * m_View; }
        glm::vec3 getViewVector(){ return glm::vec3(m_View[0][2],m_View[1][2],m_View[2][2]); }
        const CAMERA_TYPE getType() const{ return m_Type; }

        virtual bool sphereIntersectTest(Object*);
        virtual bool sphereIntersectTest(glm::v3 pos, float radius);

        //ray tests
        virtual bool rayIntersectSphere(Object*);
};
#endif