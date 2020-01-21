#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H
#define ENGINE_RENDERER_OPENGL_GLSL_DOF_CODE_H

#include <core/engine/shaders/ShaderIncludes.h>
#include <string>

namespace Engine {
    namespace priv {
        namespace opengl {
            namespace glsl {
                class DepthOfFieldCode final {
                    public:
                        static void convert(std::string& code, const unsigned int& versionNumber, const ShaderType::Type& shaderType);
                };
            };
        };
    };
};

#endif