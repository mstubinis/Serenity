#pragma once
#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/shaders/ShaderIncludes.h>

class ShaderProgram;
class Shader;
namespace Engine::priv {
    struct InternalShaderProgramPublicInterface;
    struct InternalShaderPublicInterface final {
        static void ConvertCode(Shader&);
    };
};
class Shader final : public EngineResource {
    friend class  ShaderProgram;
    friend struct Engine::priv::InternalShaderPublicInterface;
    friend struct Engine::priv::InternalShaderProgramPublicInterface;
    private:
        ShaderType::Type   m_Type;
        bool               m_FromFile;
        std::string        m_FileName;
        std::string        m_Code;
    public:
        Shader(const std::string& shaderFileOrData, const ShaderType::Type shaderType, const bool fromFile = true);
        virtual ~Shader();

        Shader& operator=(const Shader&) = delete;
        Shader(const Shader&) = default;
        Shader(Shader&&) noexcept = default;

        ShaderType::Type type() const;
        const std::string& data() const;
        bool fromFile() const;
};
#endif