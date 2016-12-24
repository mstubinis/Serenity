#pragma once
#ifndef ENGINE_MESHLOADER_H
#define ENGINE_MESHLOADER_H

#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>

enum LoadWhat{
    LOAD_POINTS = 0x01,
    LOAD_UVS = 0x02,
    LOAD_NORMALS = 0x04,
    LOAD_FACES = 0x08,
    LOAD_TBN = 0x10
    // = 0x20
};

struct Vertex final{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 binormal;
    void clear(){ position = normal = binormal = tangent = glm::vec3(0); uv = glm::vec2(0); }
    bool operator<(const Vertex that) const{ return memcmp((void*)this, (void*)&that, sizeof(Vertex))>0; };
};
struct Triangle final{
    Vertex v1;
    Vertex v2;
    Vertex v3;
};
struct MeshData final{
    std::vector<glm::vec3> file_points;
    std::vector<glm::vec2> file_uvs;
    std::vector<glm::vec3> file_normals;
    std::vector<Triangle> file_triangles;

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;
    std::vector<glm::vec3> tangents;
};

typedef unsigned int uint;
typedef unsigned short ushort;

namespace Engine{
    namespace Resources{
        namespace MeshLoader{
            void loadObj(MeshData&,std::string file,unsigned char = LOAD_POINTS | LOAD_UVS | LOAD_NORMALS | LOAD_FACES | LOAD_TBN);
            void loadObjFromMemory(MeshData&,std::string file,unsigned char = LOAD_POINTS | LOAD_UVS | LOAD_NORMALS | LOAD_FACES | LOAD_TBN);

            void _calculateGramSchmidt(std::vector<glm::vec3>& points,std::vector<glm::vec3>& normals,std::vector<glm::vec3>& binormals,std::vector<glm::vec3>& tangents);

            void _loadObjDataFromLine(std::string& line, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<uint>& vertexIndices, std::vector<uint>& uvIndices, std::vector<uint>& normalIndices, const char flags);
            void _loadObjDataIntoTriangles(MeshData& data,std::vector<glm::vec3>& _p, std::vector<glm::vec2>& _u,std::vector<glm::vec3>& _n,std::vector<uint>& _pi, std::vector<uint>& _ui,std::vector<uint>& _ni,unsigned char _flags);

            void _calculateTBN(MeshData&);

            bool _getSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& out_vertices,std::vector<glm::vec2>& out_uvs,std::vector<glm::vec3>& out_normals,ushort& result,float threshold);
            void _indexVBO(MeshData& data,std::vector<ushort> & out_indices,std::vector<glm::vec3>& out_pos, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_norm, std::vector<glm::vec3>& out_binorm,std::vector<glm::vec3>& out_tangents,float threshold);
        };
    };
};

#endif