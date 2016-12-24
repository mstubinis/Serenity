#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include "BindableResource.h"
#include <unordered_map>
typedef unsigned int uint;

enum SHADER_PIPELINE_STAGE{
	SHADER_PIPELINE_STAGE_GEOMETRY,
	SHADER_PIPELINE_STAGE_LIGHTING,
	SHADER_PIPELINE_STAGE_POSTPROCESSING,
	SHADER_PIPELINE_STAGE_NONE
};
enum SHADER_TYPE{
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT,
	SHADER_TYPE_GEOMETRY
};

class Material;
typedef int GLint;
typedef unsigned int GLuint;
class Shader final: public EngineResource{
	private:
		class impl;
		std::unique_ptr<impl> m_i;
	public:
		Shader(std::string& name, std::string& shaderFileOrData, SHADER_TYPE shaderType, bool fromFile = true);
		~Shader();

		SHADER_TYPE type();
		std::string data();
		bool fromFile();
};


class ShaderP final: public BindableResource{
    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        ShaderP(std::string& name, std::string& vertexShader,std::string& fragmentShader, SHADER_PIPELINE_STAGE = SHADER_PIPELINE_STAGE_GEOMETRY);
		ShaderP(std::string& name, Shader* vertexShader, Shader* fragmentShader, SHADER_PIPELINE_STAGE = SHADER_PIPELINE_STAGE_GEOMETRY);
        ~ShaderP();

        void initRenderingContext();
        void cleanupRenderingContext();

        GLuint program();
		SHADER_PIPELINE_STAGE stage();
        Shader* vertexShader();
        Shader* fragmentShader();
		std::vector<skey>& getMaterials();

		void addMaterial(std::string);
		const std::unordered_map<std::string,GLint>& uniforms() const;
};

#endif