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
    friend struct DefaultMeshBindFunctor;
    friend struct DefaultMeshUnbindFunctor;
    friend class AnimationData;
    friend class MeshSkeleton;
    friend class AnimationProcessor;
    friend class Engine::Resources::MeshLoader::Detail::MeshLoadingManagement;
    private:
        static DefaultMeshBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultMeshUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        std::vector<GLuint> m_buffers;
        Collision* m_Collision;

        MeshSkeleton* m_Skeleton;

        const aiScene* m_aiScene;
        Assimp::Importer m_Importer;
        std::string m_File;

        glm::vec3 m_radiusBox;
        float m_radius;
        float m_threshold;
        bool m_SaveMeshData;
        CollisionType m_Type;

        std::vector<MeshVertexData> m_Vertices;
        std::vector<ushort> m_Indices;

        void _loadData(ImportedMeshData&,float threshhold);
        void _clearData();
        void _loadFromFile(std::string,CollisionType,float threshold);
        void _loadFromOBJMemory(std::string,CollisionType,float threshold);
        void _calculateMeshRadius();
    public:
        Mesh(std::string& name,btHeightfieldTerrainShape*,float threshhold);
        Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshhold);
        Mesh(std::string& name,float width, float height,float threshhold);
        Mesh(std::string& name,float x, float y, float width, float height,float threshhold);
        Mesh(std::string& name,std::string = "",CollisionType = CollisionType::ConvexHull, bool notMemory = true,float threshhold = 0.0005f);
        ~Mesh();

        void initRenderingContext();
        void cleanupRenderingContext();

        Collision* getCollision() const { return m_Collision; }
        std::unordered_map<std::string,AnimationData*>& animationData(){ return m_Skeleton->m_AnimationData; }
        const glm::vec3& getRadiusBox() const { return m_radiusBox; }
        const float getRadius() const { return m_radius; }

        void load();
        void unload();

        //Specify wether or not to save the mesh data after loading the data into the OpenGL buffers. By default mesh data is NOT saved. Saving data is useful
        //if you plan on modifying the buffer data manually later on. Mesh data takes up alot of memory space so only save the data if you really need it.
        void saveMeshData(bool);

        void render(GLuint mode = GL_TRIANGLES);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif
