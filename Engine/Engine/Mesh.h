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

		void _generateQuad(Vertex&,Vertex&,Vertex&,Vertex&);
		void _generateTriangle(Vertex&,Vertex&,Vertex&);

		void _calculateTangent(Vertex&, Vertex&, Vertex&);

		void _loadFromFile(std::string);
		void _loadFromPLY(std::string);
		void _loadFromOBJ(std::string);

		btConvexHullShape* _loadColFromOBJ(std::string);
		btConvexHullShape* _loadColFromPLY(std::string);

		void _init();
	public:
		Mesh(float width, float height);
		Mesh(int x, int y, int width, int height);
		Mesh(std::string = "");
		~Mesh();

		//GLuint* VAO();
		const GLuint* getBuffers() const { return m_buffers; }
		btConvexHullShape* getCollision() const { return m_Collision; }

		const glm::vec3 getRadius() const { return m_radius; }

		const std::vector<glm::vec3>& getPoints() const { return m_Points; }
		const std::vector<glm::vec4>& getColors() const { return m_Colors; }
		const std::vector<glm::vec2>& getUVS() const { return m_UVs; }
		const std::vector<glm::vec3>& getNormals() const { return m_Normals; }
		const std::vector<glm::vec3>& getTangents() const { return m_Tangents; }

		void render();
};
#endif