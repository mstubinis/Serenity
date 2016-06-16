#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include <string>
#include <memory>
typedef unsigned int uint;

class ShaderP final{
    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        ShaderP(std::string vs,std::string ps, bool fromFile = true);
        ~ShaderP();

        void initRenderingContext(uint api);
        void cleanupRenderingContext(uint api);

        uint program();
        std::string vertexShader();
        std::string pixelShader();
};

#endif