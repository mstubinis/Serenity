#pragma once
#ifndef ENGINE_ANIMATION_PROCESSOR_H
#define ENGINE_ANIMATION_PROCESSOR_H

#include <vector>
#include <glm/glm.hpp>

class MeshInstanceAnimation;
class MeshInstance;

class AnimationProcessor final{
    private:
        void _cleanupQueue(std::vector<MeshInstanceAnimation>& queue);
    public:
        AnimationProcessor();
        ~AnimationProcessor();

        void process(MeshInstance*,std::vector<MeshInstanceAnimation>&,std::vector<glm::mat4>& transforms);
};
#endif