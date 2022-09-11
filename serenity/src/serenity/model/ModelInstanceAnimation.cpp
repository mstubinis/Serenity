
#include <serenity/model/ModelInstanceAnimation.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/math/MathCompression.h>
#include <glm/gtx/matrix_decompose.hpp>

using namespace Engine::priv;

#pragma region ModelInstanceAnimation

ModelInstanceAnimation::ModelInstanceAnimation(MeshNodeData& nodeData, AnimationData& animData, MeshSkeleton& skeleton, float startTime, float endTime, uint16_t requestedLoops)
    : m_AnimationData  { std::addressof(animData) }
    , m_StartTime      { startTime }
    , m_EndTime        { (endTime <= 0.0f) ? animData.duration() : endTime }
    , m_RequestedLoops { requestedLoops }
{
    m_CurrentKeyframes.resize(nodeData.m_Nodes.size(), std::array<uint16_t, 3>{0, 0, 0});
}
ModelInstanceAnimation::ModelInstanceAnimation(std::vector<AnimationChannel>&& snapshot, float blendDuration, MeshNodeData& nodeData, AnimationData& animData, MeshSkeleton&, float startTime, float endTime, uint16_t requestedLoops)
    : m_AnimationData{ std::addressof(animData) }
    , m_StartTime{ startTime }
    , m_EndTime{ (endTime <= 0.0f) ? animData.duration() : endTime }
    , m_RequestedLoops{ requestedLoops }
{
    m_CurrentKeyframes.resize(nodeData.m_Nodes.size(), std::array<uint16_t, 3>{0, 0, 0});
    m_Snapshot.emplace(std::move(snapshot), blendDuration);
}


ModelInstanceAnimation::ModelInstanceAnimation(ModelInstanceAnimation&& other) noexcept
    : m_CurrentKeyframes{ std::move(other.m_CurrentKeyframes) }
    , m_Snapshot        { std::move(other.m_Snapshot) }
    , m_AnimationData   { std::exchange(other.m_AnimationData, nullptr) }
    , m_CurrentTime     { std::move(other.m_CurrentTime) }
    , m_StartTime       { std::move(other.m_StartTime) }
    , m_EndTime         { std::move(other.m_EndTime) }
    , m_CurrentLoops    { std::move(other.m_CurrentLoops) }
    , m_RequestedLoops  { std::move(other.m_RequestedLoops) }
{}
ModelInstanceAnimation& ModelInstanceAnimation::operator=(ModelInstanceAnimation&& other) noexcept {
    if (this != &other) {
        m_CurrentKeyframes = std::move(other.m_CurrentKeyframes);
        m_Snapshot         = std::move(other.m_Snapshot);
        m_AnimationData    = std::exchange(other.m_AnimationData, nullptr);
        m_CurrentTime      = std::move(other.m_CurrentTime);
        m_StartTime        = std::move(other.m_StartTime);
        m_EndTime          = std::move(other.m_EndTime);
        m_CurrentLoops     = std::move(other.m_CurrentLoops);
        m_RequestedLoops   = std::move(other.m_RequestedLoops);
    }
    return *this;
}
void ModelInstanceAnimation::update(const float dt, std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& nodeWorldTransforms, std::vector<NodeSnapshot>& nodeTransforms, size_t numBones, MeshNodeData* nodeData, MeshSkeleton* skeleton) {
    m_CurrentTime  += dt;
    if (m_Snapshot.has_value()) {
        m_AnimationData->ComputeTransformsSnapshot(
            m_CurrentTime,
            m_Snapshot.value().snapshot,
            transforms,
            *nodeData,
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
            skeleton->m_GlobalInverseTransform,
#endif
            skeleton->m_BoneOffsets,
            nodeWorldTransforms,
            nodeTransforms,
            m_Snapshot.value().blendDuration
        );
        if (m_CurrentTime >= m_Snapshot.value().blendDuration) {
            m_CurrentTime = 0.0f;
            m_Snapshot.reset();
        }
    } else {
        m_AnimationData->ComputeTransforms(
            m_CurrentTime,
            transforms,
            *nodeData,
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
            skeleton->m_GlobalInverseTransform,
#endif
            skeleton->m_BoneOffsets,
            nodeWorldTransforms,
            nodeTransforms,
            m_CurrentKeyframes
        );
        if (m_CurrentTime >= m_EndTime) {
            m_CurrentTime = 0.0f;
            ++m_CurrentLoops;
        }
    }
}
#pragma endregion

#pragma region ModelInstanceAnimationContainer

ModelInstanceAnimationContainer::ModelInstanceAnimationContainer(MeshSkeleton* skeleton, MeshNodeData* nodeData)
    : m_Skeleton{ skeleton }
    , m_NodeData{ nodeData }
    , m_NumBones{ skeleton ? skeleton->numBones() : 0 }
{
    resetNodeWorldTransformsToRestPose();
}
void ModelInstanceAnimationContainer::setMesh(Handle meshHandle) {
    Mesh* mesh             = meshHandle.get<Mesh>();
    auto& nodeData         = mesh->m_CPUData.m_NodesData;
    MeshSkeleton* skeleton = mesh->getSkeleton();

    m_Skeleton = skeleton;
    m_NodeData = &nodeData;
    m_NumBones = skeleton ? skeleton->numBones() : 0;

    resetNodeWorldTransformsToRestPose();
}
bool ModelInstanceAnimationContainer::internal_emplace_animation(Handle meshHandle, const int32_t animationIndex, float startTime, float endTime, uint16_t requestedLoops, float blendTime) noexcept {
    if (animationIndex != -1) {
        if (empty()) {
            m_NodeSnapshots.resize(m_NodeData->m_Nodes.size());
            for (size_t i = 0; i < m_NodeSnapshots.capacity(); ++i) {
                glm::quat rotation;
                Engine::Math::decompose(m_NodeData->m_Nodes[i].Transform, m_NodeSnapshots[i].position, rotation, m_NodeSnapshots[i].scale);
                m_NodeSnapshots[i].rotation = rotation;
            }
        }
        if (blendTime == 0.0f) {
            m_Animation_Instances.emplace_back(*m_NodeData, m_Skeleton->m_AnimationData[animationIndex], *m_Skeleton, startTime, endTime, requestedLoops);
        } else {
            clear(false);
            std::vector<AnimationChannel> snapshots;
            snapshots.reserve(m_NodeSnapshots.size());
            for (const auto& nodeSnapshot : m_NodeSnapshots) {
                AnimationChannel c;
                glm::quat rotation = nodeSnapshot.rotation;
                c.PositionKeys.emplace_back(0.0f, nodeSnapshot.position);
                c.RotationKeys.emplace_back(0.0f, glm::normalize(rotation));
                c.ScalingKeys.emplace_back(0.0f, nodeSnapshot.scale);
                snapshots.emplace_back(std::move(c));
            }
            m_Animation_Instances.emplace_back(std::move(snapshots), blendTime, *m_NodeData, m_Skeleton->m_AnimationData[animationIndex], *m_Skeleton, startTime, endTime, requestedLoops);
        }
        return true;
    }
    return false;
}

bool ModelInstanceAnimationContainer::emplace_animation(Handle meshHandle, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops, float blendTime) {
    auto& mesh      = *meshHandle.get<Mesh>();
    auto& skeleton  = *mesh.getSkeleton();
    return internal_emplace_animation(meshHandle, skeleton.getAnimationIndex(animationName), startTime, endTime, requestedLoops, blendTime);
}
bool ModelInstanceAnimationContainer::emplace_animation(Handle meshHandle, const int32_t animationIndex, float startTime, float endTime, uint16_t requestedLoops, float blendTime) {
    return internal_emplace_animation(meshHandle, animationIndex, startTime, endTime, requestedLoops, blendTime);
}
void ModelInstanceAnimationContainer::clear(bool clearCachedTransforms) {
    m_Animation_Instances.clear();
    if (clearCachedTransforms) {
        m_NodeLocalTransforms.clear();
        m_NodeLocalTransforms.shrink_to_fit();
        m_NodeSnapshots.clear();
        m_NodeSnapshots.shrink_to_fit();
    }
}
void ModelInstanceAnimationContainer::resetNodeWorldTransformsToRestPose() {
    if (m_BoneVertexTransforms.size() != m_NumBones) {
        m_BoneVertexTransforms.resize(m_NumBones, glm::mat4{ 1.0f });
    }
    if (m_NodeLocalTransforms.size() != m_NodeData->m_Nodes.size()) {
        m_NodeLocalTransforms.resize(m_NodeData->m_Nodes.size());
    }
    if (!m_NodeLocalTransforms.empty()) {
        m_NodeLocalTransforms[0] = m_NodeData->m_Nodes[0].Transform;
        size_t BoneIndex = 0;
        for (uint32_t nodeIdx = 1; nodeIdx != m_NodeData->m_Nodes.size(); ++nodeIdx) {
            const auto parentIdx = m_NodeData->m_NodeHeirarchy[nodeIdx];
            const auto& currNode = m_NodeData->m_Nodes[nodeIdx];
            m_NodeLocalTransforms[nodeIdx] = m_NodeLocalTransforms[parentIdx - 1] * m_NodeData->m_Nodes[nodeIdx].Transform;

            if (currNode.IsBone) {
                m_BoneVertexTransforms[BoneIndex] =
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                    m_Skeleton->m_GlobalInverseTransform *
#endif
                    m_NodeLocalTransforms[nodeIdx] *
                    m_Skeleton->m_BoneOffsets[BoneIndex]
                ;
                ++BoneIndex;
            }
        }
    }
}
void ModelInstanceAnimationContainer::update(const float dt) {
    if (!m_Animation_Instances.empty()) {
        for (auto& boneVertexTransform : m_BoneVertexTransforms) {
            boneVertexTransform = glm::mat4{ 1.0f };
        }

        for (auto& animation : m_Animation_Instances) {
            animation.update(dt, m_BoneVertexTransforms, m_NodeLocalTransforms, m_NodeSnapshots, m_NumBones, m_NodeData, m_Skeleton);
        }


        //use m_NodeLocalTransforms to store bone local transformations for later use for ragdolls, etc
        size_t boneIndex = 0; 
        for (size_t nodeIdx = 1; nodeIdx != m_NodeData->m_Nodes.size(); ++nodeIdx) {
            if (m_NodeData->m_Nodes[nodeIdx].IsBone) {
                const size_t boneIndexPlusOne = boneIndex + 1;
                assert(boneIndexPlusOne >= 0 && boneIndexPlusOne < m_NodeLocalTransforms.size());
                m_NodeLocalTransforms[boneIndexPlusOne] = //skip node 0 as it is needed to remain constant since we never touch it in the update loop
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                    glm::inverse(m_Skeleton->m_GlobalInverseTransform) *
#endif
                    m_BoneVertexTransforms[boneIndex] *
                    glm::inverse(m_Skeleton->m_BoneOffsets[boneIndex])
                ;
                ++boneIndex;
            }
        }



        //cleanup any completed animations
        std::erase_if(m_Animation_Instances, [](const auto& anim) {
            return anim.m_RequestedLoops > 0 && anim.m_CurrentLoops >= anim.m_RequestedLoops;
        });
        if (m_Animation_Instances.empty()) {
            resetNodeWorldTransformsToRestPose();
        }
    }
}
const glm::mat4& ModelInstanceAnimationContainer::getGlobalInverseTransform() const noexcept { 
    return m_Skeleton->m_GlobalInverseTransform;
}
const glm::mat4& ModelInstanceAnimationContainer::getBoneLocalTransform(size_t boneIndex) const noexcept {
    ++boneIndex; //skip node 0 as it is needed to remain constant since we never touch it in the update loop
    assert(boneIndex >= 0 && boneIndex < m_NodeLocalTransforms.size());
    return m_NodeLocalTransforms[boneIndex];
}

#pragma endregion