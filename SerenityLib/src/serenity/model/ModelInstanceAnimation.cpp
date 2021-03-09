
#include <serenity/model/ModelInstanceAnimation.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/animation/AnimationData.h>

using namespace Engine::priv;

#pragma region ModelInstanceAnimation

ModelInstanceAnimation::ModelInstanceAnimation(MeshNodeData& nodeData, AnimationData& animData, MeshSkeleton& skeleton, float startTime, float endTime, uint16_t requestedLoops)
    : m_AnimationData  { &animData }
    , m_NodeData       { &nodeData }
    , m_Skeleton       { &skeleton }
    , m_NumBones       { skeleton.numBones() }
    , m_RequestedLoops { requestedLoops }
    , m_StartTime      { startTime }
    , m_EndTime        { (endTime < 0.0f) ? animData.duration() : endTime }
{
}

void ModelInstanceAnimation::update(const float dt, std::vector<glm::mat4>& transforms) {
    m_CurrentTime  += dt;
    transforms.resize(m_NumBones, glm::mat4{ 1.0f });
    m_AnimationData->ComputeTransforms(m_CurrentTime, transforms, *m_Skeleton, *m_NodeData);
    if (m_CurrentTime >= m_EndTime) {
        m_CurrentTime = 0.0f;
        ++m_CurrentLoops;
    }
}
#pragma endregion

#pragma region ModelInstanceAnimationContainer

void ModelInstanceAnimationContainer::internal_emplace_animation(Handle meshHandle, const uint16_t animationIndex, float startTime, float endTime, uint16_t requestedLoops) noexcept {
    auto& mesh     = *meshHandle.get<Mesh>();
    auto& nodeData = mesh.m_CPUData.m_NodeData;
    auto& skeleton = *mesh.getSkeleton();
    auto& animData = skeleton.m_AnimationData[animationIndex];
    m_Animation_Instances.emplace_back(nodeData, animData, skeleton, startTime, endTime, requestedLoops);
}

void ModelInstanceAnimationContainer::emplace_animation(Handle meshHandle, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops) {
    auto& mesh      = *meshHandle.get<Mesh>();
    auto& skeleton  = *mesh.getSkeleton();
    internal_emplace_animation(meshHandle, skeleton.getAnimationIndex(animationName), startTime, endTime, requestedLoops);
}
void ModelInstanceAnimationContainer::emplace_animation(Handle meshHandle, const uint16_t animationIndex, float startTime, float endTime, uint16_t requestedLoops) {
    internal_emplace_animation(meshHandle, animationIndex, startTime, endTime, requestedLoops);
}
void ModelInstanceAnimationContainer::clear() {
    m_Animation_Instances.clear();
    m_Transforms.clear();
}
void ModelInstanceAnimationContainer::update(const float dt) {
    if (m_Animation_Instances.size() == 0) {
        return;
    }
    m_Transforms.clear();
    std::for_each(std::begin(m_Animation_Instances), std::end(m_Animation_Instances), [this, dt](auto& animation) {
        animation.update(dt, m_Transforms);
    });
    //cleanup any completed animations
    std::erase_if(m_Animation_Instances, [](const auto& anim) {
        return anim.m_RequestedLoops > 0 && anim.m_CurrentLoops >= anim.m_RequestedLoops;
    });
}
#pragma endregion