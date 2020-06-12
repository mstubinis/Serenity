#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;
using namespace std;

ModelInstanceAnimation::ModelInstanceAnimation(Mesh& mesh, const string& animName, const float startTime, const float endTime, const unsigned int requestedLoops){
    m_Mesh             = &mesh;
    m_RequestedLoops   = requestedLoops;
    m_StartTime        = startTime;
    m_AnimationName    = animName;
    if (endTime < 0) {
        m_EndTime      = mesh.animationData().at(animName).duration();
    }else{
        m_EndTime      = endTime;
    }
}
ModelInstanceAnimation::~ModelInstanceAnimation() {

}
ModelInstanceAnimation::ModelInstanceAnimation(ModelInstanceAnimation&& other) noexcept {
    m_CurrentLoops   = std::move(other.m_CurrentLoops);
    m_RequestedLoops = std::move(other.m_RequestedLoops);
    m_CurrentTime    = std::move(other.m_CurrentTime);
    m_StartTime      = std::move(other.m_StartTime);
    m_EndTime        = std::move(other.m_EndTime);
    m_AnimationName  = std::move(other.m_AnimationName);
    m_Mesh           = std::exchange(other.m_Mesh, nullptr);
}
ModelInstanceAnimation& ModelInstanceAnimation::operator=(ModelInstanceAnimation&& other) noexcept {
    if (&other != this){
        m_CurrentLoops   = std::move(other.m_CurrentLoops);
        m_RequestedLoops = std::move(other.m_RequestedLoops);
        m_CurrentTime    = std::move(other.m_CurrentTime);
        m_StartTime      = std::move(other.m_StartTime);
        m_EndTime        = std::move(other.m_EndTime);
        m_AnimationName  = std::move(other.m_AnimationName);
        m_Mesh           = std::exchange(other.m_Mesh, nullptr);
    }
    return *this;
}

void ModelInstanceAnimation::process(const float dt, vector<glm::mat4>& transforms) {
    m_CurrentTime += dt;

    if(transforms.size() == 0){
        transforms.resize(m_Mesh->m_Skeleton->numBones(), glm::mat4(1.0f) );
    }
    auto& animation = m_Mesh->m_Skeleton->m_AnimationData.at(m_AnimationName);
    animation.BoneTransform(m_AnimationName, m_CurrentTime, transforms);

    if (m_CurrentTime >= m_EndTime) {
        m_CurrentTime = 0;
        ++m_CurrentLoops;
    }
}


ModelInstanceAnimationVector::ModelInstanceAnimationVector() {

}
ModelInstanceAnimationVector::~ModelInstanceAnimationVector() {

}
ModelInstanceAnimationVector::ModelInstanceAnimationVector(ModelInstanceAnimationVector&& other) noexcept {
    m_Animation_Instances = std::move(other.m_Animation_Instances);
}
ModelInstanceAnimationVector& ModelInstanceAnimationVector::operator=(ModelInstanceAnimationVector&& other) noexcept {
    if (&other != this) {
        m_Animation_Instances = std::move(other.m_Animation_Instances);
    }
    return *this;
}
void ModelInstanceAnimationVector::emplace_animation(Mesh& mesh, const string& animationName, const float start, const float end, const unsigned int requestedLoops) {
    m_Animation_Instances.emplace_back(mesh, animationName, start, end, requestedLoops);
}

size_t ModelInstanceAnimationVector::size() const {
    return m_Animation_Instances.size();
}
void ModelInstanceAnimationVector::clear() {
    m_Animation_Instances.clear();
    m_Transforms.clear();
}
void ModelInstanceAnimationVector::process(Mesh& mesh, const float dt) {
    if (m_Animation_Instances.size() == 0) {
        return;
    }
    m_Transforms.clear();
    for (size_t j = 0; j < m_Animation_Instances.size(); ++j) {
        auto& animation = m_Animation_Instances[j];
        if (animation.m_Mesh == &mesh) {
            animation.process(dt, m_Transforms);
        }
    }
    //cleanup any completed animations
    for (auto it = m_Animation_Instances.begin(); it != m_Animation_Instances.end();) {
        const ModelInstanceAnimation& anim = (*it);
        if (anim.m_RequestedLoops > 0 && (anim.m_CurrentLoops >= anim.m_RequestedLoops)) {
            it = m_Animation_Instances.erase(it);
        }else{
            ++it;
        }
    }
}