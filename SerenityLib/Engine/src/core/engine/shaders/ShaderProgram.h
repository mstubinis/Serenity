#pragma once
#ifndef ENGINE_SHADER_PROGRAM_H
#define ENGINE_SHADER_PROGRAM_H

class  Handle;
class  Shader;
class  ShaderProgram;
class  UniformBufferObject;
namespace Engine::priv {
    class Renderer;
    class IRenderingPipeline;
    struct InternalShaderProgramPublicInterface final {
        static void LoadCPU(ShaderProgram&);
        static void LoadGPU(ShaderProgram&);
        static void UnloadCPU(ShaderProgram&);
        static void UnloadGPU(ShaderProgram&);
    };
};

#include <core/engine/resources/Resource.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <core/engine/shaders/ShaderIncludes.h>

class ShaderProgram final : public Resource, public Engine::NonCopyable {
    friend class  UniformBufferObject;
    friend class  Shader;
    friend struct Engine::priv::InternalShaderProgramPublicInterface;
    friend class  Engine::priv::Renderer;
    friend class  Engine::priv::IRenderingPipeline;
    public:
        static ShaderProgram                      *Deferred, *Forward, *Decal; //loaded in renderer
    private:
        std::function<void(ShaderProgram*)>                    m_CustomBindFunctor;
        GLuint                                                 m_ShaderProgram      = 0;
        std::unordered_map<std::string, GLint>                 m_UniformLocations;
        std::unordered_set<GLuint>                             m_AttachedUBOs;
        Shader&                                                m_VertexShader;
        Shader&                                                m_FragmentShader;
		bool                                                   m_LoadedCPU          = false;
		bool                                                   m_LoadedGPU          = false;
    public:
        ShaderProgram(const std::string& name, Shader& vertexShader, Shader& fragmentShader);
        virtual ~ShaderProgram();

        ShaderProgram(ShaderProgram&& other) noexcept            = default;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept = default;

        template<typename T> void setCustomBindFunctor(const T& functor) {
            m_CustomBindFunctor = std::bind<void>(std::move(functor), std::placeholders::_1);
        }

        constexpr inline operator GLuint() const noexcept { return m_ShaderProgram; }

        void load() override;
        void unload() override;

        constexpr inline GLuint program() const noexcept { return m_ShaderProgram; }
        inline const std::unordered_map<std::string, GLint>& uniforms() const noexcept { return m_UniformLocations; }
};
#endif