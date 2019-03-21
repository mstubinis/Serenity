#pragma once
#ifndef ENGINE_MESH_SKELETON_INCLUDE_GUARD
#define ENGINE_MESH_SKELETON_INCLUDE_GUARD

#include <glm/glm.hpp>
#include <core/engine/Engine_Utils.h>
#include <unordered_map>
#include <assimp/scene.h>

class  Mesh;

namespace Engine {
    namespace epriv {
        struct DefaultMeshBindFunctor;
        struct DefaultMeshUnbindFunctor;
        struct MeshImportedData;
        class  AnimationData;
        struct BoneInfo final {
            glm::mat4   BoneOffset;
            glm::mat4   FinalTransform;
            BoneInfo() {
                BoneOffset     = glm::mat4(0.0f);
                FinalTransform = glm::mat4(1.0f);
            }
        };
        struct BoneNode final {
            std::string Name;
            BoneNode* Parent;
            std::vector<BoneNode*> Children;
            glm::mat4 Transform;
            BoneNode() {
                Name      = "";
                Parent    = nullptr;
                Transform = glm::mat4(1.0f);
            }
        };
        class MeshSkeleton final {
            friend class  Mesh;
            friend class  Engine::epriv::AnimationData;
            friend struct Engine::epriv::DefaultMeshBindFunctor;
            friend struct Engine::epriv::DefaultMeshUnbindFunctor;
            private:
                BoneNode*                                       m_RootNode;
                uint                                            m_NumBones;
                std::vector<BoneInfo>                           m_BoneInfo;
                std::vector<glm::vec4>                          m_BoneIDs, m_BoneWeights;
                std::unordered_map<std::string, uint>           m_BoneMapping; // maps a bone name to its index
                std::unordered_map<std::string, AnimationData>  m_AnimationData;
                glm::mat4                                       m_GlobalInverseTransform;
                void fill(const MeshImportedData& data);
                void populateCleanupMap(BoneNode* node, std::unordered_map<std::string, BoneNode*>& _map);
                void cleanup();
                void clear();
            public:
                MeshSkeleton();
                MeshSkeleton(const MeshImportedData& data);
                ~MeshSkeleton();
                uint numBones() { return m_NumBones; }
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
        class AnimationData final {
            friend class ::Engine::epriv::MeshSkeleton;
            friend class ::Mesh;
            private:
                MeshSkeleton* m_MeshSkeleton;
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
                AnimationData(const Engine::epriv::MeshSkeleton&, const aiAnimation&);
                const AnimationData& operator=(const AnimationData&) = delete;// non copyable
                AnimationData(const AnimationData&) = delete;                 // non construction-copyable
                AnimationData(AnimationData&&) = default;
                ~AnimationData();
                float duration();
        };
    };
};

#endif