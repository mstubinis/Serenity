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
                struct TextureUnitState final {
                    GLuint targetTexture1D;
                    GLuint targetTexture2D;
                    GLuint targetTexture3D;
                    GLuint targetTextureCube;
                    TextureUnitState() {
                        targetTexture1D   = 999999;
                        targetTexture2D   = 999999;
                        targetTexture3D   = 999999;
                        targetTextureCube = 999999;
                    }
                };
                std::vector<TextureUnitState> textureUnits;
                GLuint                        currentTextureUnit;
                #pragma endregion

                #pragma region ClearColor
                struct ClearColorState final {
                    GLfloat r;
                    GLfloat g;
                    GLfloat b;
                    GLfloat a;
                    ClearColorState() {
                        r = 0.0f;
                        g = 0.0f;
                        b = 0.0f;
                        a = 0.0f;
                    }
                };
                ClearColorState clearColor;
                #pragma endregion

                #pragma region ClearDepth
                struct ClearDepthState final {
                    GLdouble depth;
                    GLfloat  depthf;
                    ClearDepthState() {
                        depth  = 1.0;
                        depthf = 1.0f;
                    }
                };
                ClearDepthState clearDepth;
                #pragma endregion

                #pragma region ClearStencil
                struct ClearStencilState final {
                    GLint stencil;
                    ClearStencilState() {
                        stencil = 0;
                    }
                };
                ClearStencilState clearStencil;
                #pragma endregion

                #pragma region StencilMask
                struct StencilMaskState final {
                    GLuint front_mask;
                    GLuint back_mask;
                    StencilMaskState() {
                        front_mask = 0xFFFFFFFF;
                        back_mask = 0xFFFFFFFF;
                    }
                };
                StencilMaskState stencilMask;
                #pragma endregion

                #pragma region StencilOp
                struct StencilOpState final {
                    GLenum sFail_front;
                    GLenum dpFail_front;
                    GLenum dpPass_front;

                    GLenum sFail_back;
                    GLenum dpFail_back;
                    GLenum dpPass_back;

                    StencilOpState() {
                        sFail_front  = GL_KEEP;
                        dpFail_front = GL_KEEP;
                        dpPass_front = GL_KEEP;

                        sFail_back   = GL_KEEP;
                        dpFail_back  = GL_KEEP;
                        dpPass_back  = GL_KEEP;
                    }
                };
                StencilOpState stencilOp;
                #pragma endregion

                #pragma region UseProgram
                struct UseProgramState final {
                    GLuint program;
                    UseProgramState() {
                        program = 0;
                    }
                };
                UseProgramState useProgram;
                #pragma endregion

                #pragma region VAO
                struct VertexArrayObjState final {
                    GLuint vao;
                    VertexArrayObjState() {
                        vao = 0;
                    }
                };
                VertexArrayObjState vaoState;
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

                #pragma region DepthFunc
                struct DepthFuncState final {
                    GLenum func;
                    DepthFuncState() {
                        func = GL_LESS;
                    }
                };
                DepthFuncState depthFuncState;
                #pragma endregion

                #pragma region PixelStorei
                struct PixelStoreiState final {
                    GLenum pack_alignment;
                    GLenum unpack_alignment;
                    PixelStoreiState() {
                        pack_alignment = 4;
                        unpack_alignment = 4;
                    }
                };
                PixelStoreiState pixelStoreiState;
                #pragma endregion

                #pragma region StencilFunc
                struct StencilFuncState final {
                    GLenum func_front;
                    GLint  ref_front;
                    GLuint mask_front;

                    GLenum func_back;
                    GLint  ref_back;
                    GLuint mask_back;
                    StencilFuncState() {
                        func_front = GL_ALWAYS;
                        ref_front = 0;
                        mask_front = 0xFFFFFFFF;

                        func_back = GL_ALWAYS;
                        ref_back = 0;
                        mask_back = 0xFFFFFFFF;
                    }
                };
                StencilFuncState stencilFuncState;
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
                const bool GL_glDepthFunc(const GLenum& func);
                const bool GL_glPixelStorei(const GLenum& pname, const GLint& param);
                const bool GL_glStencilFuncSeparate(const GLenum& face, const GLenum& func, const GLint& ref, const GLuint& mask);
                const bool GL_glStencilFunc(const GLenum& func, const GLint& ref, const GLuint& mask);


                void GL_RESTORE_DEFAULT_STATE_MACHINE(const unsigned int& windowWidth, const unsigned int& windowHeight);
                void GL_RESTORE_CURRENT_STATE_MACHINE();

                OpenGLState();
                OpenGLState(const unsigned int& windowWidth, const unsigned int& windowHeight);
                ~OpenGLState();
        };
    };
};

#endif