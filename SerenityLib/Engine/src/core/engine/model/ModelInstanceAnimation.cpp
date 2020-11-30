#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;

#pragma region ModelInstanceAnimation

ModelInstanceAnimation::ModelInstanceAnimation(Handle meshHandle, const std::string& animName, float startTime, float endTime, uint32_t requestedLoops)
    : m_Mesh           { meshHandle }
    , m_RequestedLoops { requestedLoops }
    , m_StartTime      { startTime }
    , m_AnimationName  { animName }
    , m_EndTime        { (endTime < 0.0f) ? meshHandle.get<Mesh>()->animationData().at(animName).duration() : endTime }
{}

void ModelInstanceAnimation::process(const float dt, std::vector<glm::mat4>& transforms) {
    m_CurrentTime  += dt;
    auto  mesh      = m_Mesh.get<Mesh>();
    auto  skeleton  = mesh->m_CPUData.m_Skeleton;
    auto& animation = skeleton->m_AnimationData.at(m_AnimationName);
    transforms.resize(skeleton->numBones(), glm::mat4(1.0f));
    animation.BoneTransform(m_AnimationName, m_CurrentTime, transforms);

    if (m_CurrentTime >= m_EndTime) {
        m_CurrentTime = 0.0f;
        ++m_CurrentLoops;
    }
}
#pragma endregion

#pragma region ModelInstanceAnimationVector

void ModelInstanceAnimationVector::emplace_animation(Handle meshHandle, const std::string& animationName, float startTime, float endTime, uint32_t requestedLoops) {
    m_Animation_Instances.emplace_back(meshHandle, animationName, startTime, endTime, requestedLoops);
}
void ModelInstanceAnimationVector::clear() {
    m_Animation_Instances.clear();
    m_Transforms.clear();
}
void ModelInstanceAnimationVector::update(const float dt) {
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