#ifndef ENGINE_MESHLOADER_H
#define ENGINE_MESHLOADER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

enum LoadWhat{
    LOAD_POINTS = 0x01,
    LOAD_UVS = 0x02,
    LOAD_NORMALS = 0x04,
	LOAD_FACES = 0x08
	// = 0x10,
	// = 0x20
};

struct MeshData final{
    std::vector<glm::vec3> file_points;
    std::vector<glm::vec2> file_uvs;
    std::vector<glm::vec3> file_normals;
	std::vector<std::vector<glm::vec3>> file_faces;

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
	std::vector<glm::vec3> binormals;
	std::vector<glm::vec3> tangents;
};
struct ObjectLoadingData final{
    std::vector<glm::vec3> Points;
    std::vector<glm::vec2> UVs;
    std::vector<glm::vec3> Normals;
    std::vector<std::vector<glm::vec3>> Faces;
};
struct Vertex final{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 binormal;
};

typedef unsigned int uint;

namespace Engine{
	namespace Resources{
		namespace MeshLoader{
			void loadObj(MeshData&,std::string file,unsigned char = LOAD_POINTS | LOAD_UVS | LOAD_NORMALS | LOAD_FACES);
			void loadObjFromMemory(MeshData&,std::string file,unsigned char = LOAD_POINTS | LOAD_UVS | LOAD_NORMALS | LOAD_FACES);

			void _generateTriangle(MeshData&,Vertex& v1, Vertex& v2, Vertex& v3);
			void _generateQuad(MeshData&,Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4);
			void _calculateTangent(Vertex&, Vertex&, Vertex&);
		};
	};
};

#endif