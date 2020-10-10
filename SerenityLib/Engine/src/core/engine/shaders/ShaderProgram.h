#pragma once
#ifndef ENGINE_SHADER_PROGRAM_H
#define ENGINE_SHADER_PROGRAM_H

class  Handle;
class  Shader;
class  ShaderProgram;
class  UniformBufferObject;
namespace Engine::priv {
    class RenderModule;
    class IRenderingPipeline;
    struct InternalShaderProgramPublicInterface final {
        static void LoadCPU(ShaderProgram&);
        static void LoadGPU(ShaderProgram&);
        static void UnloadCPU(ShaderProgram&);
        static void UnloadGPU(ShaderProgram&);
    };
};

#include <core/engine/resources/Resource.h>
#include <core/engine/resources/Handle.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <core/engine/shaders/ShaderIncludes.h>

class ShaderProgram final : public Resource {
    friend class  UniformBufferObject;
    friend class  Shader;
    friend struct Engine::priv::InternalShaderProgramPublicInterface;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;
    public:
        static Handle Deferred, Forward, Decal; //loaded in renderer
    private:
        std::function<void(ShaderProgram*)>                    m_CustomBindFunctor  = [](ShaderProgram*) {};
        std::unordered_map<std::string, GLint>                 m_UniformLocations;
        std::unordered_set<GLuint>                             m_AttachedUBOs;
        Handle                                                 m_VertexShader       = Handle{};
        Handle                                                 m_FragmentShader     = Handle{};
        GLuint                                                 m_ShaderProgram      = 0;
		bool                                                   m_LoadedCPU          = false;
		bool                                                   m_LoadedGPU          = false;
    public:
        ShaderProgram() = default;
        ShaderProgram(const std::string& name, Handle vertexShader, Handle fragmentShader);
        ShaderProgram(const ShaderProgram& other)            = delete;
        ShaderProgram& operator=(const ShaderProgram& other) = delete;
        ShaderProgram(ShaderProgram&& other) noexcept;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept;
        ~ShaderProgram();

        inline void setCustomBindFunctor(std::function<void(ShaderProgram*)>&& function) noexcept { m_CustomBindFunctor = std::move(function); }

        inline CONSTEXPR operator GLuint() const noexcept { return m_ShaderProgram; }

        void load() override;
        void unload() override;

        inline CONSTEXPR GLuint program() const noexcept { return m_ShaderProgram; }
        inline CONSTEXPR const std::unordered_map<std::string, GLint>& uniforms() const noexcept { return m_UniformLocations; }
};
#endif