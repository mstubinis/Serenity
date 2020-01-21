#pragma once
#ifndef ENGINE_RENDERER_OPENGL_EXTENSIONS_H
#define ENGINE_RENDERER_OPENGL_EXTENSIONS_H

#include <vector>

namespace Engine {
    namespace priv {
        class OpenGLExtensions final {
            public: enum Extension {
                EXT_Ansiotropic_Filtering,
                ARB_Ansiotropic_Filtering,
                EXT_draw_instanced,
                ARB_draw_instanced,
                EXT_separate_shader_objects,
                ARB_separate_shader_objects,
                EXT_explicit_attrib_location,
                ARB_explicit_attrib_location,
                EXT_geometry_shader_4,
                ARB_geometry_shader_4,
                EXT_compute_shader,
                ARB_compute_shader,
                EXT_tessellation_shader,
                ARB_tessellation_shader,
                _TOTAL,
            };
            private:

            public:
                static std::vector<bool> OPENGL_EXTENSIONS;

                void       INIT();
                const bool checkOpenGLExtension(const char* e);
                void       printAllAvailableExtensions();

                OpenGLExtensions();
                ~OpenGLExtensions();

                static const bool supported(const OpenGLExtensions::Extension& extension);
        };
    };
};

#endif