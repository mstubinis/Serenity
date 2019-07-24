#pragma once
#ifndef ENGINE_OPENGL_STATE_MACHINE_H
#define ENGINE_OPENGL_STATE_MACHINE_H

/*
this class provides a cpu side implementation of opengl's state machine, including default values
*/

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>

namespace Engine {
    namespace epriv {
        class  RenderManager;
        class  OpenGLState final {
            friend class  RenderManager;
            private:
                #pragma region TextureUnits
                struct TextureUnit final {
                    GLuint targetTexture1D;
                    GLuint targetTexture2D;
                    GLuint targetTexture3D;
                    GLuint targetTextureCube;
                    TextureUnit() {
                        targetTexture1D   = 999999;
                        targetTexture2D   = 999999;
                        targetTexture3D   = 999999;
                        targetTextureCube = 999999;
                    }
                };
                std::vector<TextureUnit> textureUnits;
                GLuint                   currentTextureUnit;
                #pragma endregion

                #pragma region ClearColor
                struct ClearColor final {
                    GLfloat r;
                    GLfloat g;
                    GLfloat b;
                    GLfloat a;
                    ClearColor() {
                        r = 0.0f;
                        g = 0.0f;
                        b = 0.0f;
                        a = 0.0f;
                    }
                };
                ClearColor clearColor;
                #pragma endregion

                #pragma region ClearDepth
                struct ClearDepth final {
                    GLdouble depth;
                    GLfloat  depthf;
                    ClearDepth() {
                        depth  = 1.0;
                        depthf = 1.0f;
                    }
                };
                ClearDepth clearDepth;
                #pragma endregion

                #pragma region ClearStencil
                struct ClearStencil final {
                    GLint stencil;
                    ClearStencil() {
                        stencil = 0;
                    }
                };
                ClearStencil clearStencil;
                #pragma endregion

                #pragma region StencilMask
                struct StencilMask final {
                    GLuint front_mask;
                    GLuint back_mask;
                    StencilMask() {
                        front_mask = 0xFFFFFFFF;
                        back_mask = 0xFFFFFFFF;
                    }
                };
                StencilMask stencilMask;
                #pragma endregion

                #pragma region StencilOp
                struct StencilOp final {
                    GLenum sFail_front;
                    GLenum dpFail_front;
                    GLenum dpPass_front;

                    GLenum sFail_back;
                    GLenum dpFail_back;
                    GLenum dpPass_back;

                    StencilOp() {
                        sFail_front  = GL_KEEP;
                        dpFail_front = GL_KEEP;
                        dpPass_front = GL_KEEP;

                        sFail_back   = GL_KEEP;
                        dpFail_back  = GL_KEEP;
                        dpPass_back  = GL_KEEP;
                    }
                };
                StencilOp stencilOp;
                #pragma endregion

                #pragma region UseProgram
                struct UseProgram final {
                    GLuint program;
                    UseProgram() {
                        program = 0;
                    }
                };
                UseProgram useProgram;
                #pragma endregion

                #pragma region VAO
                struct VertexArrayObj final {
                    GLuint vao;
                    VertexArrayObj() {
                        vao = 0;
                    }
                };
                VertexArrayObj vaoState;
                #pragma endregion

                #pragma region Viewport
                struct ViewportState final {
                    GLint   x;
                    GLint   y;
                    GLsizei width;
                    GLsizei height;
                    ViewportState() {
                        x = 0;
                        y = 0;
                        width = 0;
                        height = 0;
                    }
                    //note - this is the real default glViewport state
                    ViewportState(const GLsizei& _width, const GLsizei& _height) {
                        x = 0;
                        y = 0;
                        width = _width;
                        height = _height;
                    }
                };
                ViewportState viewportState;
                #pragma endregion

                #pragma region CullFace
                struct CullFaceState final {
                    GLenum mode;
                    CullFaceState() {
                        mode = GL_BACK;
                    }
                };
                CullFaceState cullFaceState;
                #pragma endregion

                #pragma region FrontFace
                struct FrontFaceState final {
                    GLenum mode;
                    FrontFaceState() {
                        mode = GL_CCW;
                    }
                };
                FrontFaceState frontFaceState;
                #pragma endregion

                void GL_INIT_DEFAULT_STATE_MACHINE(const unsigned int& windowWidth, const unsigned int& windowHeight);
            public:
                const bool GL_glActiveTexture(const GLenum& textureUnit);
                const bool GL_glBindTextureForModification(const GLenum& textureTarget, const GLuint& textureObject);
                const bool GL_glBindTextureForRendering(const GLenum& textureTarget, const GLuint& textureObject);
                const bool GL_glClearColor(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a);
                const bool GL_glClearDepth(const GLdouble& depth);
                const bool GL_glClearDepthf(const GLfloat& depth_float);
                const bool GL_glClearStencil(const GLint& stencil);
                const bool GL_glStencilMaskSeparate(const GLenum& face, const GLuint& mask);
                const bool GL_glStencilMask(const GLuint& mask);
                const bool GL_glStencilOp(const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass);
                const bool GL_glStencilOpSeparate(const GLenum& face, const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass);
                const bool GL_glUseProgram(const GLuint& program);
                const bool GL_glBindVertexArray(const GLuint& vao);
                const bool GL_glViewport(const GLint& x, const GLint& y, const GLsizei& width, const GLsizei& height);
                const bool GL_glCullFace(const GLenum& mode);
                const bool GL_glFrontFace(const GLenum& mode);




                void GL_RESTORE_DEFAULT_STATE_MACHINE(const unsigned int& windowWidth, const unsigned int& windowHeight);
                void GL_RESTORE_CURRENT_STATE_MACHINE();

                OpenGLState();
                OpenGLState(const unsigned int& windowWidth, const unsigned int& windowHeight);
                ~OpenGLState();
        };
    };
};

#endif