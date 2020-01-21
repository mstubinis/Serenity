#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

struct Handle;
class  Shader;
class  ShaderProgram;
class  UniformBufferObject;

#include <core/engine/BindableResource.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <core/engine/shaders/ShaderIncludes.h>

namespace Engine {
    namespace priv {
        struct InternalShaderProgramPublicInterface final {
            static void LoadCPU(ShaderProgram&);
            static void LoadGPU(ShaderProgram&);
            static void UnloadCPU(ShaderProgram&);
            static void UnloadGPU(ShaderProgram&);
        };
    };
};
class ShaderProgram final: public BindableResource{
    friend class  UniformBufferObject;
    friend class  Shader;
    friend struct Engine::priv::InternalShaderProgramPublicInterface;
    public:
        static ShaderProgram                      *Deferred, *Forward, *Decal; //loaded in renderer
    private:
        GLuint                                    m_ShaderProgram;
        std::unordered_map<std::string, GLint>    m_UniformLocations;
        std::unordered_set<GLuint>                m_AttachedUBOs;
        Shader&                                   m_VertexShader;
        Shader&                                   m_FragmentShader;
		bool                                      m_LoadedCPU;
		bool                                      m_LoadedGPU;
    public:
        ShaderProgram(const std::string& name, Shader& vertexShader, Shader& fragmentShader);
        virtual ~ShaderProgram();

        ShaderProgram(const ShaderProgram& other)                = delete;
        ShaderProgram& operator=(const ShaderProgram& other)     = delete;
        ShaderProgram(ShaderProgram&& other) noexcept            = default;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept = default;

        inline operator GLuint() const { return m_ShaderProgram; }

        void load() override;
        void unload() override;

        void bind() override;
        void unbind() override;

        const GLuint& program() const;

        const std::unordered_map<std::string, GLint>& uniforms() const;
};
#endif