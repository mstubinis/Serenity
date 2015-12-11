#ifndef OBJECTDISPLAY_H
#define OBJECTDISPLAY_H

#include "Object.h"

class ObjectDisplay: public Object{
	protected:
		bool m_Visible;
		Mesh* m_Mesh;
		Material* m_Material;
		glm::vec4 m_Color;

		glm::vec3 m_BoundingBoxRadius;

		virtual void calculateRadius();
	public:
		ObjectDisplay(std::string = "",
				      std::string = "",
					  glm::vec3 = glm::vec3(0,0,0),
					  glm::vec3 = glm::vec3(1,1,1),
					  std::string = "Visible Object",
					  bool isNotLight = true,
				      Scene* = nullptr
					 );
		~ObjectDisplay();

		virtual void render(Mesh*, Material*,GLuint=0,bool=false);
		virtual void render(GLuint=0,bool=false);
		virtual void draw(Mesh*, Material*,GLuint shader,bool=false);

		virtual void setColor(float,float,float,float); 
		virtual void setColor(glm::vec4);

		virtual void scale(float,float,float);
		virtual void scale(glm::vec3);

		virtual const glm::vec3& getRadiusBox() const{ return m_BoundingBoxRadius; }

		const glm::vec4& getColor() const{ return m_Color; }

		void setMesh(Mesh*);
		void setMaterial(Material*);
		void setVisible(bool b);

		bool rayIntersectSphere(Camera*);
		bool rayIntersectSphere(glm::vec3 origin, glm::vec3 vector);
};
#endif