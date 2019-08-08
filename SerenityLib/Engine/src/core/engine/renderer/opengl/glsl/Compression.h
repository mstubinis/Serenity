#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H
#define ENGINE_RENDERER_OPENGL_GLSL_COMPRESSION_H

#include <string>

namespace Engine {
    namespace epriv {
        namespace opengl {
            namespace glsl {
                class Compression final {
                    public:
                        static void convert(std::string& code, const unsigned int& versionNumber);
                };
            };
        };
    };
};

#endif