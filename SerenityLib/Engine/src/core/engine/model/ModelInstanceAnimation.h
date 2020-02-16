#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

class  Mesh;
namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
};

#include <string>

namespace Engine::priv {
    class ModelInstanceAnimation final {
        friend struct DefaultModelInstanceBindFunctor;
        private:
            unsigned int   m_CurrentLoops;
            unsigned int   m_RequestedLoops;
            float          m_CurrentTime;
            float          m_StartTime;
            float          m_EndTime;
            std::string    m_AnimationName;
            Mesh*          m_Mesh;
        public:
            ModelInstanceAnimation(Mesh& mesh, const std::string& animName, const float startTime, const float endTime, const unsigned int requestedLoops = 1);

    };
};

#endif