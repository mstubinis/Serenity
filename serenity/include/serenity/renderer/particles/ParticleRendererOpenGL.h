#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_RENDERER_OPENGL_H
#define ENGINE_RENDERER_PARTICLE_RENDERER_OPENGL_H

class Material;
namespace Engine::priv {
	class RenderModule;
	class ParticleContainer;
    class GBuffer;
}

#include <cstdint>
#include <serenity/resources/Handle.h>
#include <serenity/containers/BimapUnordered.h>

namespace Engine::priv {
	class ParticleRendererOpenGL {
		private:
			std::vector<uint32_t>   m_VBOs;
		public:
			ParticleRendererOpenGL();
			~ParticleRendererOpenGL();

			void render(Engine::priv::RenderModule&, Engine::priv::ParticleContainer&);

            void render(
				Engine::priv::RenderModule&, 
				Engine::priv::ParticleContainer&,
				size_t particle_count, 
				Handle program,
				Engine::priv::GBuffer&, 
				size_t maxTextureUnits, 
				Engine::unordered_bimap<Material*, uint32_t>& bimap,
				std::unordered_map<uint32_t, uint32_t>& materialIDToIndex
			);
	};
}

#endif