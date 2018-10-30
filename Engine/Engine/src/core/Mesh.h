#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "core/engine/BindableResource.h"
#include "core/engine/Engine_EventObject.h"
#include "core/engine/Engine_Physics.h"

#include <map>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <assimp/scene.h>

typedef unsigned short ushort;

namespace sf{ class Image; };

const uint NUM_BONES_PER_VERTEX = 4;
const uint NUM_MAX_INSTANCES = 65536;

class MeshInstance;
class Mesh;

struct MeshDrawMode {enum Mode {
    Triangles = GL_TRIANGLES,
    Quads = GL_QUADS,
    Points = GL_POINTS,
    Lines = GL_LINES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN,
    QuadStrip = GL_QUAD_STRIP,
};};

namespace Engine{
    namespace epriv{
        class MeshLoader;
        class MeshSkeleton;
        class CollisionFactory;
        struct DefaultMeshBindFunctor;
        struct DefaultMeshUnbindFunctor;
        struct Vertex final{
            glm::vec3 position;
            glm::vec2 uv;
            glm::vec3 normal;
            glm::vec3 binormal;
            glm::vec3 tangent;
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
            VertexBoneData() {
                for (uint i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                    IDs[i] = 0.0f; Weights[i] = 0.0f;
                }
            }
            void AddBoneData(uint BoneID, float Weight){
                for (uint i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                    if (Weights[i] == 0.0f) {
                        IDs[i] = float(BoneID);
                        Weights[i] = Weight;
                        return;
                    } 
                }
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
        struct Vector3Key final {
            glm::vec3 value;
            double time;
            Vector3Key(double _time, glm::vec3 _value) { value = _value; time = _time; }
        };
        struct QuatKey final {
            aiQuaternion value;
            double time;
            QuatKey(double _time, aiQuaternion _value) { value = _value; time = _time; }
        };
        struct AnimationChannel final {
            std::vector<Vector3Key> PositionKeys;
            std::vector<QuatKey>    RotationKeys;
            std::vector<Vector3Key> ScalingKeys;
        };
        struct BoneNode;
        class AnimationData final{
            friend class ::Mesh;
            private:
                Mesh* m_Mesh;
                double m_TicksPerSecond;
                double m_DurationInTicks;
                std::unordered_map<std::string, AnimationChannel> m_KeyframeData;

                void ReadNodeHeirarchy(const std::string& animationName, float time, const BoneNode* node, glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
                void BoneTransform(const std::string& animationName, float TimeInSeconds, std::vector<glm::mat4>& Transforms);
                void CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
                void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node);
                void CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
                uint FindPosition(float AnimationTime, const AnimationChannel& node);
                uint FindRotation(float AnimationTime, const AnimationChannel& node);
                uint FindScaling(float AnimationTime, const AnimationChannel& node);
            public:
                AnimationData(const Mesh&, const aiAnimation&);
                const AnimationData& operator=(const AnimationData&) = delete;// non copyable
                AnimationData(const AnimationData&) = delete;                 // non construction-copyable
                AnimationData(AnimationData&&) = default;
                ~AnimationData();
                float duration();
        };
        struct InternalMeshPublicInterface final {
            static void LoadCPU(Mesh&);
            static void LoadGPU(Mesh&);
            static void UnloadCPU(Mesh&);
            static void UnloadGPU(Mesh&);
            //static void UpdateInstance(Mesh&, uint _id, glm::mat4 _modelMatrix);
            //static void UpdateInstances(Mesh&, std::vector<glm::mat4>& _modelMatrices);
            static bool SupportsInstancing();
            static btCollisionShape* BuildCollision(Mesh*, CollisionType::Type);
        };
    };
};
class Mesh final: public BindableResource, public EventObserver{
    friend struct Engine::epriv::DefaultMeshBindFunctor;
    friend struct Engine::epriv::DefaultMeshUnbindFunctor;
    friend class  Engine::epriv::AnimationData;
    friend class  Engine::epriv::MeshSkeleton;
    friend struct Engine::epriv::InternalMeshPublicInterface;
    friend class  Engine::epriv::MeshLoader;
    friend class  Engine::epriv::CollisionFactory;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        static Mesh *FontPlane, *Plane, *Cube; //loaded in renderer

        Mesh(std::string name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshhold);
        Mesh(std::string name,float width, float height,float threshhold);
        Mesh(std::string fileOrData, bool notMemory = true,float threshhold = 0.0005f,bool loadNow = true);
        ~Mesh();

        std::unordered_map<std::string, Engine::epriv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        void modifyPoints(std::vector<glm::vec3>& modifiedPoints);
        void modifyUVs(std::vector<glm::vec2>& modifiedUVs);
        void modifyPointsAndUVs(std::vector<glm::vec3>& modifiedPoints, std::vector<glm::vec2>& modifiedUVs);
        void modifyIndices(std::vector<ushort>& modifiedIndices);

        void render(bool instancing = true, MeshDrawMode::Mode = MeshDrawMode::Triangles);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif