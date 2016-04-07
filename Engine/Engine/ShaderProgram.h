#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <GL/GL.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

class ShaderP final{
    private:
        GLuint m_Shader;
		bool m_FromFile;
        std::string m_VertexShader;
        std::string m_PixelShader;
    public:
        ShaderP(std::string vs,std::string ps, bool fromFile = true);
        ~ShaderP();

		void initRenderingContext(unsigned int api);
		void cleanupRenderingContext(unsigned int api);

		GLuint CompileOpenGL(std::string vs, std::string ps, bool fromFile);
		void CompileDirectX(std::string vs, std::string ps, bool fromFile);

        const GLuint getShaderProgram() const{ return m_Shader; }

        const std::string getVertexShader() const{ return m_VertexShader; }
        const std::string getPixelShader() const{ return m_PixelShader; }
};

#endif