#pragma once
#ifndef ENGINE_MESH_SKELETON_INCLUDE_GUARD
#define ENGINE_MESH_SKELETON_INCLUDE_GUARD

#include <unordered_map>
#include <assimp/scene.h>
#include <glm/mat4x4.hpp>

class  Mesh;
namespace Engine {
    namespace priv {
        struct InternalMeshPublicInterface;
        struct DefaultMeshBindFunctor;
        struct DefaultMeshUnbindFunctor;
        struct MeshImportedData;
        class  AnimationData;
        class  MeshLoader;
        struct BoneInfo final {
            glm::mat4   BoneOffset;
            glm::mat4   FinalTransform;
            BoneInfo() {
                BoneOffset     = glm::mat4(0.0f);
                FinalTransform = glm::mat4(1.0f);
            }
        };
        struct BoneNode final {
            std::string             Name;
            BoneNode*               Parent;
            std::vector<BoneNode*>  Children;
            glm::mat4               Transform;
            BoneNode() {
                Name      = "";
                Parent    = nullptr;
                Transform = glm::mat4(1.0f);
            }
        };
        class MeshSkeleton final {
            friend class  Mesh;
            friend class  Engine::priv::MeshLoader;
            friend class  Engine::priv::AnimationData;
            friend struct Engine::priv::DefaultMeshBindFunctor;
            friend struct Engine::priv::DefaultMeshUnbindFunctor;
            friend struct Engine::priv::InternalMeshPublicInterface;
            private:
                BoneNode*                                       m_RootNode;
                unsigned int                                    m_NumBones;
                std::vector<BoneInfo>                           m_BoneInfo;
                std::vector<glm::vec4>                          m_BoneIDs, m_BoneWeights;
                std::unordered_map<std::string, unsigned int>   m_BoneMapping; // maps a bone name to its index
                std::unordered_map<std::string, AnimationData>  m_AnimationData;
                glm::mat4                                       m_GlobalInverseTransform;

                void fill(const Engine::priv::MeshImportedData& data);
                void populateCleanupMap(BoneNode* node, std::unordered_map<std::string, BoneNode*>& _map);
                void cleanup();
                void clear();
            public:
                MeshSkeleton();
                MeshSkeleton(const Engine::priv::MeshImportedData& data);
                ~MeshSkeleton();

                MeshSkeleton(const MeshSkeleton&)                      = delete;
                MeshSkeleton& operator=(const MeshSkeleton&)           = delete;
                MeshSkeleton(MeshSkeleton&& other) noexcept            = delete;
                MeshSkeleton& operator=(MeshSkeleton&& other) noexcept = delete;

                const unsigned int& numBones();
        };
        struct Vector3Key final {
            glm::vec3  value;
            double     time;
            Vector3Key(const double _time, const glm::vec3& _value) {
                value = _value;
                time = _time; 
            }
        };
        struct QuatKey final {
            aiQuaternion  value;
            double        time;
            QuatKey(const double _time, const aiQuaternion& _value) {
                value = _value;
                time = _time; 
            }
        };
        struct AnimationChannel final {
            std::vector<Vector3Key>  PositionKeys;
            std::vector<QuatKey>     RotationKeys;
            std::vector<Vector3Key>  ScalingKeys;
        };
        class AnimationData final {
            friend class Engine::priv::MeshSkeleton;
            friend class Mesh;
            private:
                MeshSkeleton*                                      m_MeshSkeleton;
                double                                             m_TicksPerSecond;
                double                                             m_DurationInTicks;
                std::unordered_map<std::string, AnimationChannel>  m_KeyframeData;

                void ReadNodeHeirarchy(const std::string& animationName, float time, const BoneNode* node, glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
                void BoneTransform(const std::string& animationName, float TimeInSeconds, std::vector<glm::mat4>& Transforms);
                void CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
                void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node);
                void CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
                const size_t FindPosition(const float AnimationTime, const AnimationChannel& node);
                const size_t FindRotation(const float AnimationTime, const AnimationChannel& node);
                const size_t FindScaling(const float AnimationTime, const AnimationChannel& node);
            public:
                AnimationData() = delete;
                AnimationData(const Engine::priv::MeshSkeleton&, const aiAnimation&);

                AnimationData(const AnimationData&)                      = delete;
                AnimationData& operator=(const AnimationData&)           = delete;
                AnimationData(AnimationData&&) noexcept                  = delete;
                AnimationData& operator=(AnimationData&& other) noexcept = delete;
                ~AnimationData();

                const float duration();
        };
    };
};

#endif