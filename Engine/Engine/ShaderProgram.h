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
class UniformBufferObject final{
    private:
        class impl; std::unique_ptr<impl> m_i;
        UniformBufferObject(const UniformBufferObject&); // non construction-copyable
        UniformBufferObject& operator=(const UniformBufferObject&); // non copyable
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
    private:
        class impl; std::unique_ptr<impl> m_i;
        Shader(const Shader&); // non construction-copyable
        Shader& operator=(const Shader&); // non copyable
    public:
        Shader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        ~Shader();

        ShaderType::Type type();
        std::string data();
        bool fromFile();
};
class ShaderP final: public BindableResource{
    friend class ::UniformBufferObject;
    private:
        class impl; std::unique_ptr<impl> m_i;
        ShaderP(const ShaderP&); // non construction-copyable
        ShaderP& operator=(const ShaderP&); // non copyable
    public:
        ShaderP(std::string name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);

        ~ShaderP();

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
