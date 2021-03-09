#pragma once
#ifndef ENGINE_SHADER_PROGRAM_H
#define ENGINE_SHADER_PROGRAM_H

class  Handle;
class  Shader;
class  ShaderProgram;
class  UniformBufferObject;

#include <cstdint>
#include <vector>

namespace Engine::priv {
    class RenderModule;
    class IRenderingPipeline;
    class PublicShaderProgram final {
        private:
            static uint32_t CompileShader(Shader&, const char* sourceCode, uint32_t GLShaderType);
            static bool     LinkShadersToProgram(ShaderProgram&, std::vector<uint32_t>&& shaderIDs, std::vector<char>& errors, const char* fragmentSourceCode = "");
            static void     PopulateUniformTable(ShaderProgram&);
        public:
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
#include <serenity/utils/Utils.h>
#include <serenity/resources/shader/ShaderIncludes.h>
#include <functional>

class ShaderProgram final : public Resource<ShaderProgram> {
    friend class  UniformBufferObject;
    friend class  Shader;
    friend class  Engine::priv::PublicShaderProgram;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;
    public:
        using UniformsContainer = Engine::unordered_string_map<std::string, GLint>;
        using BindFunc = void(*)(ShaderProgram*);

    public:
        static Handle Deferred, Forward, Decal; //loaded in renderer
    private:
        BindFunc                     m_CustomBindFunctor  = [](ShaderProgram*) {};
        UniformsContainer            m_UniformLocations;
        std::unordered_set<GLuint>   m_AttachedUBOs;
        Handle                       m_VertexShader;
        Handle                       m_FragmentShader;
        GLuint                       m_ShaderProgram      = 0;
		bool                         m_LoadedCPU          = false;
		bool                         m_LoadedGPU          = false;
    public:
        ShaderProgram() = default;
        ShaderProgram(std::string_view name, Handle vertexShader, Handle fragmentShader);
        ShaderProgram(const ShaderProgram&)            = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        ShaderProgram(ShaderProgram&&) noexcept;
        ShaderProgram& operator=(ShaderProgram&&) noexcept;
        ~ShaderProgram();

        inline void setCustomBindFunctor(const BindFunc& function) noexcept { m_CustomBindFunctor = function; }
        inline void setCustomBindFunctor(BindFunc&& function) noexcept { m_CustomBindFunctor = std::move(function); }

        //inline constexpr operator GLuint() const noexcept { return m_ShaderProgram; }

        void load() override;
        void unload() override;

        [[nodiscard]] inline constexpr GLuint program() const noexcept { return m_ShaderProgram; }
        [[nodiscard]] inline const UniformsContainer& uniforms() const noexcept { return m_UniformLocations; }
};
#endif