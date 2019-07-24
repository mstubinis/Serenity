#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include <core/engine/BindableResource.h>
#include <core/engine/Engine_Utils.h>
#include <unordered_map>
#include <unordered_set>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct Handle;
class  Shader;
class  ShaderP;
class  UniformBufferObject;
struct ShaderType{enum Type{
    Vertex,                // GL_VERTEX_SHADER
    Fragment,              // GL_FRAGMENT_SHADER
    Geometry,              // GL_GEOMETRY_SHADER
    Compute,               // GL_COMPUTE_SHADER          (requires GL 4.3 or ARB_compute_shader)
    TessellationControl,   // GL_TESS_CONTROL_SHADER     (requires GL 4.0 or ARB_tessellation_shader)
    Evaluation,            // GL_TESS_EVALUATION_SHADER  (requires GL 4.0 or ARB_tessellation_shader)
_TOTAL};};

class Shader final: public EngineResource{
    friend class ::ShaderP;
    private:
        ShaderType::Type   m_Type;
        bool               m_FromFile;
        std::string        m_FileName;
        std::string        m_Code;
    public:
        Shader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        virtual ~Shader();

        Shader& operator=(const Shader&) = delete;
        Shader(const Shader&) = default;
        Shader(Shader&&) noexcept = default;

        ShaderType::Type type();
        std::string data();
        bool fromFile();
};

namespace Engine {
namespace epriv {
    struct InternalShaderProgramPublicInterface final {
        static void LoadCPU(ShaderP&);
        static void LoadGPU(ShaderP&);
        static void UnloadCPU(ShaderP&);
        static void UnloadGPU(ShaderP&);
    };
};
};
class ShaderP final: public BindableResource{
    friend class UniformBufferObject;
    friend class Shader;
    friend struct Engine::epriv::InternalShaderProgramPublicInterface;
    private:
        GLuint                                    m_ShaderProgram;
        std::unordered_map<std::string, GLint>    m_UniformLocations;
        std::unordered_set<GLuint>                m_AttachedUBOs;
        Shader&                                   m_VertexShader;
        Shader&                                   m_FragmentShader;
		bool                                      m_LoadedCPU;
		bool                                      m_LoadedGPU;

        void _convertCode(std::string& vCode, std::string& fCode, ShaderP&);
        void _convertCode(std::string&, Shader&, ShaderP&);

        void _load_CPU(ShaderP&); void _unload_CPU(ShaderP&);
        void _load_GPU(ShaderP&); void _unload_GPU(ShaderP&);
    public:
        ShaderP(std::string name, Shader& vertexShader, Shader& fragmentShader);
        virtual ~ShaderP();

        ShaderP(const ShaderP& other) = default;
        ShaderP& operator=(const ShaderP& other) = delete;
        ShaderP(ShaderP&& other) noexcept = default;
        ShaderP& operator=(ShaderP&& other) noexcept = default;

        inline operator GLuint() const { return m_ShaderProgram; }

        void load();
        void unload();

        void bind();
        void unbind();

        const GLuint& program() const;

        const std::unordered_map<std::string,GLint>& uniforms() const;
};
#endif