#pragma once
#ifndef ENGINE_OBJECT_H
#define ENGINE_OBJECT_H

#include "Engine_Math.h"
#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>

class Mesh;
class Material;
class Scene;
class Camera;
typedef unsigned int GLuint;

template <typename T> bool exists(boost::weak_ptr<T> t){ return !(t.expired()); }

class IObject{
    public:
        virtual void update(float) = 0;

        virtual void setPosition(glm::nType,glm::nType,glm::nType) = 0;
        virtual void setPosition(glm::v3) = 0;
        virtual void setScale(float,float,float) = 0;
        virtual void setScale(glm::vec3) = 0;

        virtual void translate(glm::nType,glm::nType,glm::nType,bool local=true) = 0;
        virtual void translate(glm::v3,bool local=true) = 0;
        virtual void rotate(float,float,float,bool overTime = true) = 0;
        virtual void rotate(glm::vec3, bool overTime = true) = 0;
        virtual void scale(float,float,float) = 0;
        virtual void scale(glm::vec3) = 0;

        virtual glm::quat getOrientation() = 0;
        virtual glm::v3 getPosition() = 0;
        virtual glm::vec3 getScale() = 0;
        virtual glm::v3 getForward() = 0;
        virtual glm::v3 getRight() = 0;
        virtual glm::v3 getUp() = 0;
        virtual glm::m4 getModel() = 0;
        virtual glm::v3 getMotionVector() = 0;
};

class Object: public IObject{
    private:
        bool m_IsToBeDestroyed;
    protected:
        std::string m_Name;
        Object* m_Parent;
        float m_Radius;
        std::vector<Object*> m_Children;
    public:
        Object(
                std::string = "Object",   //Object
                Scene* = nullptr,         //The scene to add the object to (default nullptr = the current scene)
                bool = true               //This is not a camera
        );
        virtual ~Object();

        void destroy(){ m_IsToBeDestroyed = true; }
        bool isDestroyed(){ return m_IsToBeDestroyed; }

        virtual glm::nType getDistance(Object*);
        virtual unsigned long long getDistanceLL(Object*);

        virtual glm::vec3 getScreenCoordinates();

        virtual void alignTo(glm::v3,float speed=0,bool overTime=false){}

        virtual void addChild(Object*);

        virtual void update(float){}
        virtual void render(GLuint=0,bool=false){}
        virtual void draw(GLuint shader,bool=false,bool=false){}

        virtual float getRadius(){ return m_Radius; }
        virtual std::string& getName(){ return m_Name; }

        virtual Object* getParent(){ return m_Parent; }
        virtual std::vector<Object*>& getChildren(){ return m_Children; }

        virtual void setName(std::string);

        virtual bool rayIntersectSphere(Camera*){return false;}
        virtual bool rayIntersectSphere(glm::v3 origin, glm::vec3 vector){return false;}
};

class ObjectBasic: public Object{
    protected:
        glm::v3 m_Forward, m_Right, m_Up;
        glm::vec3 m_Scale;
        glm::v3 m_Position;
        glm::m4 m_Model;
        glm::quat m_Orientation;
        glm::v3 _prevPosition;
    public:
        ObjectBasic(
                glm::v3 = glm::v3(0),         //Position
                glm::vec3 = glm::vec3(1),     //Scale
                std::string = "Object Basic", //Object name
                Scene* = nullptr,             //The scene to add the object to (default nullptr = the current scene)
                bool = true                   //This is not a camera
        );
        virtual ~ObjectBasic();

        virtual void setPosition(glm::nType,glm::nType,glm::nType);
        virtual void setPosition(glm::v3);
        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

        virtual void rotate(float,float,float,bool overTime = true); 
        virtual void rotate(glm::vec3, bool overTime = true);
        virtual void translate(glm::nType,glm::nType,glm::nType,bool local=true); 
        virtual void translate(glm::v3,bool local=true);
        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        virtual void update(float);

        virtual glm::v3 getForward(){ return m_Forward; }
        virtual glm::v3 getRight(){ return m_Right; }
        virtual glm::v3 getUp(){ return m_Up; }
        virtual glm::v3 getPosition(){ return glm::v3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
        virtual glm::vec3 getScale(){ return m_Scale; }
        virtual glm::m4 getModel(){ return m_Model; }
        virtual glm::quat getOrientation(){ return m_Orientation; }
        virtual void setOrientation(glm::quat q){ m_Orientation = q; }
        virtual glm::v3 getMotionVector() { return getPosition() - _prevPosition; }

        virtual void alignTo(glm::v3,float speed=0,bool overTime=false);
};
#endif