#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;
using namespace std;

ModelInstanceAnimation::ModelInstanceAnimation(Mesh& mesh, const string& animName, const float startTime, const float endTime, const unsigned int requestedLoops) {
    m_CurrentLoops = 0;
    m_RequestedLoops = requestedLoops;
    m_CurrentTime = 0;
    m_StartTime = startTime;
    m_AnimationName = animName;
    m_Mesh = &mesh;
    if (endTime < 0) {
        m_EndTime = mesh.animationData().at(animName).duration();
    }else{
        m_EndTime = endTime;
    }
}