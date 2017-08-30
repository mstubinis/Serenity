#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "BindableResource.h"
#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <memory>
#include <map>

namespace sf{ class Image; };

class btHeightfieldTerrainShape;
class MeshInstance;
struct ImportedMeshData;
struct BoneInfo;
struct VertexBoneData;
struct aiAnimation;
typedef unsigned int uint;
typedef unsigned short ushort;


const uint NUM_BONES_PER_VERTEX = 4;

class VertexFormat{ public: enum Format{
    Position,UV,Normal,Binormal,Tangent,
    EnumTotal
};};
class VertexFormatAnimated{ public: enum Format{
    Position,UV,Normal,Binormal,Tangent,BoneIDs,BoneWeights,
    EnumTotal
};};
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
    float IDs[NUM_BONES_PER_VERTEX];
    float Weights[NUM_BONES_PER_VERTEX];

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
    ~ImportedMeshData(){ clear(); }

    void clear(){
        vector_clear(file_points); vector_clear(file_uvs); vector_clear(file_normals); vector_clear(file_triangles);
        vector_clear(points); vector_clear(uvs); vector_clear(normals); vector_clear(binormals); vector_clear(tangents); vector_clear(indices);
    }
};
struct MeshVertexData{
    glm::vec3 position;
    float uv;
    GLuint normal;
    GLuint binormal;
    GLuint tangent;
    MeshVertexData(){
    }
    MeshVertexData(const MeshVertexData& copy){
        position = copy.position; uv = copy.uv; normal = copy.normal;
        binormal = copy.binormal; tangent = copy.tangent;
    }
    ~MeshVertexData(){
    }
};
struct MeshVertexDataAnimated: public MeshVertexData{
    glm::vec4 boneIDs;
    glm::vec4 boneWeights;
    MeshVertexDataAnimated():MeshVertexData(){
    }
    MeshVertexDataAnimated(const MeshVertexData& copy){
        position = copy.position; uv = copy.uv; normal = copy.normal;
        binormal = copy.binormal; tangent = copy.tangent;
    }
    MeshVertexDataAnimated(const MeshVertexDataAnimated& copy){
        boneIDs = copy.boneIDs; boneWeights = copy.boneWeights;
        position = copy.position; uv = copy.uv; normal = copy.normal;
        binormal = copy.binormal; tangent = copy.tangent;
    }
    ~MeshVertexDataAnimated(){
    }
};
class AnimationData{
    friend class Mesh;
    private:
        Mesh* m_Mesh;
        double m_TicksPerSecond;
        double m_DurationInTicks;
        std::unordered_map<std::string,aiNodeAnim*> m_KeyframeData;

        void _ReadNodeHeirarchy(const std::string& animationName,float AnimationTime, const aiNode* node,glm::mat4& ParentTransform,std::vector<glm::mat4>& Transforms);
        void _BoneTransform(const std::string& animationName,float TimeInSeconds, std::vector<glm::mat4>& Transforms);
        void _CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node);
        void _CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* node);
        void _CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node);
        uint _FindPosition(float AnimationTime, const aiNodeAnim* node);
        uint _FindRotation(float AnimationTime, const aiNodeAnim* node);
        uint _FindScaling(float AnimationTime, const aiNodeAnim* node);
    public:
        AnimationData(Mesh*,aiAnimation*);
        ~AnimationData();

        float duration();
};
class MeshSkeleton final{
    friend class AnimationData;
    friend class AnimationProcessor;
    friend class Mesh;
    friend struct DefaultMeshBindFunctor;
    friend struct DefaultMeshUnbindFunctor;
    private:
        //animation data
        std::unordered_map<std::string,uint> m_BoneMapping; // maps a bone name to its index
        uint m_NumBones;
        std::vector<BoneInfo> m_BoneInfo;
        glm::mat4 m_GlobalInverseTransform;
        std::unordered_map<std::string,AnimationData*> m_AnimationData;

        std::vector<glm::vec4> m_BoneIDs;
        std::vector<glm::vec4> m_BoneWeights;

    public:
        MeshSkeleton();
        MeshSkeleton(ImportedMeshData&);
        void fill(ImportedMeshData&);
        void clear();
        ~MeshSkeleton();
};
struct DefaultMeshBindFunctor;
struct DefaultMeshUnbindFunctor;
class Mesh final: public BindableResource{
    friend struct DefaultMeshBindFunctor;
    friend struct DefaultMeshUnbindFunctor;
    friend class AnimationData;
    friend class MeshSkeleton;
    friend class AnimationProcessor;
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        Mesh(std::string& name,btHeightfieldTerrainShape*,float threshhold);
        Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshhold);
        Mesh(std::string& name,float width, float height,float threshhold);
        Mesh(std::string& name,float x, float y, float width, float height,float threshhold);
        Mesh(std::string& name,std::string = "",CollisionType = CollisionType::ConvexHull, bool notMemory = true,float threshhold = 0.0005f);
        ~Mesh();

        Collision* getCollision() const;
        std::unordered_map<std::string,AnimationData*>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;
		MeshSkeleton* skeleton();

        void load();
        void unload();

        //Specify wether or not to save the mesh data after loading the data into the OpenGL buffers. By default mesh data is NOT saved. Saving data is useful
        //if you plan on modifying the buffer data manually later on. Mesh data takes up alot of memory space so only save the data if you really need it.
        void saveMeshData(bool);

        void render(GLuint mode = GL_TRIANGLES);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif
