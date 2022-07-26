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
        public:
            static void LoadCPU(ShaderProgram&);
            static void LoadGPU(ShaderProgram&, bool dispatchEventLoaded = true);
            static void UnloadCPU(ShaderProgram&, bool dispatchEventUnloaded = true);
            static void UnloadGPU(ShaderProgram&);
    };
};

#include <serenity/resources/Resource.h>
#include <serenity/resources/Handle.h>
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/utils/Utils.h>
#include <serenity/resources/shader/ShaderIncludes.h>
#include <functional>

struct ShaderProgramParameters final {
    Handle vertexShaderHandle;
    Handle fragmentShaderHandle;
    Handle geometryShaderHandle;
    Handle computeShaderHandle;
    Handle tesselationControlShaderHandle;
    Handle tesselationEvaluationShaderHandle;
};

class ShaderProgram final : public Resource<ShaderProgram> {
    friend class  ::UniformBufferObject;
    friend class  ::Shader;
    friend class  Engine::priv::PublicShaderProgram;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;
    public:
        using UniformsContainer = Engine::unordered_string_map<std::string, GLint>;
        using BindFunc = void(*)(ShaderProgram*);
    public:
        static Handle Deferred, Forward, Decal; //loaded in renderer
    private:
        BindFunc                                   m_CustomBindFunctor  = [](ShaderProgram*) {};
        UniformsContainer                          m_UniformLocations;
        std::unordered_set<GLuint>                 m_AttachedUBOs;
        std::vector<std::pair<Handle, ShaderType>> m_Shaders;
        GLuint                                     m_ShaderProgram      = 0;
		bool                                       m_LoadedCPU          = false;
		bool                                       m_LoadedGPU          = false;

        void internal_init(std::string_view inName, const ShaderProgramParameters&);
    public:
        ShaderProgram() = default;
        ShaderProgram(std::string_view shaderProgramName, Handle vertexShader, Handle fragmentShader);
        ShaderProgram(std::string_view shaderProgramName, Handle vertexShader, Handle fragmentShader, Handle geometryShader);
        ShaderProgram(std::string_view shaderProgramName, const ShaderProgramParameters&);
        ShaderProgram(std::string_view shaderProgramName, std::string_view vertexShaderFileOrContent, std::string_view fragmentShaderFileOrContent);

        ShaderProgram(const ShaderProgram&)            = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        ShaderProgram(ShaderProgram&&) noexcept;
        ShaderProgram& operator=(ShaderProgram&&) noexcept;
        ~ShaderProgram();

        inline void setCustomBindFunctor(const BindFunc& function) noexcept { m_CustomBindFunctor = function; }
        inline void setCustomBindFunctor(BindFunc&& function) noexcept { m_CustomBindFunctor = std::move(function); }

        void load(bool dispatchEventLoaded = true) override;
        void unload(bool dispatchEventUnloaded = true) override;

        [[nodiscard]] inline const std::vector<std::pair<Handle, ShaderType>>& getShaders() const noexcept { return m_Shaders; }
        [[nodiscard]] inline constexpr GLuint program() const noexcept { return m_ShaderProgram; }
        [[nodiscard]] inline const UniformsContainer& uniforms() const noexcept { return m_UniformLocations; }
};
#endif