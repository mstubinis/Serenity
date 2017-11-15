#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include "BindableResource.h"
#include <unordered_map>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

class Material;
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

class Shader final: public EngineResource{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Shader(std::string name, std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        ~Shader();

        ShaderType::Type type();
        std::string data();
        bool fromFile();
};
class ShaderP final: public BindableResource{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        ShaderP(std::string& name, std::string& vertexShader,std::string& fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        ShaderP(std::string& name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);

        ShaderP(std::string& name, Shader* vertexShader,std::string& fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        ShaderP(std::string& name, std::string& vertexShader,Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);

        ~ShaderP();

        void bind();
        void unbind();

        GLuint program();
        ShaderRenderPass::Pass stage();
        std::vector<Material*>& getMaterials();

        void addMaterial(const std::string&);
        const std::unordered_map<std::string,GLint>& uniforms() const;
};

#endif
