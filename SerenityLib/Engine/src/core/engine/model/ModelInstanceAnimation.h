#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
    class  ModelInstanceAnimationVector;
};

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class ModelInstanceAnimation final {
        friend struct DefaultModelInstanceBindFunctor;
        friend class  ModelInstanceAnimationVector;
        private:
            std::string    m_AnimationName    = "";
            Handle         m_Mesh             = Handle{};
            uint32_t       m_CurrentLoops     = 0U;
            uint32_t       m_RequestedLoops   = 1U;
            float          m_CurrentTime      = 0.0f;
            float          m_StartTime        = 0.0f;
            float          m_EndTime          = 0.0f;

            ModelInstanceAnimation() = delete;
        public:
            ModelInstanceAnimation(Handle mesh, const std::string& animationName, float startTime, float endTime, uint32_t requestedLoops = 1);

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
            void emplace_animation(Handle mesh, const std::string& animationName, float startTime, float endTime, uint32_t requestedLoops);

            inline const std::vector<glm::mat4>& getTransforms() const noexcept { return m_Transforms; }

            inline size_t size() const noexcept { return m_Animation_Instances.size(); }
            void clear();
            void update(const float dt);
    };
};

#endif