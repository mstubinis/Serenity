#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include <core/engine/BindableResource.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/Engine_Utils.h>
#include <unordered_map>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct Handle;
class Shader;
class ShaderP;
class UniformBufferObject;

struct ShaderType{enum Type{
    Vertex,                // GL_VERTEX_SHADER
    Fragment,              // GL_FRAGMENT_SHADER
    Geometry,              // GL_GEOMETRY_SHADER
    Compute,               // GL_COMPUTE_SHADER          (requires GL 4.3 or ARB_compute_shader)
    TessellationControl,   // GL_TESS_CONTROL_SHADER     (requires GL 4.0 or ARB_tessellation_shader)
    Evaluation,            // GL_TESS_EVALUATION_SHADER  (requires GL 4.0 or ARB_tessellation_shader)
_TOTAL};};

//Core since version 3.1 (GLSL 140)
class UniformBufferObject final: public EventObserver{
    friend class ::Shader;
    private:
        const char* m_NameInShader;
        uint m_SizeOfStruct;
        int m_GlobalBindingPointNumber;
        GLuint m_UBOObject;

        void _load_CPU(); void _unload_CPU();
        void _load_GPU(); void _unload_GPU();
    public:
        static UniformBufferObject* UBO_CAMERA;

        static GLint MAX_UBO_BINDINGS;
        static uint CUSTOM_UBO_AUTOMATIC_COUNT;

        UniformBufferObject(const char* nameInShader,uint sizeofStruct,int globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void onEvent(const Event& e);
        GLuint address();

        void attachToShader(ShaderP& shaderProgram);
        void updateData(void* data);
};

class Shader final: public EngineResource{
    friend class ::ShaderP;
    private:
        ShaderType::Type m_Type;
        bool m_FromFile;
        std::string m_FileName, m_Code;
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
class ShaderP final: public BindableResource, public EventObserver{
    friend class ::UniformBufferObject;
    friend class ::Shader;
    friend struct Engine::epriv::InternalShaderProgramPublicInterface;
    private:
        GLuint m_ShaderProgram;
        std::unordered_map<std::string, GLint> m_UniformLocations;
        std::unordered_map<GLuint, bool> m_AttachedUBOs;
        Shader& m_VertexShader;
        Shader& m_FragmentShader;
        bool m_LoadedCPU, m_LoadedGPU;

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

        void onEvent(const Event& e);

        void load();
        void unload();

        void bind();
        void unbind();

        GLuint program();

        const std::unordered_map<std::string,GLint>& uniforms() const;
};
#endif