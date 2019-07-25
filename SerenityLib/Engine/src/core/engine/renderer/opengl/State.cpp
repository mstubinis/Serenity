#include <core/engine/renderer/opengl/State.h>

#include <glm/glm.hpp>

using namespace Engine;
using namespace Engine::epriv;

OpenGLState::OpenGLState() {

}
OpenGLState::OpenGLState(const unsigned int& windowWidth, const unsigned int& windowHeight) {
    viewportState = ViewportState(static_cast<GLsizei>(windowWidth), static_cast<GLsizei>(windowHeight));
}
OpenGLState::~OpenGLState() {
}

void OpenGLState::GL_INIT_DEFAULT_STATE_MACHINE(const unsigned int& windowWidth, const unsigned int& windowHeight) {
    GLint     int_value;
    GLfloat   float_value;
    GLboolean boolean_value;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &int_value); //what about GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS?
    textureUnits.reserve(int_value);
    for (unsigned int i = 0; i < textureUnits.capacity(); ++i)
        textureUnits.push_back(TextureUnitState());


    GL_RESTORE_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
}
void OpenGLState::GL_RESTORE_DEFAULT_STATE_MACHINE(const unsigned int& windowWidth, const unsigned int& windowHeight) {
    glewExperimental = GL_TRUE;
    glewInit(); glGetError();//stupid glew always inits an error. nothing we can do about it.

    currentTextureUnit = 0;

    //TODO: might need to gen and bind a dummy vao
    glActiveTexture(GL_TEXTURE0); //this was said to be needed for some drivers

    GL_glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GL_glClearDepth(1.0);
    GL_glClearDepthf(1.0f);
    GL_glClearStencil(0);
    GL_glStencilMask(0xFFFFFFFF);
    GL_glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    GL_glUseProgram(0);
    GL_glBindVertexArray(0);
    GL_glViewport(0, 0, static_cast<GLsizei>(windowWidth), static_cast<GLsizei>(windowHeight));
    GL_glCullFace(GL_BACK);
    GL_glFrontFace(GL_CCW);
    GL_glDepthFunc(GL_LESS);
    GL_glPixelStorei(GL_PACK_ALIGNMENT, 4);
    GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    GL_glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
}
void OpenGLState::GL_RESTORE_CURRENT_STATE_MACHINE() {
    glActiveTexture(currentTextureUnit);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClearDepth(clearDepth.depth);
    glClearDepthf(clearDepth.depthf);
    glClearStencil(clearStencil.stencil);
    glStencilMaskSeparate(GL_FRONT, stencilMask.front_mask);
    glStencilMaskSeparate(GL_BACK, stencilMask.back_mask);
    glStencilOpSeparate(GL_FRONT, stencilOp.sFail_front, stencilOp.dpFail_front, stencilOp.dpPass_front);
    glStencilOpSeparate(GL_FRONT, stencilOp.sFail_back, stencilOp.dpFail_back, stencilOp.dpPass_back);
    glUseProgram(useProgram.program);
    glBindVertexArray(vaoState.vao);
    glViewport(viewportState.x, viewportState.y, viewportState.width, viewportState.height);
    glCullFace(cullFaceState.mode);
    glFrontFace(frontFaceState.mode);
    glDepthFunc(depthFuncState.func);
    glPixelStorei(GL_PACK_ALIGNMENT, pixelStoreiState.pack_alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixelStoreiState.unpack_alignment);
    glStencilFuncSeparate(GL_FRONT, stencilFuncState.func_front, stencilFuncState.ref_front, stencilFuncState.mask_front);
    glStencilFuncSeparate(GL_BACK, stencilFuncState.func_back, stencilFuncState.ref_back, stencilFuncState.mask_back);
}
const bool OpenGLState::GL_glActiveTexture(const GLenum& textureUnit) {
    //if (textureUnit == 0 || currentTextureUnit != textureUnit) {
        //currentTextureUnit = textureUnit;
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        return true;
    //}
    //return false;
}

const bool OpenGLState::GL_glBindTextureForModification(const GLenum& textureTarget, const GLuint& textureObject) {
    glActiveTexture(GL_TEXTURE0 + textureUnits.capacity());
    glBindTexture(textureTarget, textureObject);
    return true;
}
const bool OpenGLState::GL_glBindTextureForRendering(const GLenum& textureTarget, const GLuint& textureObject) {
    auto& texUnit = textureUnits[currentTextureUnit];
    switch (textureTarget){
        case GL_TEXTURE_1D: {
            if (texUnit.targetTexture1D != textureObject) {
                glBindTexture(textureTarget, textureObject);
                texUnit.targetTexture1D = textureObject;
                return true;
            }
            break;
        }case GL_TEXTURE_2D: {
            if (texUnit.targetTexture2D != textureObject) {
                glBindTexture(textureTarget, textureObject);
                texUnit.targetTexture2D = textureObject;
                return true;
            }
            break;
        }case GL_TEXTURE_3D: {
            if (texUnit.targetTexture3D != textureObject) {
                glBindTexture(textureTarget, textureObject);
                texUnit.targetTexture3D = textureObject;
                return true;
            }
            break;
        }case GL_TEXTURE_CUBE_MAP: {
            if (texUnit.targetTextureCube != textureObject) {
                glBindTexture(textureTarget, textureObject);
                texUnit.targetTextureCube = textureObject;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
const bool OpenGLState::GL_glClearColor(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a) {
    //Values are clamped to the range [0,1]
    const GLfloat _r = glm::clamp(r, 0.0f, 1.0f);
    const GLfloat _g = glm::clamp(g, 0.0f, 1.0f);
    const GLfloat _b = glm::clamp(b, 0.0f, 1.0f);
    const GLfloat _a = glm::clamp(a, 0.0f, 1.0f);
    if (_r == clearColor.r && _g == clearColor.g && _b == clearColor.b && _a == clearColor.a) {
        return false;
    }
    glClearColor(_r, _g, _b, _a);
    clearColor.r = _r;
    clearColor.g = _g;
    clearColor.b = _b;
    clearColor.a = _a;
    return true;
}
const bool OpenGLState::GL_glClearDepth(const GLdouble& depth) {
    //Values are clamped to the range [0,1]
    const GLfloat d = glm::clamp(depth, 0.0, 1.0);
    if (clearDepth.depth == d) {
        return false;
    }
    clearDepth.depth = d;
    glClearDepth(d);
    return true;
}
const bool OpenGLState::GL_glClearDepthf(const GLfloat& depth_float) {
    //Values are clamped to the range [0,1]
    const GLfloat f = glm::clamp(depth_float, 0.0f, 1.0f);
    if (clearDepth.depthf == f) {
        return false;
    }
    clearDepth.depthf = f;
    glClearDepthf(f);
    return true;
}
const bool OpenGLState::GL_glClearStencil(const GLint& stencil) {
    if (clearStencil.stencil == stencil) {
        return false;
    }
    clearStencil.stencil = stencil;
    glClearStencil(stencil);
    return true;
}
const bool OpenGLState::GL_glStencilMaskSeparate(const GLenum& face, const GLuint& mask) {
    switch (face) {
        case GL_FRONT: {
            if (stencilMask.front_mask != mask) {
                glStencilMaskSeparate(GL_FRONT, mask);
                stencilMask.front_mask = mask;
                return true;
            }
            break;
        }
        case GL_BACK: {
            if (stencilMask.back_mask != mask) {
                glStencilMaskSeparate(GL_BACK, mask);
                stencilMask.back_mask = mask;
                return true;
            }
            break;
        }
        case GL_FRONT_AND_BACK: {
            if (stencilMask.back_mask != mask || stencilMask.front_mask != mask) {
                glStencilMaskSeparate(GL_FRONT_AND_BACK, mask);
                stencilMask.front_mask = mask;
                stencilMask.back_mask = mask;
                return true;
            }
            break;
        }
        default: {
            break;
        }
    }
    return false;
}
const bool OpenGLState::GL_glStencilMask(const GLuint& mask) {
    if (stencilMask.back_mask != mask || stencilMask.front_mask != mask) {
        glStencilMask(mask);
        stencilMask.front_mask = mask;
        stencilMask.back_mask = mask;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glStencilOp(const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass) {
    if (stencilOp.sFail_back != sfail || stencilOp.sFail_front != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpPass_front != dppass || stencilOp.dpFail_back != dpfail || stencilOp.dpFail_front != dpfail) {
        glStencilOp(sfail, dpfail, dppass);
        stencilOp.dpFail_back = dpfail;
        stencilOp.dpPass_back = dppass;
        stencilOp.sFail_back = sfail;
        stencilOp.dpFail_front = dpfail;
        stencilOp.dpPass_front = dppass;
        stencilOp.sFail_front = sfail;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glStencilOpSeparate(const GLenum& face, const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass) {
    switch (face) {
        case GL_FRONT: {
            if (stencilOp.sFail_front != sfail || stencilOp.dpPass_front != dppass || stencilOp.dpFail_front != dpfail) {
                glStencilOpSeparate(GL_FRONT, sfail, dpfail, dppass);
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }
        case GL_BACK: {
            if (stencilOp.sFail_back != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpFail_back != dpfail) {
                glStencilOpSeparate(GL_BACK, sfail, dpfail, dppass);
                stencilOp.dpFail_back = dpfail;
                stencilOp.dpPass_back = dppass;
                stencilOp.sFail_back = sfail;
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }
        case GL_FRONT_AND_BACK: {
            if (stencilOp.sFail_back != sfail || stencilOp.sFail_front != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpPass_front != dppass || stencilOp.dpFail_back != dpfail || stencilOp.dpFail_front != dpfail) {
                glStencilOpSeparate(GL_FRONT_AND_BACK, sfail, dpfail, dppass);
                stencilOp.dpFail_back = dpfail;
                stencilOp.dpPass_back = dppass;
                stencilOp.sFail_back = sfail;
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }
        default: {
            break;
        }
    }
    return false;
}
const bool OpenGLState::GL_glUseProgram(const GLuint& program) {
    if (useProgram.program != program) {
        glUseProgram(program);
        useProgram.program = program;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glBindVertexArray(const GLuint& vao) {
    if (vaoState.vao != vao) {
        glBindVertexArray(vao);
        vaoState.vao = vao;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glViewport(const GLint& x, const GLint& y, const GLsizei& width, const GLsizei& height) {
    // Viewport width and height are silently clamped to a range that depends on the implementation.To query this range, call glGet with argument GL_MAX_VIEWPORT_DIMS.
    if (x == viewportState.x && y == viewportState.y && width == viewportState.width && height == viewportState.height) {
        return false;
    }
    glViewport(x, y, width, height);
    viewportState.x = x;
    viewportState.y = y;
    viewportState.width = width;
    viewportState.height = height;
    return true;
}
const bool OpenGLState::GL_glCullFace(const GLenum& mode) {
    if (cullFaceState.mode != mode) {
        glCullFace(mode);
        cullFaceState.mode = mode;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glFrontFace(const GLenum& mode) {
    if (frontFaceState.mode != mode) {
        glFrontFace(mode);
        frontFaceState.mode = mode;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glDepthFunc(const GLenum& func) {
    if (depthFuncState.func != func) {
        glDepthFunc(func);
        depthFuncState.func = func;
        return true;
    }
    return false;
}
const bool OpenGLState::GL_glPixelStorei(const GLenum& pname, const GLint& param) {
    if (param != 1 && param != 2 && param != 4 && param != 8)
        return false;
    switch (pname) {
        case GL_PACK_ALIGNMENT: {
            if (pixelStoreiState.pack_alignment != param) {
                glPixelStorei(GL_PACK_ALIGNMENT, param);
                pixelStoreiState.pack_alignment = param;
                return true;
            }
            break;
        }case GL_UNPACK_ALIGNMENT: {
            if (pixelStoreiState.unpack_alignment != param) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, param);
                pixelStoreiState.unpack_alignment = param;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
const bool OpenGLState::GL_glStencilFuncSeparate(const GLenum& face, const GLenum& func, const GLint& ref, const GLuint& mask) {
    switch (face) {
        case GL_FRONT: {
            if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask) {
                glStencilFuncSeparate(face, func, ref, mask);
                stencilFuncState.func_front = func;
                stencilFuncState.ref_front = ref;
                stencilFuncState.mask_front = mask;
                return true;
            }
            break;
        }
        case GL_BACK: {
            if (stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
                glStencilFuncSeparate(face, func, ref, mask);
                stencilFuncState.func_back = func;
                stencilFuncState.ref_back = ref;
                stencilFuncState.mask_back = mask;
                return true;
            }
            break;
        }
        case GL_FRONT_AND_BACK: {
            if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask || stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
                glStencilFuncSeparate(face, func, ref, mask);
                stencilFuncState.func_back = func;
                stencilFuncState.ref_back = ref;
                stencilFuncState.mask_back = mask;
                stencilFuncState.func_front = func;
                stencilFuncState.ref_front = ref;
                stencilFuncState.mask_front = mask;
                return true;
            }
            break;
        }
        default: {
            break;
        }
    }
    return false;
}
const bool OpenGLState::GL_glStencilFunc(const GLenum& func, const GLint& ref, const GLuint& mask) {
    if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask || stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
        glStencilFunc(func, ref, mask);
        stencilFuncState.func_back = func;
        stencilFuncState.ref_back = ref;
        stencilFuncState.mask_back = mask;
        stencilFuncState.func_front = func;
        stencilFuncState.ref_front = ref;
        stencilFuncState.mask_front = mask;
        return true;
    }
    return false;
}