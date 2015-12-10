#ifndef LAGRANGE_H
#define LAGRANGE_H

#include "Object.h"

class Planet;
class Lagrange: public Object{
	private:
		Planet* m_Planet1;
		Planet* m_Planet2;
		bool m_Visible;
		void _calculateLagrangePosition();

		static float radius;
		static GLuint m_Buffer;
		static std::vector<glm::vec3> m_Vertices;

	public:
		Lagrange(Planet*,
				 Planet*,
				 std::string = "Lagrange Point",   //Object name
				 Scene* = nullptr
			   );
		~Lagrange();


		static void _genBuffers(){
			glGenBuffers(1, &Lagrange::m_Buffer);

			glBindBuffer(GL_ARRAY_BUFFER, Lagrange::m_Buffer );
			glBufferData(GL_ARRAY_BUFFER, Lagrange::m_Vertices.size() * sizeof(glm::vec3),&Lagrange::m_Vertices[0], GL_STATIC_DRAW );
		}

		void update(float);
		void render(Mesh*, Material*,bool=false);
		void render(bool=false);
};

#endif