
#include <serenity/model/ModelInstanceAnimation.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/animation/AnimationData.h>

using namespace Engine::priv;

#pragma region ModelInstanceAnimation

ModelInstanceAnimation::ModelInstanceAnimation(Handle meshHandle, std::string_view animName, float startTime, float endTime, uint16_t requestedLoops)
    : m_AnimationData  { &meshHandle.get<Mesh>()->getAnimationData().find(animName)->second }
    , m_NumBones       { meshHandle.get<Mesh>()->getSkeleton()->numBones() }
    , m_RequestedLoops { requestedLoops }
    , m_StartTime      { startTime }
    , m_EndTime        { (endTime < 0.0f) ? m_AnimationData->duration() : endTime }
{
}

void ModelInstanceAnimation::process(const float dt, std::vector<glm::mat4>& transforms) {
    m_CurrentTime  += dt;
    transforms.resize(m_NumBones, glm::mat4{ 1.0f });
    m_AnimationData->ComputeTransforms(m_CurrentTime, transforms, m_KeyframeIndices);
    if (m_CurrentTime >= m_EndTime) {
        m_CurrentTime = 0.0f;
        ++m_CurrentLoops;
    }
}
#pragma endregion

#pragma region ModelInstanceAnimationContainer

void ModelInstanceAnimationContainer::emplace_animation(Handle meshHandle, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops) {
    m_Animation_Instances.emplace_back(meshHandle, animationName, startTime, endTime, requestedLoops);
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
        animation.process(dt, m_Transforms);
    });
    //cleanup any completed animations
    std::erase_if(m_Animation_Instances, [](const auto& anim) {
        return anim.m_RequestedLoops > 0 && anim.m_CurrentLoops >= anim.m_RequestedLoops;
    });
}
#pragma endregion