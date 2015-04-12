#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/GL.h>

class btCollisionShape;
class btTriangleMesh;
class btConvexHullShape;

const unsigned int NUM_VERTEX_DATA = 5;
const unsigned int VERTEX_AMOUNTS[NUM_VERTEX_DATA] = {3,2,3,3,4};

struct Vertex{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	Vertex(){
		uv = glm::vec2(0,0);
		normal = tangent = glm::vec3(0,0,0);
	}
};

class Mesh{
	private:
		GLuint m_buffers[NUM_VERTEX_DATA]; //0 - position, 1 - uv, 2 - normal, 3 - tangent, 4 - color
		btConvexHullShape* m_Collision;

		glm::vec3 m_radius;
		std::vector<glm::vec3> m_Points;
		std::vector<glm::vec4> m_Colors;
		std::vector<glm::vec2> m_UVs;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;

		void GenerateQuad(Vertex&,Vertex&,Vertex&,Vertex&);
		void GenerateTriangle(Vertex&,Vertex&,Vertex&);

		void CalculateTangent(Vertex&, Vertex&, Vertex&);

		void LoadFromFile(std::string);
		void LoadFromPLY(std::string);
		void LoadFromOBJ(std::string);

		btConvexHullShape* LoadColFromOBJ(std::string);

		void Init();
	public:
		Mesh(int x, int y, int width, int height);
		Mesh(std::string = "");
		~Mesh();

		//GLuint* VAO();
		const GLuint* Buffers() const { return m_buffers; }
		btConvexHullShape* Collision() const { return m_Collision; }

		const glm::vec3 Radius() const { return m_radius; }

		const std::vector<glm::vec3>& Points() const { return m_Points; }
		const std::vector<glm::vec4>& Colors() const { return m_Colors; }
		const std::vector<glm::vec2>& UVS() const { return m_UVs; }
		const std::vector<glm::vec3>& Normals() const { return m_Normals; }
		const std::vector<glm::vec3>& Tangents() const { return m_Tangents; }

		void Render();
};
#endif