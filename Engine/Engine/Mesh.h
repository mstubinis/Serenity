#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "BindableResource.h"
#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include "Components.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <map>

#include <glm/glm.hpp>

namespace sf{ class Image; };

const uint NUM_BONES_PER_VERTEX = 4;

struct aiAnimation;
struct DefaultMeshBindFunctor;
struct DefaultMeshUnbindFunctor;
class btHeightfieldTerrainShape;
class MeshInstance;
class InternalMeshPublicInterface;
class Mesh;
class MeshSkeleton;

typedef unsigned int uint;
typedef unsigned short ushort;

namespace Engine{
    namespace epriv{
        class VertexFormat final{ public: enum Format{
            Position,UV,Normal,Binormal,Tangent,
            EnumTotal
        };};
        class VertexFormatAnimated final{ public: enum Format{
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
        struct Triangle final{
			Vertex v1;
			Vertex v2;
			Vertex v3;
			Triangle(){}
			Triangle(Vertex& _v1, Vertex& _v2, Vertex& _v3){ v1 = _v1; v2 = _v2; v3 = _v3; }
			~Triangle(){}
		};
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
                for (uint i = 0; i < size; ++i) {
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
            void clear(){
                vector_clear(file_points); vector_clear(file_uvs); vector_clear(file_normals); vector_clear(file_triangles);
                vector_clear(points); vector_clear(uvs); vector_clear(normals); vector_clear(binormals); vector_clear(tangents); vector_clear(indices);
            }
            ImportedMeshData(){ }
            ~ImportedMeshData(){ clear(); }
        };
        class AnimationData final: private Engine::epriv::noncopyable{
            friend class ::Mesh;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                AnimationData(Mesh*,aiAnimation*);
                ~AnimationData();
                float duration();
        };
        class MeshSkeleton final: private Engine::epriv::noncopyable{
            friend class ::Engine::epriv::AnimationData;
            friend class ::Mesh;
            friend struct ::DefaultMeshBindFunctor;
            friend struct ::DefaultMeshUnbindFunctor;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                MeshSkeleton();
                MeshSkeleton(Engine::epriv::ImportedMeshData&);
                ~MeshSkeleton();

                void fill(Engine::epriv::ImportedMeshData&);
                void clear();
                uint numBones();
        };
    };
};
class InternalMeshPublicInterface final{
    public:
        static void LoadCPU(Mesh*);
        static void LoadGPU(Mesh*);
        static void UnloadCPU(Mesh*);
        static void UnloadGPU(Mesh*);
};

class Mesh final: public BindableResource{
    friend struct ::DefaultMeshBindFunctor;
    friend struct ::DefaultMeshUnbindFunctor;
    friend class ::Engine::epriv::AnimationData;
    friend class ::Engine::epriv::MeshSkeleton;
    friend class ::InternalMeshPublicInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        //loaded in renderer
        static Mesh* FontPlane;
        static Mesh* Plane;
        static Mesh* Cube;

        Mesh(std::string name,btHeightfieldTerrainShape*,float threshhold);
        Mesh(std::string name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshhold);
        Mesh(std::string name,float width, float height,float threshhold);
        Mesh(std::string name,float x, float y, float width, float height,float threshhold);
        Mesh(std::string fileOrData,CollisionType::Type = CollisionType::ConvexHull, bool notMemory = true,float threshhold = 0.0005f,bool loadImmediately = true);
        ~Mesh();


        Collision* getCollision() const;
        std::unordered_map<std::string, Engine::epriv::AnimationData*>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;

        void load();
        void unload();

        void modifyPoints(std::vector<glm::vec3>& modifiedPoints);
        void modifyUVs(std::vector<glm::vec2>& modifiedUVs);
        void modifyPointsAndUVs(std::vector<glm::vec3>& modifiedPoints, std::vector<glm::vec2>& modifiedUVs);

        //Specify wether or not to save the mesh data after loading the data into the OpenGL buffers. By default mesh data is saved.
        void saveMeshData(bool);

        void render(GLuint mode = GL_TRIANGLES);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};


#endif
