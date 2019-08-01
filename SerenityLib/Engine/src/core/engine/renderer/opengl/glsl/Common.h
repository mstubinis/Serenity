#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMMON_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMMON_H

#include <string>

namespace Engine {
    namespace epriv {
        namespace opengl {
            namespace glsl {
                class Common final {
                    public:
                        static void convert(std::string& code);
                };
            };
        };
    };
};

#endif