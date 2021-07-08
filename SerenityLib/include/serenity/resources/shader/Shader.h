#pragma once
#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

class ShaderProgram;
class Shader;
namespace Engine::priv {
    class PublicShaderProgram;
    class PublicShader final {
        public:
            static void ConvertCode(Shader&);
    };
};

#include <serenity/resources/Resource.h>
#include <serenity/resources/shader/ShaderIncludes.h>

class Shader final : public Resource<Shader> {
    friend class ShaderProgram;
    friend class Engine::priv::PublicShader;
    friend class Engine::priv::PublicShaderProgram;
    private:
        std::string  m_FileName;
        std::string  m_Code;
        ShaderType   m_ShaderType = ShaderType::Vertex;
        bool         m_FromFile   = false;
    public:
        Shader() = default;
        Shader(std::string_view shaderFileOrData, ShaderType, bool fromFile = true);

        Shader(const Shader&)                  = delete;
        Shader& operator=(const Shader&)       = delete;
        Shader(Shader&&) noexcept;
        Shader& operator=(Shader&&) noexcept;

        [[nodiscard]] inline constexpr ShaderType type() const noexcept { return m_ShaderType; }
        [[nodiscard]] inline constexpr const std::string& data() const noexcept { return m_Code; }
        [[nodiscard]] inline constexpr bool fromFile() const noexcept { return m_FromFile; }
};
#endif