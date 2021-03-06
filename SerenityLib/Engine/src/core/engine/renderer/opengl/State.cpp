#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/opengl/State.h>
#include <core/engine/renderer/opengl/OpenGL.h>

using namespace Engine;
using namespace Engine::priv;

uint32_t OpenGLState::MAX_TEXTURE_UNITS           = 0;
float    OpenGLState::MAX_TEXTURE_MAX_ANISOTROPY  = 1.0f;

void OpenGLState::GL_INIT_DEFAULT_STATE_MACHINE(uint32_t windowWidth, uint32_t windowHeight) {
    GLint    int_value;
    GLfloat  float_value;
    //GLboolean boolean_value;

    viewportState = ViewportState((GLsizei)windowWidth, (GLsizei)windowHeight);

    GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &int_value)); //what about GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS?
    MAX_TEXTURE_UNITS = glm::max(MAX_TEXTURE_UNITS, (uint32_t)int_value);

    GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &float_value));
    MAX_TEXTURE_MAX_ANISOTROPY = float_value;


    textureUnits.reserve(MAX_TEXTURE_UNITS);
    for (size_t i = 0; i < textureUnits.capacity(); ++i) {
        textureUnits.emplace_back();
    }

    GLCall(glGetIntegerv(GL_MAX_DRAW_BUFFERS, &int_value));
    enabledState.blendState.reserve((size_t)int_value);
    blendEquationState.reserve((size_t)int_value);
    for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
        enabledState.blendState.emplace_back();
    }
    for (size_t i = 0; i < blendEquationState.capacity(); ++i) {
        blendEquationState.emplace_back();
    }

    GLCall(glGetIntegerv(GL_MAX_VIEWPORTS, &int_value));
    enabledState.scissorState.reserve((size_t)int_value);
    for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
        enabledState.scissorState.emplace_back();
    }
    GL_RESTORE_DEFAULT_STATE_MACHINE(windowWidth, windowHeight);
}
void OpenGLState::GL_RESTORE_DEFAULT_STATE_MACHINE(uint32_t windowWidth, uint32_t windowHeight) {
    glewExperimental = GL_TRUE;
    glewInit(); 
    glGetError();//stupid glew always inits an error. nothing we can do about it.

    currentActiveTextureUnit = 0;

    //TODO: might need to gen and bind a dummy vao
    GLCall(glActiveTexture(GL_TEXTURE0)); //this was said to be needed for some drivers

    GL_glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GL_glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    GL_glClearDepth(1.0);
    GL_glClearDepthf(1.0f);
    GL_glClearStencil(0);
    GL_glStencilMask(0xFFFFFFFF);
    GL_glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    GL_glUseProgram(0);
    GL_glBindVertexArray(0);
    GL_glViewport(0, 0, (GLsizei)windowWidth, (GLsizei)windowHeight);
    GL_glCullFace(GL_BACK);
    GL_glFrontFace(GL_CCW);
    GL_glDepthFunc(GL_LESS);
    GL_glPixelStorei(GL_PACK_ALIGNMENT, 4);
    GL_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    GL_glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);

    //glenable stuff
    for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
        GL_glDisablei(GL_BLEND, (GLuint)i);
    }
    for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
        GL_glDisablei(GL_SCISSOR_TEST, (GLuint)i);
    }
    GL_glEnable(GL_MULTISAMPLE);
    GL_glEnable(GL_DITHER);

    GL_glDisable(GL_CLIP_DISTANCE1);
    GL_glDisable(GL_CLIP_DISTANCE2);
    GL_glDisable(GL_CLIP_DISTANCE3);
    GL_glDisable(GL_CLIP_DISTANCE4);
    GL_glDisable(GL_CLIP_DISTANCE5);
    GL_glDisable(GL_COLOR_LOGIC_OP);
    GL_glDisable(GL_CULL_FACE);
    GL_glDisable(GL_DEBUG_OUTPUT);
    GL_glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    GL_glDisable(GL_DEPTH_CLAMP);
    GL_glDisable(GL_DEPTH_TEST);
    GL_glDisable(GL_FRAMEBUFFER_SRGB);
    GL_glDisable(GL_LINE_SMOOTH);
    GL_glDisable(GL_POLYGON_OFFSET_FILL);
    GL_glDisable(GL_POLYGON_OFFSET_LINE);
    GL_glDisable(GL_POLYGON_OFFSET_POINT);
    GL_glDisable(GL_POLYGON_SMOOTH);
    GL_glDisable(GL_PRIMITIVE_RESTART);
    GL_glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    GL_glDisable(GL_RASTERIZER_DISCARD);
    GL_glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    GL_glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    GL_glDisable(GL_SAMPLE_COVERAGE);
    GL_glDisable(GL_SAMPLE_SHADING);
    GL_glDisable(GL_SAMPLE_MASK);
    GL_glDisable(GL_STENCIL_TEST);
    GL_glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    GL_glDisable(GL_PROGRAM_POINT_SIZE);

    GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GL_glBindRenderbuffer(0);

    for (size_t i = 0; i < blendEquationState.capacity(); ++i) {
        GL_glBlendEquationi((GLuint)i, GL_FUNC_ADD);
    }
}
void OpenGLState::GL_RESTORE_CURRENT_STATE_MACHINE() {
    GLCall(glActiveTexture(currentActiveTextureUnit));
    GLCall(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
    GLCall(glColorMask(colorMaskState.r, colorMaskState.g, colorMaskState.b, colorMaskState.a));
    GLCall(glClearDepth(clearDepth.depth));
    GLCall(glClearDepthf(clearDepth.depthf));
    GLCall(glClearStencil(clearStencil.stencil));
    GLCall(glStencilMaskSeparate(GL_FRONT, stencilMask.front_mask));
    GLCall(glStencilMaskSeparate(GL_BACK, stencilMask.back_mask));
    GLCall(glStencilOpSeparate(GL_FRONT, stencilOp.sFail_front, stencilOp.dpFail_front, stencilOp.dpPass_front));
    GLCall(glStencilOpSeparate(GL_FRONT, stencilOp.sFail_back, stencilOp.dpFail_back, stencilOp.dpPass_back));
    GLCall(glUseProgram(useProgram.program));
    GLCall(glBindVertexArray(vaoState.vao));
    GLCall(glViewport(viewportState.x, viewportState.y, viewportState.width, viewportState.height));
    GLCall(glCullFace(cullFaceState.mode));
    GLCall(glFrontFace(frontFaceState.mode));
    GLCall(glDepthFunc(depthFuncState.func));
    GLCall(glPixelStorei(GL_PACK_ALIGNMENT, pixelStoreiState.pack_alignment));
    GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, pixelStoreiState.unpack_alignment));
    GLCall(glStencilFuncSeparate(GL_FRONT, stencilFuncState.func_front, stencilFuncState.ref_front, stencilFuncState.mask_front));
    GLCall(glStencilFuncSeparate(GL_BACK, stencilFuncState.func_back, stencilFuncState.ref_back, stencilFuncState.mask_back));

    //glenable stuff
    for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
        auto& blend = enabledState.blendState[i];
        (blend.gl_blend == GL_TRUE) ? glEnablei(GL_BLEND, (GLuint)i) : glDisablei(GL_BLEND, (GLuint)i);
    }
    for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
        auto& scissor = enabledState.scissorState[i];
        (scissor.gl_scissor_test == GL_TRUE) ? glEnablei(GL_SCISSOR_TEST, (GLuint)i) : glDisablei(GL_SCISSOR_TEST, (GLuint)i);
    }
    enabledState.gl_multisample == GL_TRUE ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
    enabledState.gl_dither == GL_TRUE ? glEnable(GL_DITHER) : glDisable(GL_DITHER);
    enabledState.gl_clip_distance_1 == GL_TRUE ? glEnable(GL_CLIP_DISTANCE1) : glDisable(GL_CLIP_DISTANCE1);
    enabledState.gl_clip_distance_2 == GL_TRUE ? glEnable(GL_CLIP_DISTANCE2) : glDisable(GL_CLIP_DISTANCE2);
    enabledState.gl_clip_distance_3 == GL_TRUE ? glEnable(GL_CLIP_DISTANCE3) : glDisable(GL_CLIP_DISTANCE3);
    enabledState.gl_clip_distance_4 == GL_TRUE ? glEnable(GL_CLIP_DISTANCE4) : glDisable(GL_CLIP_DISTANCE4);
    enabledState.gl_clip_distance_5 == GL_TRUE ? glEnable(GL_CLIP_DISTANCE5) : glDisable(GL_CLIP_DISTANCE5);
    enabledState.gl_color_logic_op == GL_TRUE ? glEnable(GL_COLOR_LOGIC_OP) : glDisable(GL_COLOR_LOGIC_OP);
    enabledState.gl_cull_face == GL_TRUE ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    enabledState.gl_debug_output == GL_TRUE ? glEnable(GL_DEBUG_OUTPUT) : glDisable(GL_DEBUG_OUTPUT);
    enabledState.gl_debug_output_syncronous == GL_TRUE ? glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS) : glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    enabledState.gl_depth_clamp == GL_TRUE ? glEnable(GL_DEPTH_CLAMP) : glDisable(GL_DEPTH_CLAMP);
    enabledState.gl_depth_test == GL_TRUE ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    enabledState.gl_framebuffer_srgb == GL_TRUE ? glEnable(GL_FRAMEBUFFER_SRGB) : glDisable(GL_FRAMEBUFFER_SRGB);
    enabledState.gl_line_smooth == GL_TRUE ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH);
    enabledState.gl_polygon_offset_fill == GL_TRUE ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL);
    enabledState.gl_polygon_offset_line == GL_TRUE ? glEnable(GL_POLYGON_OFFSET_LINE) : glDisable(GL_POLYGON_OFFSET_LINE);
    enabledState.gl_polygon_offset_point == GL_TRUE ? glEnable(GL_POLYGON_OFFSET_POINT) : glDisable(GL_POLYGON_OFFSET_POINT);
    enabledState.gl_polygon_smooth == GL_TRUE ? glEnable(GL_POLYGON_SMOOTH) : glDisable(GL_POLYGON_SMOOTH);
    enabledState.gl_primitive_restart == GL_TRUE ? glEnable(GL_PRIMITIVE_RESTART) : glDisable(GL_PRIMITIVE_RESTART);
    enabledState.gl_primitive_restart_fixed_index == GL_TRUE ? glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX) : glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    enabledState.gl_rasterizer_discard == GL_TRUE ? glEnable(GL_RASTERIZER_DISCARD) : glDisable(GL_RASTERIZER_DISCARD);
    enabledState.gl_sample_alpha_to_coverage == GL_TRUE ? glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE) : glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    enabledState.gl_sample_alpha_to_one == GL_TRUE ? glEnable(GL_SAMPLE_ALPHA_TO_ONE) : glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    enabledState.gl_sample_coverage == GL_TRUE ? glEnable(GL_SAMPLE_COVERAGE) : glDisable(GL_SAMPLE_COVERAGE);
    enabledState.gl_sample_shading == GL_TRUE ? glEnable(GL_SAMPLE_SHADING) : glDisable(GL_SAMPLE_SHADING);
    enabledState.gl_sample_mask == GL_TRUE ? glEnable(GL_SAMPLE_MASK) : glDisable(GL_SAMPLE_MASK);
    enabledState.gl_stencil_test == GL_TRUE ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
    enabledState.gl_texture_cube_map_seamless == GL_TRUE ? glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS) : glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    enabledState.gl_program_point_size == GL_TRUE ? glEnable(GL_PROGRAM_POINT_SIZE) : glDisable(GL_PROGRAM_POINT_SIZE);

    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferState.framebuffer_read));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferState.framebuffer_draw));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, framebufferState.renderbuffer));

    for (size_t i = 0; i < blendEquationState.capacity(); ++i) {
        GLCall(glBlendEquationi((GLuint)i, blendEquationState[i].mode));
    }
}
bool OpenGLState::GL_glActiveTexture(GLenum textureUnit) {
    currentActiveTextureUnit = textureUnit;
    currentActiveTextureUnit = std::min(currentActiveTextureUnit, (GLuint)(textureUnits.capacity() - 1U));
    GLCall(glActiveTexture(GL_TEXTURE0 + currentActiveTextureUnit));
    return true;
}
bool OpenGLState::GL_glUnbindTexture(GLenum textureUnit, GLenum textureTarget) {
    GL_glActiveTexture(textureUnit);
    GL_glBindTextureForRendering(textureTarget, 0);
    return true;
}
bool OpenGLState::GL_glBindTextureForModification(GLenum textureTarget, GLuint textureObject) {
    GL_glActiveTexture((GLenum)(textureUnits.capacity() - 1U));
    GL_glBindTextureForRendering(textureTarget, textureObject);
    return true;
}
uint32_t OpenGLState::internal_get_enum_index_from_gl_texture_type(GLenum textureType) noexcept {
    switch (textureType) {
        case GL_TEXTURE_1D: {
            return (uint32_t)TextureType::Texture1D;
        }case GL_TEXTURE_2D: {
            return (uint32_t)TextureType::Texture2D;
        }case GL_TEXTURE_3D: {
            return (uint32_t)TextureType::Texture3D;
        }case GL_TEXTURE_CUBE_MAP: {
            return (uint32_t)TextureType::CubeMap;
        }
    }
    return 0;
}
GLuint OpenGLState::getCurrentlyBoundTextureOfType(GLenum textureType) noexcept {
    uint32_t index = internal_get_enum_index_from_gl_texture_type(textureType);
    return textureUnits[currentActiveTextureUnit].openglIDs[index];
}
bool OpenGLState::GL_glBindTextureForRendering(GLenum textureTarget, GLuint textureObject) {
    auto& currentBoundUnit            = textureUnits[currentActiveTextureUnit];
    uint32_t index                    = internal_get_enum_index_from_gl_texture_type(textureTarget);
    currentBoundUnit.openglIDs[index] = textureObject;    
    if (index > 0) {
        GLCall(glBindTexture(textureTarget, textureObject));
    }
    return (bool)index;
}
bool OpenGLState::GL_glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    //Values are clamped to the range [0,1]
    r = glm::clamp(r, 0.0f, 1.0f);
    g = glm::clamp(g, 0.0f, 1.0f);
    b = glm::clamp(b, 0.0f, 1.0f);
    a = glm::clamp(a, 0.0f, 1.0f);
    if (r == clearColor.r && g == clearColor.g && b == clearColor.b && a == clearColor.a) {
        return false;
    }
    GLCall(glClearColor(r, g, b, a));
    clearColor.r = r;
    clearColor.g = g;
    clearColor.b = b;
    clearColor.a = a;
    return true;
}
bool OpenGLState::GL_glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    if (r == colorMaskState.r && g == colorMaskState.g && b == colorMaskState.b && a == colorMaskState.a)
        return false;
    GLCall(glColorMask(r, g, b, a));
    colorMaskState.r = r;
    colorMaskState.g = g;
    colorMaskState.b = b;
    colorMaskState.a = a;
    return true;
}
bool OpenGLState::GL_glClearDepth(GLdouble depth) {
    //Values are clamped to the range [0,1]
    depth = glm::clamp(depth, 0.0, 1.0);
    if (clearDepth.depth == depth) {
        return false;
    }
    clearDepth.depth = depth;
    GLCall(glClearDepth(depth));
    return true;
}
bool OpenGLState::GL_glClearDepthf(GLfloat depth_float) {
    //Values are clamped to the range [0,1]
    depth_float = glm::clamp(depth_float, 0.0f, 1.0f);
    if (clearDepth.depthf == depth_float) {
        return false;
    }
    clearDepth.depthf = depth_float;
    GLCall(glClearDepthf(depth_float));
    return true;
}
bool OpenGLState::GL_glClearStencil(GLint stencil) {
    if (clearStencil.stencil == stencil) {
        return false;
    }
    clearStencil.stencil = stencil;
    GLCall(glClearStencil(stencil));
    return true;
}
bool OpenGLState::GL_glStencilMaskSeparate(GLenum face, GLuint mask) {
    switch (face) {
        case GL_FRONT: {
            if (stencilMask.front_mask != mask) {
                GLCall(glStencilMaskSeparate(GL_FRONT, mask));
                stencilMask.front_mask = mask;
                return true;
            }
            break;
        }
        case GL_BACK: {
            if (stencilMask.back_mask != mask) {
                GLCall(glStencilMaskSeparate(GL_BACK, mask));
                stencilMask.back_mask = mask;
                return true;
            }
            break;
        }
        case GL_FRONT_AND_BACK: {
            if (stencilMask.back_mask != mask || stencilMask.front_mask != mask) {
                GLCall(glStencilMaskSeparate(GL_FRONT_AND_BACK, mask));
                stencilMask.front_mask = mask;
                stencilMask.back_mask = mask;
                return true;
            }
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glStencilMask(GLuint mask) {
    if (stencilMask.back_mask != mask || stencilMask.front_mask != mask) {
        GLCall(glStencilMask(mask));
        stencilMask.front_mask = mask;
        stencilMask.back_mask = mask;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    if (stencilOp.sFail_back != sfail || stencilOp.sFail_front != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpPass_front != dppass || stencilOp.dpFail_back != dpfail || stencilOp.dpFail_front != dpfail) {
        GLCall(glStencilOp(sfail, dpfail, dppass));
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
bool OpenGLState::GL_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    switch (face) {
        case GL_FRONT: {
            if (stencilOp.sFail_front != sfail || stencilOp.dpPass_front != dppass || stencilOp.dpFail_front != dpfail) {
                GLCall(glStencilOpSeparate(GL_FRONT, sfail, dpfail, dppass));
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }case GL_BACK: {
            if (stencilOp.sFail_back != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpFail_back != dpfail) {
                GLCall(glStencilOpSeparate(GL_BACK, sfail, dpfail, dppass));
                stencilOp.dpFail_back = dpfail;
                stencilOp.dpPass_back = dppass;
                stencilOp.sFail_back = sfail;
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }case GL_FRONT_AND_BACK: {
            if (stencilOp.sFail_back != sfail || stencilOp.sFail_front != sfail || stencilOp.dpPass_back != dppass || stencilOp.dpPass_front != dppass || stencilOp.dpFail_back != dpfail || stencilOp.dpFail_front != dpfail) {
                GLCall(glStencilOpSeparate(GL_FRONT_AND_BACK, sfail, dpfail, dppass));
                stencilOp.dpFail_back = dpfail;
                stencilOp.dpPass_back = dppass;
                stencilOp.sFail_back = sfail;
                stencilOp.dpFail_front = dpfail;
                stencilOp.dpPass_front = dppass;
                stencilOp.sFail_front = sfail;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glUseProgram(GLuint program) {
    if (useProgram.program != program) {
        GLCall(glUseProgram(program));
        useProgram.program = program;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glBindVertexArray(GLuint vao) {
    if (vaoState.vao != vao) {
        GLCall(glBindVertexArray(vao));
        vaoState.vao = vao;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    // Viewport width and height are silently clamped to a range that depends on the implementation.To query this range, call glGet with argument GL_MAX_VIEWPORT_DIMS.
    if (x == viewportState.x && y == viewportState.y && width == viewportState.width && height == viewportState.height) {
        return false;
    }
    GLCall(glViewport(x, y, width, height));
    viewportState.x = x;
    viewportState.y = y;
    viewportState.width = width;
    viewportState.height = height;
    return true;
}
bool OpenGLState::GL_glCullFace(GLenum mode) {
    if (cullFaceState.mode != mode) {
        GLCall(glCullFace(mode));
        cullFaceState.mode = mode;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glFrontFace(GLenum mode) {
    if (frontFaceState.mode != mode) {
        GLCall(glFrontFace(mode));
        frontFaceState.mode = mode;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glDepthFunc(GLenum func) {
    if (depthFuncState.func != func) {
        GLCall(glDepthFunc(func));
        depthFuncState.func = func;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glPixelStorei(GLenum pname, GLint param) {
    if (param != 1 && param != 2 && param != 4 && param != 8) {
        return false;
    }
    switch (pname) {
        case GL_PACK_ALIGNMENT: {
            if (pixelStoreiState.pack_alignment != param) {
                GLCall(glPixelStorei(GL_PACK_ALIGNMENT, param));
                pixelStoreiState.pack_alignment = param;
                return true;
            }
            break;
        }case GL_UNPACK_ALIGNMENT: {
            if (pixelStoreiState.unpack_alignment != param) {
                GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, param));
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
bool OpenGLState::GL_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    switch (face) {
        case GL_FRONT: {
            if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask) {
                GLCall(glStencilFuncSeparate(face, func, ref, mask));
                stencilFuncState.func_front = func;
                stencilFuncState.ref_front = ref;
                stencilFuncState.mask_front = mask;
                return true;
            }
            break;
        }case GL_BACK: {
            if (stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
                GLCall(glStencilFuncSeparate(face, func, ref, mask));
                stencilFuncState.func_back = func;
                stencilFuncState.ref_back = ref;
                stencilFuncState.mask_back = mask;
                return true;
            }
            break;
        }case GL_FRONT_AND_BACK: {
            if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask || stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
                GLCall(glStencilFuncSeparate(face, func, ref, mask));
                stencilFuncState.func_back = func;
                stencilFuncState.ref_back = ref;
                stencilFuncState.mask_back = mask;
                stencilFuncState.func_front = func;
                stencilFuncState.ref_front = ref;
                stencilFuncState.mask_front = mask;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    if (stencilFuncState.func_front != func || stencilFuncState.ref_front != ref || stencilFuncState.mask_front != mask || stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
        GLCall(glStencilFunc(func, ref, mask));
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
bool OpenGLState::GL_glEnable(GLenum capability) {
    auto boolean = GL_TRUE;
    switch (capability) {
        case GL_MULTISAMPLE: {
            if (enabledState.gl_multisample != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_multisample = boolean;
                return true;
            }
            break;
        }case GL_DITHER: {
            if (enabledState.gl_dither != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_dither = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE1: {
            if (enabledState.gl_clip_distance_1 != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_clip_distance_1 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE2: {
            if (enabledState.gl_clip_distance_2 != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_clip_distance_2 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE3: {
            if (enabledState.gl_clip_distance_3 != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_clip_distance_3 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE4: {
            if (enabledState.gl_clip_distance_4 != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_clip_distance_4 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE5: {
            if (enabledState.gl_clip_distance_5 != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_clip_distance_5 = boolean;
                return true;
            }
            break;
        }case GL_COLOR_LOGIC_OP: {
            if (enabledState.gl_color_logic_op != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_color_logic_op = boolean;
                return true;
            }
            break;
        }case GL_CULL_FACE: {
            if (enabledState.gl_cull_face != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_cull_face = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT: {
            if (enabledState.gl_debug_output != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_debug_output = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT_SYNCHRONOUS: {
            if (enabledState.gl_debug_output_syncronous != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_debug_output_syncronous = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_CLAMP: {
            if (enabledState.gl_depth_clamp != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_depth_clamp = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_TEST: {
            if (enabledState.gl_depth_test != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_depth_test = boolean;
                return true;
            }
            break;
        }case GL_FRAMEBUFFER_SRGB: {
            if (enabledState.gl_framebuffer_srgb != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_framebuffer_srgb = boolean;
                return true;
            }
            break;
        }case GL_LINE_SMOOTH: {
            if (enabledState.gl_line_smooth != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_line_smooth = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_FILL: {
            if (enabledState.gl_polygon_offset_fill != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_polygon_offset_fill = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_LINE: {
            if (enabledState.gl_polygon_offset_line != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_polygon_offset_line = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_POINT: {
            if (enabledState.gl_polygon_offset_point != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_polygon_offset_point = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_SMOOTH: {
            if (enabledState.gl_polygon_smooth != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_polygon_smooth = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART: {
            if (enabledState.gl_primitive_restart != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_primitive_restart = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART_FIXED_INDEX: {
            if (enabledState.gl_primitive_restart_fixed_index != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_primitive_restart_fixed_index = boolean;
                return true;
            }
            break;
        }case GL_RASTERIZER_DISCARD: {
            if (enabledState.gl_rasterizer_discard != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_rasterizer_discard = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_COVERAGE: {
            if (enabledState.gl_sample_alpha_to_coverage != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_sample_alpha_to_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_ONE: {
            if (enabledState.gl_sample_alpha_to_one != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_sample_alpha_to_one = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_COVERAGE: {
            if (enabledState.gl_sample_coverage != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_sample_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_SHADING: {
            if (enabledState.gl_sample_shading != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_sample_shading = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_MASK: {
            if (enabledState.gl_sample_mask != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_sample_mask = boolean;
                return true;
            }
            break;
        }case GL_STENCIL_TEST: {
            if (enabledState.gl_stencil_test != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_stencil_test = boolean;
                return true;
            }
            break;
        }case GL_TEXTURE_CUBE_MAP_SEAMLESS: {
            if (enabledState.gl_texture_cube_map_seamless != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_texture_cube_map_seamless = boolean;
                return true;
            }
            break;
        }case GL_PROGRAM_POINT_SIZE: {
            if (enabledState.gl_program_point_size != boolean) {
                GLCall(glEnable(capability));
                enabledState.gl_program_point_size = boolean;
                return true;
            }
            break;
        }case GL_BLEND: {
            for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
                auto& blend = enabledState.blendState[i];
                if (blend.gl_blend != boolean) {
                    GLCall(glEnablei(capability, (GLuint)i));
                    blend.gl_blend = boolean;
                }
            }
            return true;
        }case GL_SCISSOR_TEST: {
            for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
                auto& scissor = enabledState.scissorState[i];
                if (scissor.gl_scissor_test != boolean) {
                    GLCall(glEnablei(capability, (GLuint)i));
                    scissor.gl_scissor_test = boolean;
                }
            }
            return true;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glDisable(GLenum capability) {
    auto boolean = GL_FALSE;
    switch (capability) {
        case GL_MULTISAMPLE: {
            if (enabledState.gl_multisample != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_multisample = boolean;
                return true;
            }
            break;
        }case GL_DITHER: {
            if (enabledState.gl_dither != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_dither = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE1: {
            if (enabledState.gl_clip_distance_1 != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_clip_distance_1 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE2: {
            if (enabledState.gl_clip_distance_2 != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_clip_distance_2 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE3: {
            if (enabledState.gl_clip_distance_3 != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_clip_distance_3 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE4: {
            if (enabledState.gl_clip_distance_4 != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_clip_distance_4 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE5: {
            if (enabledState.gl_clip_distance_5 != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_clip_distance_5 = boolean;
                return true;
            }
            break;
        }case GL_COLOR_LOGIC_OP: {
            if (enabledState.gl_color_logic_op != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_color_logic_op = boolean;
                return true;
            }
            break;
        }case GL_CULL_FACE: {
            if (enabledState.gl_cull_face != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_cull_face = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT: {
            if (enabledState.gl_debug_output != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_debug_output = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT_SYNCHRONOUS: {
            if (enabledState.gl_debug_output_syncronous != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_debug_output_syncronous = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_CLAMP: {
            if (enabledState.gl_depth_clamp != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_depth_clamp = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_TEST: {
            if (enabledState.gl_depth_test != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_depth_test = boolean;
                return true;
            }
            break;
        }case GL_FRAMEBUFFER_SRGB: {
            if (enabledState.gl_framebuffer_srgb != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_framebuffer_srgb = boolean;
                return true;
            }
            break;
        }case GL_LINE_SMOOTH: {
            if (enabledState.gl_line_smooth != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_line_smooth = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_FILL: {
            if (enabledState.gl_polygon_offset_fill != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_polygon_offset_fill = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_LINE: {
            if (enabledState.gl_polygon_offset_line != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_polygon_offset_line = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_POINT: {
            if (enabledState.gl_polygon_offset_point != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_polygon_offset_point = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_SMOOTH: {
            if (enabledState.gl_polygon_smooth != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_polygon_smooth = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART: {
            if (enabledState.gl_primitive_restart != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_primitive_restart = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART_FIXED_INDEX: {
            if (enabledState.gl_primitive_restart_fixed_index != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_primitive_restart_fixed_index = boolean;
                return true;
            }
            break;
        }case GL_RASTERIZER_DISCARD: {
            if (enabledState.gl_rasterizer_discard != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_rasterizer_discard = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_COVERAGE: {
            if (enabledState.gl_sample_alpha_to_coverage != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_sample_alpha_to_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_ONE: {
            if (enabledState.gl_sample_alpha_to_one != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_sample_alpha_to_one = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_COVERAGE: {
            if (enabledState.gl_sample_coverage != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_sample_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_SHADING: {
            if (enabledState.gl_sample_shading != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_sample_shading = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_MASK: {
            if (enabledState.gl_sample_mask != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_sample_mask = boolean;
                return true;
            }
            break;
        }case GL_STENCIL_TEST: {
            if (enabledState.gl_stencil_test != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_stencil_test = boolean;
                return true;
            }
            break;
        }case GL_TEXTURE_CUBE_MAP_SEAMLESS: {
            if (enabledState.gl_texture_cube_map_seamless != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_texture_cube_map_seamless = boolean;
                return true;
            }
            break;
        }case GL_PROGRAM_POINT_SIZE: {
            if (enabledState.gl_program_point_size != boolean) {
                GLCall(glDisable(capability));
                enabledState.gl_program_point_size = boolean;
                return true;
            }
            break;
        }case GL_BLEND: {
            for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
                auto& blend = enabledState.blendState[i];
                if (blend.gl_blend != boolean) {
                    GLCall(glDisablei(capability, (GLuint)i));
                    blend.gl_blend = boolean;
                }
            }
            return true;
            break;
        }case GL_SCISSOR_TEST: {
            for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
                auto& scissor = enabledState.scissorState[i];
                if (scissor.gl_scissor_test != boolean) {
                    GLCall(glDisablei(capability, (GLuint)i));
                    scissor.gl_scissor_test = boolean;
                }
            }
            return true;
            break;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glEnablei(GLenum capability, GLuint index) {
    auto boolean = GL_TRUE;
    switch (capability) {
        case GL_BLEND: {
            auto& blendState = enabledState.blendState[index];
            if (blendState.gl_blend != boolean) {
                GLCall(glEnablei(capability, index));
                blendState.gl_blend = boolean;
                return true;
            }
            break;
        }case GL_SCISSOR_TEST: {
            auto& scissorState = enabledState.scissorState[index];
            if (scissorState.gl_scissor_test != boolean) {
                GLCall(glEnablei(capability, index));
                scissorState.gl_scissor_test = boolean;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glDisablei(GLenum capability, GLuint index) {
    auto boolean = GL_FALSE;
    switch (capability) {
        case GL_BLEND: {
            auto& blendState = enabledState.blendState[index];
            if (blendState.gl_blend != boolean) {
                GLCall(glDisablei(capability, index));
                blendState.gl_blend = boolean;
                return true;
            }
            break;
        }case GL_SCISSOR_TEST: {
            auto& scissorState = enabledState.scissorState[index];
            if (scissorState.gl_scissor_test != boolean) {
                GLCall(glDisablei(capability, index));
                scissorState.gl_scissor_test = boolean;
                return true;
            }
            break;
        }default: {
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    switch (target) {
        case GL_READ_FRAMEBUFFER: {
            if (framebufferState.framebuffer_read != framebuffer) {
                GLCall(glBindFramebuffer(target, framebuffer));
                framebufferState.framebuffer_read = framebuffer;
                return true;
            }
            break;
        }case GL_DRAW_FRAMEBUFFER: {
            if (framebufferState.framebuffer_draw != framebuffer) {
                GLCall(glBindFramebuffer(target, framebuffer));
                framebufferState.framebuffer_draw = framebuffer;
                return true;
            }
            break;
        }case GL_FRAMEBUFFER: {
            bool success = false;
            if (framebufferState.framebuffer_read != framebuffer) {
                GLCall(glBindFramebuffer(target, framebuffer));
                framebufferState.framebuffer_read = framebuffer;
                success = true;
            }
            if (framebufferState.framebuffer_draw != framebuffer) {
                GLCall(glBindFramebuffer(target, framebuffer));
                framebufferState.framebuffer_draw = framebuffer;
                success = true;
            }
            if (success) {
                return true;
            }
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glBindRenderbuffer(GLuint renderBuffer) {
    if (framebufferState.renderbuffer != renderBuffer) {
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer));
        framebufferState.renderbuffer = renderBuffer;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glBlendEquation(GLenum mode) {
    unsigned int buf = 0;
    for (auto& state : blendEquationState) {
        if (state.mode != mode) {
            GLCall(glBlendEquationi(buf, mode));
            state.mode = mode;
        }
        ++buf;
    }
    return true;
}
bool OpenGLState::GL_glBlendEquationi(GLuint buf, GLenum mode) {
    auto& state = blendEquationState[buf];
    if (state.mode != mode) {
        GLCall(glBlendEquationi(buf, mode));
        state.mode = mode;
        return true;
    }
    return false;   
}