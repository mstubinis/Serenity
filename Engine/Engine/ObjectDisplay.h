#ifndef ENGINE_OBJECTDISPLAY_H
#define ENGINE_OBJECTDISPLAY_H

#include "Object.h"

struct DisplayItem{
	Mesh* mesh;
	Material* material;
	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scale;
	DisplayItem(Mesh* _m, Material* _mat, glm::vec3 _p = glm::vec3(0), glm::quat _o = glm::quat(), glm::vec3 _s = glm::vec3(1)){
		mesh = _m; material = _mat; position = _p; orientation = _o; scale = _s;
	}
	~DisplayItem(){
	}
};
class ObjectDisplay: public Object{
	protected:
		bool m_Visible;
		std::vector<DisplayItem*> m_DisplayItems;
		glm::vec4 m_Color;
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

		virtual void render(GLuint=0,bool=false);
		virtual void draw(GLuint shader,bool=false);

		virtual void setColor(float,float,float,float); 
		virtual void setColor(glm::vec4);

		virtual void scale(float,float,float);
		virtual void scale(glm::vec3);

		virtual const glm::vec3& getRadiusBox() const{ return m_BoundingBoxRadius; }

		const glm::vec4& getColor() const{ return m_Color; }

		void setVisible(bool b);

		bool rayIntersectSphere(Camera*);
		bool rayIntersectSphere(glm::v3 origin, glm::vec3 vector);
};
#endif