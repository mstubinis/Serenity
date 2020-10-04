#pragma once
#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

class ShaderProgram;
class Shader;
namespace Engine::priv {
    struct InternalShaderProgramPublicInterface;
    struct InternalShaderPublicInterface final {
        static void ConvertCode(Shader&);
    };
};

#include <core/engine/resources/Resource.h>
#include <core/engine/shaders/ShaderIncludes.h>

class Shader final : public Resource {
    friend class  ShaderProgram;
    friend struct Engine::priv::InternalShaderPublicInterface;
    friend struct Engine::priv::InternalShaderProgramPublicInterface;
    private:
        ShaderType   m_Type     = ShaderType::Unknown;
        bool         m_FromFile = false;
        std::string  m_FileName = "";
        std::string  m_Code     = "";
    public:
        Shader() = default;
        Shader(const std::string& shaderFileOrData, ShaderType shaderType, bool fromFile = true);

        Shader(const Shader& other) = default; //TODO: delete here?
        Shader& operator=(const Shader& other) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;
        ~Shader() = default;


        inline CONSTEXPR ShaderType type() const noexcept { return m_Type; }
        inline CONSTEXPR const std::string& data() const noexcept { return m_Code; }
        inline CONSTEXPR bool fromFile() const noexcept { return m_FromFile; }
};
#endif