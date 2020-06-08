#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

class  Mesh;
namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
    class  ModelInstanceAnimationVector;
};

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>

namespace Engine::priv {
    class ModelInstanceAnimation final : public Engine::NonCopyable {
        friend struct DefaultModelInstanceBindFunctor;
        friend class  ModelInstanceAnimationVector;
        private:
            unsigned int   m_CurrentLoops     = 0U;
            unsigned int   m_RequestedLoops   = 1U;
            float          m_CurrentTime      = 0.0f;
            float          m_StartTime        = 0.0f;
            float          m_EndTime          = 0.0f;
            std::string    m_AnimationName    = "";
            Mesh*          m_Mesh             = nullptr;
        public:
            ModelInstanceAnimation(Mesh& mesh, const std::string& animName, const float startTime, const float endTime, const unsigned int requestedLoops = 1);
            ~ModelInstanceAnimation();

            ModelInstanceAnimation(ModelInstanceAnimation&& other) noexcept;
            ModelInstanceAnimation& operator=(ModelInstanceAnimation&& other) noexcept;

            void process(const float dt, std::vector<glm::mat4>& transforms);
    };
    class ModelInstanceAnimationVector final : public Engine::NonCopyable {
        friend struct DefaultModelInstanceBindFunctor;
        private:
            std::vector<ModelInstanceAnimation> m_Animation_Instances;
            std::vector<glm::mat4>              m_Transforms;
        public:
            ModelInstanceAnimationVector();
            ~ModelInstanceAnimationVector();

            ModelInstanceAnimationVector(ModelInstanceAnimationVector&& other) noexcept;
            ModelInstanceAnimationVector& operator=(ModelInstanceAnimationVector&& other) noexcept;

            void emplace_animation(Mesh&, const std::string& animationName, const float start, const float end, const unsigned int requestedLoops);

            size_t size() const;
            void clear();
            void process(Mesh& mesh, const float dt);
    };
};

#endif