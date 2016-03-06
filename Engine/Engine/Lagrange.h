#ifndef LAGRANGE_H
#define LAGRANGE_H

#include "Object.h"

class Planet;
enum LAGRANGE_TYPE { LAGRANGE_TYPE_L1,LAGRANGE_TYPE_L2,LAGRANGE_TYPE_L3,LAGRANGE_TYPE_L4,LAGRANGE_TYPE_L5 };
class Lagrange: public Object{
	private:
		Planet* m_Planet1;
		Planet* m_Planet2;
		bool m_Visible;
		void _calculateLagrangePosition(LAGRANGE_TYPE);
		void _init(Planet*,Planet*,LAGRANGE_TYPE);

		LAGRANGE_TYPE m_Type;

		static float radius;
		static GLuint m_Buffer;
		static std::vector<glm::vec3> m_Vertices;

	public:
		Lagrange(Planet*,
				 Planet*,
				 LAGRANGE_TYPE = LAGRANGE_TYPE_L1,
				 std::string = "Lagrange Point",   //Object name
				 Scene* = nullptr
			   );
		Lagrange(Planet*,
				 Planet*,
				 std::string = "L1",
				 std::string = "Lagrange Point",   //Object name
				 Scene* = nullptr
			   );
		virtual ~Lagrange();


		static void _genBuffers(){
			glGenBuffers(1, &Lagrange::m_Buffer);

			glBindBuffer(GL_ARRAY_BUFFER, Lagrange::m_Buffer );
			glBufferData(GL_ARRAY_BUFFER, Lagrange::m_Vertices.size() * sizeof(glm::vec3),&Lagrange::m_Vertices[0], GL_STATIC_DRAW );
		}

		const LAGRANGE_TYPE getType() const{ return m_Type; }

		glm::vec2 getGravityInfo(){ return glm::vec2(this->getRadius()*100,this->getRadius()*150); }

		void update(float);
		void render(GLuint=0, bool=false);
		void draw(GLuint=0,bool=false);
};

#endif