#pragma once
#ifndef ENGINE_MODEL_INSTANCE_ANIMATION_H
#define ENGINE_MODEL_INSTANCE_ANIMATION_H

namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
    class  ModelInstanceAnimationContainer;
    class  AnimationData;
};

#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <string>
#include <vector>
#include <array>

namespace Engine::priv {
    class ModelInstanceAnimation final {
        friend struct DefaultModelInstanceBindFunctor;
        friend class  ModelInstanceAnimationContainer;
        private:
            AnimationData* m_AnimationData    = nullptr;
            uint16_t       m_NumBones         = 0;
            uint16_t       m_CurrentLoops     = 0;
            uint16_t       m_RequestedLoops   = 1;

            float          m_CurrentTime      = 0.0f;
            float          m_StartTime        = 0.0f;
            float          m_EndTime          = 0.0f;

            ModelInstanceAnimation() = delete;
        public:
            ModelInstanceAnimation(Handle mesh, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops = 1);

            ModelInstanceAnimation(const ModelInstanceAnimation&)                = delete;
            ModelInstanceAnimation& operator=(const ModelInstanceAnimation&)     = delete;
            ModelInstanceAnimation(ModelInstanceAnimation&&) noexcept            = default;
            ModelInstanceAnimation& operator=(ModelInstanceAnimation&&) noexcept = default;

            void process(const float dt, std::vector<glm::mat4>& transforms);
    };
    class ModelInstanceAnimationContainer final {
        friend struct DefaultModelInstanceBindFunctor;
        private:
            std::vector<ModelInstanceAnimation> m_Animation_Instances;
            std::vector<glm::mat4>              m_Transforms;
        public:
            void emplace_animation(Handle mesh, std::string_view animationName, float startTime, float endTime, uint16_t requestedLoops);

            [[nodiscard]] inline const std::vector<glm::mat4>& getTransforms() const noexcept { return m_Transforms; }

            [[nodiscard]] inline size_t size() const noexcept { return m_Animation_Instances.size(); }
            void clear();
            void update(const float dt);
    };
};

#endif