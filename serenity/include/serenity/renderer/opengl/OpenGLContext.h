#pragma once
#ifndef ENGINE_RENDERER_OPENGL_CONTEXT_H
#define ENGINE_RENDERER_OPENGL_CONTEXT_H

class Window;

#include <memory>
#include <serenity/renderer/APIContext.h>

namespace Engine::priv {
	class OpenGLContext : public IContext {
        public: struct Extensions {
            enum Type : uint32_t {
                EXT_texture_filter_anisotropic,
                ARB_texture_filter_anisotropic,
                EXT_draw_instanced,
                ARB_draw_instanced,
                EXT_separate_shader_objects,
                ARB_separate_shader_objects,
                EXT_explicit_attrib_location,
                ARB_explicit_attrib_location,
                EXT_geometry_shader4,
                ARB_geometry_shader4,
                EXT_compute_shader,
                ARB_compute_shader,
                EXT_tessellation_shader,
                ARB_tessellation_shader,
                ARB_bindless_texture,
                NV_bindless_texture,

                ARB_gpu_shader_int64,
                NV_gpu_shader5,
                ARB_clip_control,
                ARB_seamless_cube_map,

                _TOTAL,
            };
		};

		private:
			struct impl;
			std::unique_ptr<impl> m_i;
		public:
			OpenGLContext();

			bool init(Window&) override;
			void destroy(Window&) override;
			bool detatch(Window&) override;

			void setFramerateLimit(int limit) override;
			bool setVerticalSyncEnabled(bool enabled) override;
			bool display() override;

            static bool checkOpenGLExtension(const char* e) noexcept;
            static void printAllAvailableExtensions() noexcept;
            static bool supported(Extensions::Type extension) noexcept;
            static bool isBindlessTexturesSupported() noexcept;
	};
}

#endif