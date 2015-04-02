#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm\glm.hpp>
#include <GL\glew.h>
#include <SFML\OpenGL.hpp>

class btCollisionShape;
class btTriangleMesh;

const unsigned int NUM_VERTEX_DATA = 6;
const unsigned int VERTEX_AMOUNTS[NUM_VERTEX_DATA] = {3,2,3,3,3,4};

struct Vertex{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 binormal;
	glm::vec3 tangent;
};

class Mesh{
	private:
		GLuint m_buffers[NUM_VERTEX_DATA]; //0 - position, 1 - uv, 2 - normal, 3 - binormal, 4 - tangent, 5 - color
		btCollisionShape* m_Collision;

		glm::vec3 m_radius;
		std::vector<glm::vec3> m_Points;
		std::vector<glm::vec4> m_Colors;
		std::vector<glm::vec2> m_UVs;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Binormals;
		std::vector<glm::vec3> m_Tangents;

		void GenerateQuad(btTriangleMesh*,Vertex&,Vertex&,Vertex&,Vertex&);
		void GenerateTriangle(btTriangleMesh*,Vertex&,Vertex&,Vertex&);

		void CalculateTangentBinormal(Vertex&, Vertex&, Vertex&);

		void LoadFromFile(std::string);
		void LoadFromPLY(std::string);
		void LoadFromOBJ(std::string);

		void Init();
	public:
		Mesh(int x, int y, int width, int height);
		Mesh(std::string = "");
		~Mesh();

		GLuint* VAO();
		GLuint* Buffers();
		btCollisionShape* Collision() const { return m_Collision; }

		glm::vec3 Radius();

		std::vector<glm::vec3>& Points();
		std::vector<glm::vec4>& Colors();
		std::vector<glm::vec2>& UVS();
		std::vector<glm::vec3>& Normals();
		std::vector<glm::vec3>& Binormals();
		std::vector<glm::vec3>& Tangents();

		void Render();
};
#endif