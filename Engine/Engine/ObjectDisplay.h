#pragma once
#ifndef ENGINE_OBJECTDISPLAY_H
#define ENGINE_OBJECTDISPLAY_H

#include "Object.h"
#include "RenderedItem.h"

class ObjectDisplay: public ObjectBasic{
    protected:
        bool m_Shadeless;
        bool m_Visible;
		bool m_PassedRenderCheck;
        std::vector<RenderedItem*> m_DisplayItems;
        glm::vec4 m_Color;
		glm::vec3 m_GodsRaysColor;
        glm::vec3 m_BoundingBoxRadius;
        virtual void calculateRadius();
    public:
        ObjectDisplay(std::string = "",
                      std::string = "",
                      glm::v3 = glm::v3(0),
                      glm::vec3 = glm::vec3(1),
                      std::string = "Visible Object",
                      Scene* = nullptr
                     );
        virtual ~ObjectDisplay();

		virtual void update(float);
		virtual void bind();
		virtual void unbind();
        virtual void render(GLuint=0,bool=false);
        virtual void draw(GLuint shader,bool=false,bool=false);

        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

        virtual void setColor(float,float,float,float); 
        virtual void setColor(glm::vec4);

        virtual void setGodsRaysColor(float,float,float); 
        virtual void setGodsRaysColor(glm::vec3);

        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        virtual glm::vec3& getRadiusBox(){ return m_BoundingBoxRadius; }

        glm::vec4& getColor(){ return m_Color; }
		glm::vec3& getGodsRaysColor(){ return m_GodsRaysColor; }
        std::vector<RenderedItem*>&  getDisplayItems(){ return m_DisplayItems; }

        virtual void setVisible(bool b);

		virtual bool visible() { return m_Visible; }
		virtual bool passedRenderCheck(){ return m_PassedRenderCheck; }
        virtual bool rayIntersectSphere(Camera* = nullptr);
        virtual bool rayIntersectSphere(glm::v3 origin, glm::vec3 vector);
};
#endif