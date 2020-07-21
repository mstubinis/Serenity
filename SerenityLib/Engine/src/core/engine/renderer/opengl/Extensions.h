#pragma once
#ifndef ENGINE_RENDERER_OPENGL_EXTENSIONS_H
#define ENGINE_RENDERER_OPENGL_EXTENSIONS_H

//#define ENGINE_PRINT_OPENGL_EXTENSIONS

namespace Engine::priv {
    class OpenGLExtensions final {
        public: enum Extension {
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

            _TOTAL,
        };
        private:

        public:
            void       INIT() noexcept;
            bool       checkOpenGLExtension(const char* e) noexcept;
            void       printAllAvailableExtensions() noexcept;

            OpenGLExtensions() = default;
            ~OpenGLExtensions() = default;

            static bool supported(OpenGLExtensions::Extension extension) noexcept;

            static bool isBindlessTexturesSupported() noexcept;
    };
};

#endif