#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include "BindableResource.h"
#include <unordered_map>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct Handle;
class Material;
class Shader;
class ShaderP;
class UniformBufferObject;
typedef unsigned int uint;

class ShaderRenderPass{public: enum Pass{
    Geometry,
    Forward,
    Lighting,
    Postprocess,
    None,
};};
class ShaderType{public:enum Type{
    Vertex,
    Fragment,
    Geometry,
};};

//Core since version 3.1 (GLSL 140)
class UniformBufferObject final: private Engine::epriv::noncopyable{
	friend class ::Shader;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        static UniformBufferObject* UBO_CAMERA;

        static GLint MAX_UBO_BINDINGS;
        static uint CUSTOM_UBO_AUTOMATIC_COUNT;

        UniformBufferObject(const char* nameInShader,uint sizeofStruct,int globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void attachToShader(ShaderP* shaderProgram);
        void updateData(void* data);
};

class Shader final: public EngineResource{
	friend class ::ShaderP;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Shader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        virtual ~Shader();

        ShaderType::Type type();
        std::string data();
        bool fromFile();
};
class InternalShaderProgramPublicInterface final{
    public:
        static void LoadCPU(ShaderP*);
        static void LoadGPU(ShaderP*);
		static void UnloadCPU(ShaderP*);
		static void UnloadGPU(ShaderP*);
};
class ShaderP final: public BindableResource{
    friend class ::UniformBufferObject;
	friend class ::Shader;
	friend class ::InternalShaderProgramPublicInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        ShaderP(std::string name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        virtual ~ShaderP();

		void load();
		void unload();

        void bind();
        void unbind();

        GLuint program();
        ShaderRenderPass::Pass stage();
        std::vector<Material*>& getMaterials();

        void addMaterial(Handle& materialHandle);
        void addMaterial(Material*);
        const std::unordered_map<std::string,GLint>& uniforms() const;
};

#endif
