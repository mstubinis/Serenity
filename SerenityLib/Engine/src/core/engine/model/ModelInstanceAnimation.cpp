#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;

ModelInstanceAnimation::ModelInstanceAnimation(Handle meshHandle, const std::string& animName, float startTime, float endTime, unsigned int requestedLoops)
    : m_Mesh{ meshHandle }
    , m_RequestedLoops{ requestedLoops }
    , m_StartTime{ startTime }
    , m_AnimationName{ animName }
    , m_EndTime{ (endTime < 0) ? meshHandle.get<Mesh>()->animationData().at(animName).duration() : endTime }
{}

void ModelInstanceAnimation::process(const float dt, std::vector<glm::mat4>& transforms) {
    m_CurrentTime += dt;
    auto& mesh     = *m_Mesh.get<Mesh>();
    if(transforms.size() == 0){
        transforms.resize(mesh.m_Skeleton->numBones(), glm::mat4(1.0f) );
    }
    auto& animation = mesh.m_Skeleton->m_AnimationData.at(m_AnimationName);
    animation.BoneTransform(m_AnimationName, m_CurrentTime, transforms);

    if (m_CurrentTime >= m_EndTime) {
        m_CurrentTime = 0;
        ++m_CurrentLoops;
    }
}


void ModelInstanceAnimationVector::emplace_animation(Handle meshHandle, const std::string& animationName, float start, float end, unsigned int requestedLoops) {
    m_Animation_Instances.emplace_back(meshHandle, animationName, start, end, requestedLoops);
}

void ModelInstanceAnimationVector::clear() {
    m_Animation_Instances.clear();
    m_Transforms.clear();
}
void ModelInstanceAnimationVector::process(Handle meshHandle, const float dt) {
    if (m_Animation_Instances.size() == 0) {
        return;
    }
    m_Transforms.clear();
    for (size_t j = 0; j < m_Animation_Instances.size(); ++j) {
        auto& animation = m_Animation_Instances[j];
        if (animation.m_Mesh == meshHandle) {
            animation.process(dt, m_Transforms);
        }
    }
    //cleanup any completed animations
    std::erase_if(m_Animation_Instances, [](const auto& anim) {
        return anim.m_RequestedLoops > 0 && (anim.m_CurrentLoops >= anim.m_RequestedLoops);
    });
}