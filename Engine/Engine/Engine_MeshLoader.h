#pragma once
#ifndef ENGINE_MESHLOADER_H
#define ENGINE_MESHLOADER_H

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include "Engine_Math.h"

typedef unsigned int uint;
typedef unsigned short ushort;

struct aiScene;
struct aiNode;
struct aiNodeAnim;
class Mesh;
class AnimationData;

const uint NUM_BONES_PER_VEREX = 4;

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
    void clear(){ position = normal = binormal = tangent = glm::vec3(0.0f); uv = glm::vec2(0.0f); }
};
struct Triangle final{Vertex v1;Vertex v2;Vertex v3;};
struct VertexBoneData final{
    float IDs[NUM_BONES_PER_VEREX];
    float Weights[NUM_BONES_PER_VEREX];

    VertexBoneData(){
        memset(&IDs,0,sizeof(IDs));
        memset(&Weights,0,sizeof(Weights));  
    }
    ~VertexBoneData(){
    }
    void AddBoneData(uint BoneID, float Weight){
        uint size = sizeof(IDs) / sizeof(IDs[0]);
        for (uint i = 0; i < size; i++) {
            if (Weights[i] == 0.0) {
                IDs[i] = float(BoneID); Weights[i] = Weight; return;
            } 
        }
    }
};
struct BoneInfo final{
    glm::mat4 BoneOffset;
    glm::mat4 FinalTransform;        
    BoneInfo(){
        BoneOffset = glm::mat4(0.0f);
        FinalTransform = glm::mat4(1.0f);   
    }
};
struct ImportedMeshData final{
    std::map<uint,VertexBoneData> m_Bones;

    std::vector<glm::vec3> file_points;
    std::vector<glm::vec2> file_uvs;
    std::vector<glm::vec3> file_normals;
    std::vector<Triangle>  file_triangles;

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;
    std::vector<glm::vec3> tangents;
    std::vector<ushort> indices;

    ImportedMeshData(){ clear(); }
    ~ImportedMeshData(){}

    void clear(){ 
        file_points.clear(); file_uvs.clear(); file_normals.clear(); file_triangles.clear();
        points.clear(); uvs.clear(); normals.clear(); binormals.clear(); tangents.clear(); indices.clear();
        file_points.shrink_to_fit(); file_uvs.shrink_to_fit(); file_normals.shrink_to_fit(); file_triangles.shrink_to_fit();
        points.shrink_to_fit(); uvs.shrink_to_fit(); normals.shrink_to_fit(); binormals.shrink_to_fit(); tangents.shrink_to_fit(); indices.shrink_to_fit();
    }
};

namespace Engine{
    namespace Resources{
        namespace MeshLoader{
            void loadObjFromMemory(ImportedMeshData&,std::string file,unsigned char = LOAD_POINTS | LOAD_UVS | LOAD_NORMALS | LOAD_FACES | LOAD_TBN);
            void load(Mesh*,ImportedMeshData&,std::string file);
            namespace Detail{

                class MeshLoadingManagement{
                    friend class ::Mesh;
                    friend class ::AnimationData;
                    public:
                        static void _load(Mesh*,ImportedMeshData&,std::string file);
                        static void _processNode(Mesh*,ImportedMeshData&,aiNode* node, const aiScene* scene);
                        static void _calculateGramSchmidt(std::vector<glm::vec3>& points,std::vector<glm::vec3>& normals,std::vector<glm::vec3>& binormals,std::vector<glm::vec3>& tangents);
                        static void _calculateTBN(ImportedMeshData&);
                        static bool _getSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& out_vertices,std::vector<glm::vec2>& out_uvs,std::vector<glm::vec3>& out_normals,ushort& result,float threshold);
						/*
						static void _indexVBO(ImportedMeshData&,std::vector<ushort> & out_indices,std::vector<glm::vec3>& out_pos, std::vector<glm::vec2>& out_uvs, 
							std::vector<glm::vec3>& out_norm, 
							std::vector<glm::vec3>& out_binorm,
							std::vector<glm::vec3>& out_tangents,
						float threshold);
						*/

						static void _indexVBO(ImportedMeshData&,std::vector<ushort> & out_indices,std::vector<glm::vec3>& out_pos, std::vector<float>& out_uvs, 
							std::vector<std::uint32_t>& out_norm, 
							std::vector<std::uint32_t>& out_binorm,
							std::vector<std::uint32_t>& out_tangents,
						float threshold);

                        static void _loadDataIntoTriangles(ImportedMeshData&,std::vector<uint>& _pi, std::vector<uint>& _ui,std::vector<uint>& _ni,unsigned char _flags);
                };

                namespace _OBJ{
                    void _loadObjDataFromLine(std::string& line,ImportedMeshData&, std::vector<uint>& vertexIndices, std::vector<uint>& uvIndices, std::vector<uint>& normalIndices, const char flags);
                    
                };
                namespace _3DS{
                };
            };
        };
    };
};

#endif