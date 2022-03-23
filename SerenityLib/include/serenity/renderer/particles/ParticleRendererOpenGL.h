#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_RENDERER_OPENGL_H
#define ENGINE_RENDERER_PARTICLE_RENDERER_OPENGL_H

namespace Engine::priv {
	class Renderer;
	class ParticleContainer;
}

#include <cstdint>

namespace Engine::priv {
	class ParticleRendererOpenGL {
		private:
			uint32_t   m_VBO = 0;
		public:
			
			void init();

			void render(Engine::priv::Renderer&, Engine::priv::ParticleContainer&);
	};
}

#endif