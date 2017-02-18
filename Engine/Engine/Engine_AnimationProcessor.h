#pragma once
#ifndef ENGINE_ANIMATION_PROCESSOR_H
#define ENGINE_ANIMATION_PROCESSOR_H

#include <vector>
#include <glm/glm.hpp>

class RenderedItemAnimation;
class RenderedItem;

class AnimationProcessor final{
	private:
		void _cleanupQueue(std::vector<RenderedItemAnimation>& queue);
	public:
		AnimationProcessor();
		~AnimationProcessor();

		void process(RenderedItem*,std::vector<RenderedItemAnimation>&,std::vector<glm::mat4>& transforms);
};
#endif