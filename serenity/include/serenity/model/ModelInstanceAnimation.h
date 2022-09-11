#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

struct MeshNodeData;
namespace Engine::priv {
    class  ModelInstanceAnimationContainer;
    class  AnimationData;
    class  MeshSkeleton;
};

#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <string>
#include <vector>
#include <array>
#include <serenity/resources/mesh/animation/AnimationIncludes.h>

namespace Engine::priv {
    class ModelInstanceAnimation final {
        friend class  ModelInstanceAnimationContainer;
        private:
            struct Snapshot {
                std::vector<AnimationChannel> snapshot;
                float blendDuration;
            };
        private:
            std::vector<std::array<uint16_t, 3>>  m_CurrentKeyframes; //[0] = pos, [1] = rot, [2] = scl
            AnimationData*                        m_AnimationData    = nullptr;
            std::optional<Snapshot>               m_Snapshot;
            float                                 m_CurrentTime      = 0.0f;
            float                                 m_StartTime        = 0.0f;
            float                                 m_EndTime          = 0.0f;
            uint16_t                              m_CurrentLoops     = 0;
            uint16_t                              m_RequestedLoops   = 1;

            ModelInstanceAnimation() = delete;
        public:
            ModelInstanceAnimation(MeshNodeData&, AnimationData&, MeshSkeleton&, float startTime, float endTime, uint16_t requestedLoops);
            ModelInstanceAnimation(std::vector<AnimationChannel>&& snapshot, float blendDuration, MeshNodeData&, AnimationData&, MeshSkeleton&, float startTime, float endTime, uint16_t requestedLoops);

            ModelInstanceAnimation(const ModelInstanceAnimation&)                = delete;
            ModelInstanceAnimation& operator=(const ModelInstanceAnimation&)     = delete;
            ModelInstanceAnimation(ModelInstanceAnimation&&) noexcept;
            ModelInstanceAnimation& operator=(ModelInstanceAnimation&&) noexcept;

            void update(const float dt, std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& tempTransforms, std::vector<NodeSnapshot>& nodeTransforms, size_t numBones, MeshNodeData*, MeshSkeleton*);
    };
    class ModelInstanceAnimationContainer final {
        private:
            std::vector<ModelInstanceAnimation>  m_Animation_Instances;
            std::vector<glm::mat4>               m_NodeLocalTransforms;
            std::vector<NodeSnapshot>            m_NodeSnapshots;
            std::vector<glm::mat4>               m_BoneVertexTransforms;
            MeshSkeleton*                        m_Skeleton             = nullptr;
            MeshNodeData*                        m_NodeData             = nullptr;
            size_t                               m_NumBones             = 0;

            bool internal_emplace_animation(Handle mesh, const int32_t animationIndex, float startTime, float endTime, uint16_t requestedLoops, float blendTime) noexcept;
        public:
            ModelInstanceAnimationContainer(MeshSkeleton*, MeshNodeData*);

            bool emplace_animation(Handle mesh, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops, float blendTime);
            bool emplace_animation(Handle mesh, const int32_t animationIndex, float startTime, float endTime, uint16_t requestedLoops, float blendTime);

            void setMesh(Handle mesh);
            void resetNodeWorldTransformsToRestPose();

            [[nodiscard]] inline const std::vector<glm::mat4>& getBoneVertexTransforms() const noexcept { return m_BoneVertexTransforms; }
            [[nodiscard]] inline const std::vector<glm::mat4>& getNodeLocalTransforms() const noexcept { return m_NodeLocalTransforms; }
            [[nodiscard]] inline const std::vector<NodeSnapshot>& getNodeSnapshots() const noexcept { return m_NodeSnapshots; }
            [[nodiscard]] const glm::mat4& getGlobalInverseTransform() const noexcept;
            [[nodiscard]] inline const MeshNodeData& getNodeData() const noexcept { return *m_NodeData; }
            [[nodiscard]] inline const MeshSkeleton& getSkeleton() const noexcept { return *m_Skeleton; }
            [[nodiscard]] inline size_t getNumBones() const noexcept { return m_NumBones; }

            [[nodiscard]] const glm::mat4& getBoneLocalTransform(size_t boneIndex) const noexcept;

            [[nodiscard]] inline size_t size() const noexcept { return m_Animation_Instances.size(); }
            [[nodiscard]] inline bool empty() const noexcept { return size() == 0; }
            void clear(bool clearCachedTransforms = true);
            void update(const float dt);
    };
};

#endif