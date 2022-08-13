#pragma once
#ifndef ENGINE_SHADER_PROGRAM_H
#define ENGINE_SHADER_PROGRAM_H

class  Handle;
class  Shader;
class  ShaderProgram;
class  UniformBufferObject;
struct ShaderProgramParameters;

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
#include <serenity/utils/Utils.h>
#include <serenity/resources/shader/ShaderIncludes.h>

class ShaderProgram final : public Resource<ShaderProgram> {
    public:
        class Impl;
    friend class  ::UniformBufferObject;
    friend class  ::Shader;
    friend class  Engine::priv::PublicShaderProgram;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;
    friend class  ShaderProgram::Impl;
    public:
        using BindFunc = void(*)(ShaderProgram*);
    public:
        static Handle Deferred, Forward, Decal; //loaded in renderer
    private:
        BindFunc                                            m_CustomBindFunctor  = [](ShaderProgram*) {};
        Engine::unordered_string_map<std::string, int32_t>  m_UniformLocations;
        std::unordered_set<uint32_t>                        m_AttachedUBOs;
        std::vector<std::pair<Handle, ShaderType>>          m_Shaders;
        uint32_t                                            m_ShaderProgram      = 0;
		bool                                                m_LoadedCPU          = false;
		bool                                                m_LoadedGPU          = false;
    public:
        ShaderProgram() = default;
        ShaderProgram(std::string_view shaderProgramName, const ShaderProgramParameters&);
        ShaderProgram(std::string_view shaderProgramName, Handle vertexShader, Handle fragmentShader);
        ShaderProgram(std::string_view shaderProgramName, Handle vertexShader, Handle fragmentShader, Handle geometryShader);
        ShaderProgram(std::string_view shaderProgramName, std::string_view vertexShaderFileOrContent, std::string_view fragmentShaderFileOrContent);

        ShaderProgram(const ShaderProgram&)            = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        ShaderProgram(ShaderProgram&&) noexcept;
        ShaderProgram& operator=(ShaderProgram&&) noexcept;
        ~ShaderProgram();

        template<class FUNC>
        inline void setCustomBindFunctor(FUNC&& function) noexcept { m_CustomBindFunctor = std::forward<FUNC>(function); }

        [[nodiscard]] inline const std::vector<std::pair<Handle, ShaderType>>& getShaders() const noexcept { return m_Shaders; }
        [[nodiscard]] inline constexpr uint32_t program() const noexcept { return m_ShaderProgram; }
        [[nodiscard]] inline const Engine::unordered_string_map<std::string, int32_t>& uniforms() const noexcept { return m_UniformLocations; }
};
#endif