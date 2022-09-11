#pragma once
#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

class ShaderProgram;
class Shader;
namespace Engine::priv {
    class PublicShaderProgram;
};

#include <serenity/resources/Resource.h>
#include <serenity/resources/shader/ShaderIncludes.h>

class Shader final : public Resource<Shader> {
    friend class ShaderProgram;
    friend class Engine::priv::PublicShaderProgram;
    private:
        std::string  m_FileName;
        std::string  m_Code;
        ShaderType   m_ShaderType = ShaderType::Vertex;
        uint32_t     m_GLShaderID = 0;
    public:
        Shader() = default;
        Shader(std::string_view shaderFileOrData, ShaderType);

        Shader(const Shader&)                  = delete;
        Shader& operator=(const Shader&)       = delete;
        Shader(Shader&&) noexcept;
        Shader& operator=(Shader&&) noexcept;
        ~Shader();

        void loadCode(const std::string& code);

        //orphans the m_Code content if loaded from file unless force = true, in which case m_Code is cleared.
        void orphan(bool force = false);

        [[nodiscard]] inline uint32_t getGLShaderID() const noexcept { return m_GLShaderID; }
        [[nodiscard]] inline const std::string& getFilename() const noexcept { return m_FileName; }
        [[nodiscard]] inline ShaderType getType() const noexcept { return m_ShaderType; }
        [[nodiscard]] inline const std::string& getData() const noexcept { return m_Code; }
        [[nodiscard]] inline bool isFromFile() const noexcept { return !m_FileName.empty(); }
};
#endif