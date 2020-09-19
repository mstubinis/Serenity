#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

class  Mesh;
namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
    class  ModelInstanceAnimationVector;
};

namespace Engine::priv {
    class ModelInstanceAnimation final {
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
            ModelInstanceAnimation(Mesh& mesh, const std::string& animName, float startTime, float endTime, unsigned int requestedLoops = 1);
            ~ModelInstanceAnimation() = default;

            ModelInstanceAnimation(const ModelInstanceAnimation& other)                = delete;
            ModelInstanceAnimation& operator=(const ModelInstanceAnimation& other)     = delete;
            ModelInstanceAnimation(ModelInstanceAnimation&& other) noexcept            = default;
            ModelInstanceAnimation& operator=(ModelInstanceAnimation&& other) noexcept = default;

            void process(const float dt, std::vector<glm::mat4>& transforms);
    };
    class ModelInstanceAnimationVector final {
        friend struct DefaultModelInstanceBindFunctor;
        private:
            std::vector<ModelInstanceAnimation> m_Animation_Instances;
            std::vector<glm::mat4>              m_Transforms;
        public:
            ModelInstanceAnimationVector() = default;
            ~ModelInstanceAnimationVector() = default;

            ModelInstanceAnimationVector(const ModelInstanceAnimationVector& other)                = default;
            ModelInstanceAnimationVector& operator=(const ModelInstanceAnimationVector& other)     = default;
            ModelInstanceAnimationVector(ModelInstanceAnimationVector&& other) noexcept            = default;
            ModelInstanceAnimationVector& operator=(ModelInstanceAnimationVector&& other) noexcept = default;

            void emplace_animation(Mesh&, const std::string& animationName, float start, float end, unsigned int requestedLoops);

            inline size_t size() const noexcept { return m_Animation_Instances.size(); }
            void clear();
            void process(Mesh& mesh, const float dt);
    };
};

#endif