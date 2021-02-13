#pragma once
#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

class ShaderProgram;
class Shader;
namespace Engine::priv {
    struct PublicShaderProgram;
    struct PublicShader final {
        static void ConvertCode(Shader&);
    };
};

#include <serenity/resources/Resource.h>
#include <serenity/resources/shader/ShaderIncludes.h>

class Shader final : public Resource {
    friend class  ShaderProgram;
    friend struct Engine::priv::PublicShader;
    friend struct Engine::priv::PublicShaderProgram;
    private:
        ShaderType   m_Type     = ShaderType::Unknown;
        bool         m_FromFile = false;
        std::string  m_FileName;
        std::string  m_Code;
    public:
        Shader() = default;
        Shader(const std::string& shaderFileOrData, ShaderType shaderType, bool fromFile = true);

        Shader(const Shader& other) = default; //TODO: delete here?
        Shader& operator=(const Shader& other) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        [[nodiscard]] inline constexpr ShaderType type() const noexcept { return m_Type; }
        [[nodiscard]] inline constexpr const std::string& data() const noexcept { return m_Code; }
        [[nodiscard]] inline constexpr bool fromFile() const noexcept { return m_FromFile; }
};
#endif