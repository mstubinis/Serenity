#pragma once
#ifndef ENGINE_OBJECT_H
#define ENGINE_OBJECT_H

#include "BindableResource.h"
#include "Engine_Math.h"

class Mesh;
class Material;
class Scene;
class Camera;
class MeshInstance;
typedef unsigned int GLuint;

template <typename T> bool exists(const boost::weak_ptr<T>& t){ if(t.expired() || !t.lock().get()) return false; return true; }

class IObject: public BindableResource{
    public:
        virtual void update(float) = 0;

        virtual void setPosition(float,float,float) = 0;
        virtual void setPosition(glm::vec3) = 0;
        virtual void setScale(float,float,float) = 0;
        virtual void setScale(glm::vec3) = 0;

        virtual void setOrientation(glm::quat) = 0;

        virtual void translate(float,float,float,bool local=true) = 0;
        virtual void translate(glm::vec3,bool local=true) = 0;
        virtual void rotate(float,float,float,bool overTime = true) = 0;
        virtual void rotate(glm::vec3, bool overTime = true) = 0;
        virtual void scale(float,float,float) = 0;
        virtual void scale(glm::vec3) = 0;

        virtual void suspend() = 0;
        virtual void resume() = 0;

        virtual glm::quat& getOrientation() = 0;
        virtual glm::vec3 getPosition() = 0;
        virtual glm::vec3 getScale() = 0;
        virtual glm::vec3 getForward() = 0;
        virtual glm::vec3 getRight() = 0;
        virtual glm::vec3 getUp() = 0;
        virtual glm::mat4& getModel() = 0;
};

class Object: public IObject{
    friend class MeshInstance;
    private:
        bool m_IsToBeDestroyed;
    protected:
        static float m_VisibilityThreshold;
        static float m_RotationThreshold;
        Object* m_Parent;
        float m_Radius;
        std::vector<Object*> m_Children;
    public:
        static void setGlobalRotationThreshold(float t){ m_RotationThreshold = t; }
        static void setGlobalVisibilityThreshold(float t){ m_VisibilityThreshold = t; }
        Object(
            std::string = "Object",   //Object
            Scene* = nullptr,         //The scene to add the object to (default nullptr = the current scene)
            bool = true               //This is not a camera
        );
        virtual ~Object();

        void destroy(){ m_IsToBeDestroyed = true; }
        bool isDestroyed(){ return m_IsToBeDestroyed; }

        virtual float getDistance(Object*);
        virtual unsigned long long getDistanceLL(Object*);

        virtual glm::vec3 getScreenCoordinates();

        virtual void lookAt(glm::vec3,glm::vec3,glm::vec3){}
        virtual void lookAt(Object*){}
        virtual void alignTo(glm::vec3,float speed=0){}
        virtual void alignTo(Object*,float speed=0){}
        virtual void alignToX(Object*,float speed=0){}
        virtual void alignToY(Object*,float speed=0){}
        virtual void alignToZ(Object*,float speed=0){}

        virtual void suspend(){}
        virtual void resume(){}

        virtual void addChild(Object*);

        virtual void update(float){}
        virtual float getRadius(){ return m_Radius; }

        virtual Object* getParent(){ return m_Parent; }
        virtual std::vector<Object*>& getChildren(){ return m_Children; }

        virtual bool visible(){ return false; }
        virtual bool checkRender(Camera*){ return false; }
        virtual bool rayIntersectSphere(Camera* = nullptr){return false;}
        virtual bool rayIntersectSphere(glm::vec3 origin, glm::vec3 vector){return false;}
};

class ObjectBasic: public Object{
    protected:
        glm::vec3 m_Forward, m_Right, m_Up;
        glm::vec3 m_Scale;
        glm::vec3 m_Position;
        glm::mat4 m_Model;
        glm::quat m_Orientation;
    public:
        ObjectBasic(
            glm::vec3 = glm::vec3(0),     //Position
            glm::vec3 = glm::vec3(1),     //Scale
            std::string = "Object Basic", //Object name
            Scene* = nullptr,             //The scene to add the object to (default nullptr = the current scene)
            bool = true                   //This is not a camera
        );
        virtual ~ObjectBasic();

        virtual void setPosition(float,float,float);
        virtual void setPosition(glm::vec3);
        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

        virtual void rotate(float,float,float,bool overTime = true); 
        virtual void rotate(glm::vec3, bool overTime = true);
        virtual void translate(float,float,float,bool local=true); 
        virtual void translate(glm::vec3,bool local=true);
        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        virtual void update(float);

        virtual glm::vec3 getForward();
        virtual glm::vec3 getRight();
        virtual glm::vec3 getUp();
		virtual glm::vec3 getPosition(){ return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		virtual glm::vec3 getScale(){ return m_Scale; }
		virtual glm::mat4& getModel(){ return m_Model; }
		virtual glm::quat& getOrientation(){ return m_Orientation; }
        virtual void setOrientation(glm::quat);

        virtual void lookAt(glm::vec3,glm::vec3,glm::vec3);
        virtual void lookAt(Object*);

        virtual void alignTo(glm::vec3,float speed=0);
        virtual void alignTo(Object*,float speed=0);
        virtual void alignToX(Object*,float speed=0);
        virtual void alignToY(Object*,float speed=0);
        virtual void alignToZ(Object*,float speed=0);
};
#endif
