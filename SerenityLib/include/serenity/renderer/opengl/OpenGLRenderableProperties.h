#pragma once
#ifndef ENGINE_RENDERER_OPENGL_RENDERABLE_PROPERTIES_H
#define ENGINE_RENDERER_OPENGL_RENDERABLE_PROPERTIES_H

#include <serenity/system/TypeDefs.h>
#include <serenity/renderer/opengl/OpenGL.h>

//creates a tightly packed data object that can hash fast.
class OpenGLRenderableProperties final {
    private:
        //byte 1
        uint8_t  m_BlendFuncSrcRGB : 5;
        uint8_t  m_DepthFunc : 3;

        //byte 2
        uint8_t  m_BlendFuncDstRGB : 5;
        uint8_t  m_CullFace : 3;

        //byte 3
        uint8_t  m_BlendEquationRGB : 4;
        uint8_t  m_BlendEquationAlpha : 4;


        uint8_t  m_BlendFuncSrcAlpha : 5;
        uint8_t  m_FrontFace : 2;


        uint8_t  m_BlendFuncDstAlpha : 5;


        uint8_t  m_Bools : 5;
        /*
        bool m_DoCullFace  = true;
        bool m_DoStencil   = false;
        bool m_DoBlend     = true;
        bool m_DoDepthMask = true;
        bool m_DoDepthTest = true;
        */
    public:
        /*
        accepted parameter values : GL_CW, GL_CCW
        */
        inline void setFrontFace(GLuint frontFace) noexcept {
            m_FrontFace = frontFace - GL_CW;
        }
        inline GLuint getFrontFace() const noexcept {
            return m_FrontFace + GL_CW;
        }



        /*
        accepted parameter values : GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
        */
        inline void setCullFace(GLuint cullFaceMode) noexcept {
            m_CullFace = cullFaceMode - GL_FRONT;
        }
        inline GLuint getCullFace() const noexcept {
            return m_CullFace + GL_FRONT;
        }


        /*
        accepted parameter values : GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS
        */
        inline void setDepthFunc(GLuint func) noexcept {
            m_DepthFunc = func - GL_NEVER;
        }
        inline GLuint getDepthFunc() const noexcept {
            return m_DepthFunc + GL_NEVER;
        }


        /*
        accepted parameter values : GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX
        */
        void setBlendEquation(GLuint rgb, GLuint alpha) noexcept {
            m_BlendEquationRGB   = rgb   - GL_FUNC_ADD;
            m_BlendEquationAlpha = alpha - GL_FUNC_ADD;
        }
        std::pair<GLuint, GLuint> getBlendEquation() const noexcept {
            return { m_BlendEquationRGB + GL_FUNC_ADD , m_BlendEquationAlpha + GL_FUNC_ADD };
        }
        
        /*
        accepted parameter values : GL_ZERO GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR
                                    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR
                                    GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE
                                    GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_SRC1_ALPHA, GL_ONE_MINUS_SRC1_ALPHA
        */
        void setBlendFuncSeparate(GLuint srcRGB, GLuint dstRGB, GLuint srcAlpha, GLuint dstAlpha) noexcept {
            auto transform = [](GLuint input) {
                if (input >= GL_SRC_COLOR && input <= GL_SRC_ALPHA_SATURATE) {
                    return (input - GL_SRC_COLOR) + 2;
                } else if (input >= GL_CONSTANT_COLOR && input <= GL_ONE_MINUS_CONSTANT_ALPHA) {
                    return (input - GL_CONSTANT_COLOR) + 11;
                } else if (input >= GL_SRC1_COLOR && input <= GL_ONE_MINUS_SRC1_ALPHA) {
                    return (input - GL_SRC1_COLOR) + 15;
                } else if (input == GL_SRC1_ALPHA) {
                    return (input - GL_SRC1_ALPHA) + 18;
                }
                return input;
            };
            m_BlendFuncSrcRGB = transform(srcRGB);
            m_BlendFuncDstRGB = transform(dstRGB);
            m_BlendFuncSrcAlpha = transform(srcAlpha);
            m_BlendFuncDstAlpha = transform(dstAlpha);
        }
        std::tuple<GLuint, GLuint, GLuint, GLuint> getBlendFuncSeparate() const noexcept {
            auto transform = [](uint8_t input) -> GLuint {
                if (input >= 18) {
                    return (input - 18) + GL_SRC1_ALPHA;
                } else if (input >= 15) {
                    return (input - 15) + GL_SRC1_COLOR;
                } else if (input >= 11) {
                    return (input - 11) + GL_CONSTANT_COLOR;
                } else if (input >= 2) {
                    return (input - 2) + GL_SRC_COLOR;
                }
                return input;
            };
            return { transform(m_BlendFuncSrcRGB), transform(m_BlendFuncDstRGB), transform(m_BlendFuncSrcAlpha), transform(m_BlendFuncDstAlpha) };
        }

};

#endif