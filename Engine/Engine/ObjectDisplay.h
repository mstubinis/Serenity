#ifndef OBJECTDISPLAY_H
#define OBJECTDISPLAY_H

#include "Object.h"

class ObjectDisplay: public Object{
	protected:
		Mesh* m_Mesh;
		Material* m_Material;
		glm::vec4 m_Color;

		float m_Radius;
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

		virtual void _updateMatrix();
		virtual void render(Mesh*, Material*,bool=false);
		virtual void render(bool=false);

		virtual void setColor(float,float,float,float); 
		virtual void setColor(glm::vec4);

		virtual void scale(float,float,float);
		virtual void scale(glm::vec3);

		virtual const glm::vec3& getRadiusBox() const{ return m_BoundingBoxRadius; }
		virtual const float getRadius() const { return m_Radius; }

		const glm::vec4& getColor() const{ return m_Color; }

		void setMesh(Mesh*);
		void setMaterial(Material*);

		bool rayIntersectSphere(Camera*);
};
#endif