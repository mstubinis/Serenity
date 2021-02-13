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
    struct PublicShaderProgram final {
        static void LoadCPU(ShaderProgram&);
        static void LoadGPU(ShaderProgram&);
        static void UnloadCPU(ShaderProgram&);
        static void UnloadGPU(ShaderProgram&);
    };
};

#include <serenity/resources/Resource.h>
#include <serenity/resources/Handle.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/resources/shader/ShaderIncludes.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>

class ShaderProgram final : public Resource {
    friend class  UniformBufferObject;
    friend class  Shader;
    friend struct Engine::priv::PublicShaderProgram;
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

        inline constexpr operator GLuint() const noexcept { return m_ShaderProgram; }

        void load() override;
        void unload() override;

        [[nodiscard]] inline constexpr GLuint program() const noexcept { return m_ShaderProgram; }
        [[nodiscard]] inline constexpr const std::unordered_map<std::string, GLint>& uniforms() const noexcept { return m_UniformLocations; }
};
#endif