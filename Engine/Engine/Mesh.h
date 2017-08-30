#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "BindableResource.h"
#include "Engine_MeshLoader.h"
#include "Engine_Physics.h"
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <memory>

namespace sf{ class Image; };

class btHeightfieldTerrainShape;
class MeshInstance;
struct ImportedMeshData;
struct BoneInfo;
struct VertexBoneData;
struct aiAnimation;
typedef unsigned int uint;
typedef unsigned short ushort;

class VertexFormat{ public: enum Format{
    Position,UV,Normal,Binormal,Tangent,
    EnumTotal
};};
class VertexFormatAnimated{ public: enum Format{
    Position,UV,Normal,Binormal,Tangent,BoneIDs,BoneWeights,
    EnumTotal
};};
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
    friend class Engine::Resources::MeshLoader::Detail::MeshLoadingManagement;
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
    friend class Engine::Resources::MeshLoader::Detail::MeshLoadingManagement;
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

        void load();
        void unload();

        //Specify wether or not to save the mesh data after loading the data into the OpenGL buffers. By default mesh data is NOT saved. Saving data is useful
        //if you plan on modifying the buffer data manually later on. Mesh data takes up alot of memory space so only save the data if you really need it.
        void saveMeshData(bool);

        void render(GLuint mode = GL_TRIANGLES);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif
