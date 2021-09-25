
#include <serenity/renderer/opengl/State.h>
#include <serenity/renderer/opengl/OpenGL.h>
#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/dependencies/glm.h>
#include <serenity/utils/Utils.h>


using namespace Engine;
using namespace Engine::priv;

namespace {
    constexpr std::array<void(*)(GLenum), 2> GL_ENABLE_FUNCTIONS = {
        glDisable,
        glEnable,
    };
    inline void call_gl_enable(GLenum isTrue, GLenum cap) noexcept {
        GL_ENABLE_FUNCTIONS[isTrue](cap);
    }
}

Engine::priv::OpenGLState::Constants Engine::priv::OpenGLState::constants;

Engine::unordered_string_map<std::string, std::string> VERSION_MAP;
Engine::unordered_string_map<std::string, std::string> POPULATE_VERSION_MAP() {
    Engine::unordered_string_map<std::string, std::string> ret;
    ret["1.1"] = "110";
    ret["2.0"] = "110";
    ret["2.1"] = "120";
    ret["3.0"] = "130";
    ret["3.1"] = "140";
    ret["3.2"] = "150";
    ret["3.3"] = "330";
    ret["4.0"] = "400";
    ret["4.1"] = "410";
    ret["4.2"] = "420";
    ret["4.3"] = "430";
    ret["4.4"] = "440";
    ret["4.5"] = "450";
    ret["4.6"] = "460";
    return ret;
}


void OpenGLState::Constants::calculate() {
    glGetIntegerv(GL_MAJOR_VERSION, &MAJOR_VERSION);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &MAX_3D_TEXTURE_SIZE);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &MAX_ARRAY_TEXTURE_LAYERS);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MAX_COLOR_ATTACHMENTS);
    glGetIntegerv(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, &MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &MAX_COMBINED_UNIFORM_BLOCKS);
    glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &MAX_CUBE_MAP_TEXTURE_SIZE);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
    glGetIntegerv(GL_MAX_ELEMENT_INDEX, &MAX_ELEMENT_INDEX);
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &MAX_ELEMENTS_INDICES);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &MAX_ELEMENTS_VERTICES);
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &MAX_FRAGMENT_INPUT_COMPONENTS);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &MAX_FRAGMENT_UNIFORM_BLOCKS);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &MAX_FRAGMENT_UNIFORM_COMPONENTS);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &MAX_FRAGMENT_UNIFORM_VECTORS);
    glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &MAX_PROGRAM_TEXEL_OFFSET);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &MAX_RENDERBUFFER_SIZE);
    glGetIntegerv(GL_MAX_SAMPLES, &MAX_SAMPLES);

    glGetIntegerv(GL_MAX_SERVER_WAIT_TIMEOUT, &MAX_SERVER_WAIT_TIMEOUT);

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MAX_TEXTURE_IMAGE_UNITS);
    glGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS, &MAX_TEXTURE_LOD_BIAS);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, &MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, &MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &MAX_UNIFORM_BLOCK_SIZE);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MAX_UNIFORM_BUFFER_BINDINGS);
    glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &MAX_VARYING_COMPONENTS);
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &MAX_VARYING_VECTORS);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MAX_VERTEX_ATTRIBS);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MAX_VERTEX_TEXTURE_IMAGE_UNITS);
    glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &MAX_VERTEX_OUTPUT_COMPONENTS);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &MAX_VERTEX_UNIFORM_BLOCKS);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &MAX_VERTEX_UNIFORM_BLOCKS);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &MAX_VERTEX_UNIFORM_COMPONENTS);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &MAX_VERTEX_UNIFORM_VECTORS);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &MAX_VIEWPORT_DIMS);
    glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &MIN_PROGRAM_TEXEL_OFFSET);
    glGetIntegerv(GL_MINOR_VERSION, &MINOR_VERSION);
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &NUM_COMPRESSED_TEXTURE_FORMATS);
    glGetIntegerv(GL_NUM_EXTENSIONS, &NUM_EXTENSIONS);
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &NUM_PROGRAM_BINARY_FORMATS);
    glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &NUM_SHADER_BINARY_FORMATS);

    COMPRESSED_TEXTURE_FORMATS.resize(NUM_COMPRESSED_TEXTURE_FORMATS, 0);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, COMPRESSED_TEXTURE_FORMATS.data());

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &MAX_TEXTURE_MAX_ANISOTROPY);

    GLSL_VERSION = std::stoi(OpenGLState::getHighestGLSLVersion());
}

void OpenGLState::GL_INIT_DEFAULT_STATE_MACHINE(uint32_t windowWidth, uint32_t windowHeight) {
    GLint    int_value;
    GLfloat  float_value;
    //GLboolean boolean_value;

    viewportState = ViewportState((GLsizei)windowWidth, (GLsizei)windowHeight);

    constants.calculate();

    textureUnits.reserve(constants.MAX_TEXTURE_IMAGE_UNITS);
    for (size_t i = 0; i < textureUnits.capacity(); ++i) {
        textureUnits.emplace_back();
    }

    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &int_value);
    enabledState.blendState.reserve((size_t)int_value);
    blendEquationState.reserve((size_t)int_value);
    for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
        enabledState.blendState.emplace_back();
    }
    for (size_t i = 0; i < blendEquationState.capacity(); ++i) {
        blendEquationState.emplace_back();
    }

    glGetIntegerv(GL_MAX_VIEWPORTS, &int_value);
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
    glActiveTexture(GL_TEXTURE0); //this was said to be needed for some drivers

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
    glActiveTexture(currentActiveTextureUnit);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glColorMask(colorMaskState.r, colorMaskState.g, colorMaskState.b, colorMaskState.a);
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

    //glenable stuff
    for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
        auto& blend = enabledState.blendState[i];
        (blend.gl_blend == GL_TRUE) ? glEnablei(GL_BLEND, (GLuint)i) : glDisablei(GL_BLEND, (GLuint)i);
    }
    for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
        auto& scissor = enabledState.scissorState[i];
        (scissor.gl_scissor_test == GL_TRUE) ? glEnablei(GL_SCISSOR_TEST, (GLuint)i) : glDisablei(GL_SCISSOR_TEST, (GLuint)i);
    }

    call_gl_enable(enabledState.gl_multisample, GL_MULTISAMPLE);
    call_gl_enable(enabledState.gl_dither, GL_DITHER);
    call_gl_enable(enabledState.gl_clip_distance_1, GL_CLIP_DISTANCE1);
    call_gl_enable(enabledState.gl_clip_distance_2, GL_CLIP_DISTANCE2);
    call_gl_enable(enabledState.gl_clip_distance_3, GL_CLIP_DISTANCE3);
    call_gl_enable(enabledState.gl_clip_distance_4, GL_CLIP_DISTANCE4);
    call_gl_enable(enabledState.gl_clip_distance_5, GL_CLIP_DISTANCE5);
    call_gl_enable(enabledState.gl_color_logic_op, GL_COLOR_LOGIC_OP);
    call_gl_enable(enabledState.gl_cull_face, GL_CULL_FACE);
    call_gl_enable(enabledState.gl_debug_output, GL_DEBUG_OUTPUT);
    call_gl_enable(enabledState.gl_debug_output_syncronous, GL_DEBUG_OUTPUT_SYNCHRONOUS);
    call_gl_enable(enabledState.gl_depth_clamp, GL_DEPTH_CLAMP);
    call_gl_enable(enabledState.gl_depth_test, GL_DEPTH_TEST);
    call_gl_enable(enabledState.gl_framebuffer_srgb, GL_FRAMEBUFFER_SRGB);
    call_gl_enable(enabledState.gl_line_smooth, GL_LINE_SMOOTH);
    call_gl_enable(enabledState.gl_polygon_offset_fill, GL_POLYGON_OFFSET_FILL);
    call_gl_enable(enabledState.gl_polygon_offset_line, GL_POLYGON_OFFSET_LINE);
    call_gl_enable(enabledState.gl_polygon_offset_point, GL_POLYGON_OFFSET_POINT);
    call_gl_enable(enabledState.gl_polygon_smooth, GL_POLYGON_SMOOTH);
    call_gl_enable(enabledState.gl_primitive_restart, GL_PRIMITIVE_RESTART);
    call_gl_enable(enabledState.gl_primitive_restart_fixed_index, GL_PRIMITIVE_RESTART_FIXED_INDEX);
    call_gl_enable(enabledState.gl_rasterizer_discard, GL_RASTERIZER_DISCARD);
    call_gl_enable(enabledState.gl_sample_alpha_to_coverage, GL_SAMPLE_ALPHA_TO_COVERAGE);
    call_gl_enable(enabledState.gl_sample_alpha_to_one, GL_SAMPLE_ALPHA_TO_ONE);
    call_gl_enable(enabledState.gl_sample_coverage, GL_SAMPLE_COVERAGE);
    call_gl_enable(enabledState.gl_sample_shading, GL_SAMPLE_SHADING);
    call_gl_enable(enabledState.gl_sample_mask, GL_SAMPLE_MASK);
    call_gl_enable(enabledState.gl_stencil_test, GL_STENCIL_TEST);
    call_gl_enable(enabledState.gl_texture_cube_map_seamless, GL_TEXTURE_CUBE_MAP_SEAMLESS);
    call_gl_enable(enabledState.gl_program_point_size, GL_PROGRAM_POINT_SIZE);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferState.framebuffer_read);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferState.framebuffer_draw);
    glBindRenderbuffer(GL_RENDERBUFFER, framebufferState.renderbuffer);

    for (size_t i = 0; i < blendEquationState.capacity(); ++i) {
        glBlendEquationi(GLuint(i), blendEquationState[i].mode);
    }
}
bool OpenGLState::GL_glActiveTexture(GLenum textureUnit) {
    currentActiveTextureUnit = textureUnit;
    const GLuint capacity    = textureUnits.capacity() - 1;
    currentActiveTextureUnit = std::min(currentActiveTextureUnit, capacity);
    glActiveTexture(GL_TEXTURE0 + currentActiveTextureUnit);
    return true;
}
bool OpenGLState::GL_glUnbindTexture(GLenum textureUnit, GLenum textureTarget) {
    GL_glActiveTexture(textureUnit);
    GL_glBindTextureForRendering(textureTarget, 0);
    return true;
}
bool OpenGLState::GL_glBindTextureForModification(GLenum textureTarget, GLuint textureObject) {
    const GLuint capacity = textureUnits.capacity() - 1;
    GL_glActiveTexture(GLenum(capacity));
    GL_glBindTextureForRendering(textureTarget, textureObject);
    return true;
}
uint32_t OpenGLState::internal_get_enum_index_from_gl_texture_type(GLenum textureType) noexcept {
    switch (textureType) {
        case GL_TEXTURE_1D: {
            return uint32_t(TextureType::Texture1D);
        } case GL_TEXTURE_2D: {
            return uint32_t(TextureType::Texture2D);
        } case GL_TEXTURE_3D: {
            return uint32_t(TextureType::Texture3D);
        } case GL_TEXTURE_CUBE_MAP: {
            return uint32_t(TextureType::CubeMap);
        }
    }
    return 0;
}
GLuint OpenGLState::getCurrentlyBoundTextureOfType(GLenum textureType) noexcept {
    const uint32_t typeIndex = internal_get_enum_index_from_gl_texture_type(textureType);
    return textureUnits[currentActiveTextureUnit].openglIDs[typeIndex];
}
bool OpenGLState::GL_glBindTextureForRendering(GLenum textureTarget, GLuint textureObject) {
    const uint32_t typeIndex = internal_get_enum_index_from_gl_texture_type(textureTarget);
    textureUnits[currentActiveTextureUnit].openglIDs[typeIndex] = textureObject;
    if (typeIndex > 0) {
        glBindTexture(textureTarget, textureObject);
    }
    return static_cast<bool>(typeIndex);
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
    glClearColor(r, g, b, a);
    clearColor.r = r;
    clearColor.g = g;
    clearColor.b = b;
    clearColor.a = a;
    return true;
}
bool OpenGLState::GL_glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    if (r == colorMaskState.r && g == colorMaskState.g && b == colorMaskState.b && a == colorMaskState.a) {
        return false;
    }
    glColorMask(r, g, b, a);
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
    glClearDepth(depth);
    return true;
}
bool OpenGLState::GL_glClearDepthf(GLfloat depth_float) {
    //Values are clamped to the range [0,1]
    depth_float = glm::clamp(depth_float, 0.0f, 1.0f);
    if (clearDepth.depthf == depth_float) {
        return false;
    }
    clearDepth.depthf = depth_float;
    glClearDepthf(depth_float);
    return true;
}
bool OpenGLState::GL_glClearStencil(GLint stencil) {
    if (clearStencil.stencil == stencil) {
        return false;
    }
    clearStencil.stencil = stencil;
    glClearStencil(stencil);
    return true;
}
bool OpenGLState::GL_glStencilMaskSeparate(GLenum face, GLuint mask) {
    switch (face) {
        case GL_FRONT: {
            if (stencilMask.front_mask != mask) {
                glStencilMaskSeparate(GL_FRONT, mask);
                stencilMask.front_mask = mask;
                return true;
            }
            break;
        } case GL_BACK: {
            if (stencilMask.back_mask != mask) {
                glStencilMaskSeparate(GL_BACK, mask);
                stencilMask.back_mask = mask;
                return true;
            }
            break;
        } case GL_FRONT_AND_BACK: {
            if (stencilMask.back_mask != mask || stencilMask.front_mask != mask) {
                glStencilMaskSeparate(GL_FRONT_AND_BACK, mask);
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
        glStencilMask(mask);
        stencilMask.front_mask = mask;
        stencilMask.back_mask = mask;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
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
bool OpenGLState::GL_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
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
        } case GL_BACK: {
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
        } case GL_FRONT_AND_BACK: {
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
    }
    return false;
}
bool OpenGLState::GL_glUseProgram(GLuint program) {
    if (useProgram.program != program) {
        glUseProgram(program);
        useProgram.program = program;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glBindVertexArray(GLuint vao) {
    if (vaoState.vao != vao) {
        glBindVertexArray(vao);
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
    glViewport(x, y, width, height);
    viewportState.x = x;
    viewportState.y = y;
    viewportState.width = width;
    viewportState.height = height;
    return true;
}
bool OpenGLState::GL_glCullFace(GLenum mode) {
    if (cullFaceState.mode != mode) {
        glCullFace(mode);
        cullFaceState.mode = mode;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glFrontFace(GLenum mode) {
    if (frontFaceState.mode != mode) {
        glFrontFace(mode);
        frontFaceState.mode = mode;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glDepthFunc(GLenum func) {
    if (depthFuncState.func != func) {
        glDepthFunc(func);
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
                glPixelStorei(GL_PACK_ALIGNMENT, param);
                pixelStoreiState.pack_alignment = param;
                return true;
            }
            break;
        } case GL_UNPACK_ALIGNMENT: {
            if (pixelStoreiState.unpack_alignment != param) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, param);
                pixelStoreiState.unpack_alignment = param;
                return true;
            }
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
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
        } case GL_BACK: {
            if (stencilFuncState.func_back != func || stencilFuncState.ref_back != ref || stencilFuncState.mask_back != mask) {
                glStencilFuncSeparate(face, func, ref, mask);
                stencilFuncState.func_back = func;
                stencilFuncState.ref_back = ref;
                stencilFuncState.mask_back = mask;
                return true;
            }
            break;
        } case GL_FRONT_AND_BACK: {
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
    }
    return false;
}
bool OpenGLState::GL_glStencilFunc(GLenum func, GLint ref, GLuint mask) {
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
bool OpenGLState::GL_glEnable(GLenum capability) {
    auto boolean = GL_TRUE;
    switch (capability) {
        case GL_MULTISAMPLE: {
            if (enabledState.gl_multisample != boolean) {
                glEnable(capability);
                enabledState.gl_multisample = boolean;
                return true;
            }
            break;
        } case GL_DITHER: {
            if (enabledState.gl_dither != boolean) {
                glEnable(capability);
                enabledState.gl_dither = boolean;
                return true;
            }
            break;
        } case GL_CLIP_DISTANCE1: {
            if (enabledState.gl_clip_distance_1 != boolean) {
                glEnable(capability);
                enabledState.gl_clip_distance_1 = boolean;
                return true;
            }
            break;
        } case GL_CLIP_DISTANCE2: {
            if (enabledState.gl_clip_distance_2 != boolean) {
                glEnable(capability);
                enabledState.gl_clip_distance_2 = boolean;
                return true;
            }
            break;
        } case GL_CLIP_DISTANCE3: {
            if (enabledState.gl_clip_distance_3 != boolean) {
                glEnable(capability);
                enabledState.gl_clip_distance_3 = boolean;
                return true;
            }
            break;
        } case GL_CLIP_DISTANCE4: {
            if (enabledState.gl_clip_distance_4 != boolean) {
                glEnable(capability);
                enabledState.gl_clip_distance_4 = boolean;
                return true;
            }
            break;
        } case GL_CLIP_DISTANCE5: {
            if (enabledState.gl_clip_distance_5 != boolean) {
                glEnable(capability);
                enabledState.gl_clip_distance_5 = boolean;
                return true;
            }
            break;
        } case GL_COLOR_LOGIC_OP: {
            if (enabledState.gl_color_logic_op != boolean) {
                glEnable(capability);
                enabledState.gl_color_logic_op = boolean;
                return true;
            }
            break;
        } case GL_CULL_FACE: {
            if (enabledState.gl_cull_face != boolean) {
                glEnable(capability);
                enabledState.gl_cull_face = boolean;
                return true;
            }
            break;
        } case GL_DEBUG_OUTPUT: {
            if (enabledState.gl_debug_output != boolean) {
                glEnable(capability);
                enabledState.gl_debug_output = boolean;
                return true;
            }
            break;
        } case GL_DEBUG_OUTPUT_SYNCHRONOUS: {
            if (enabledState.gl_debug_output_syncronous != boolean) {
                glEnable(capability);
                enabledState.gl_debug_output_syncronous = boolean;
                return true;
            }
            break;
        } case GL_DEPTH_CLAMP: {
            if (enabledState.gl_depth_clamp != boolean) {
                glEnable(capability);
                enabledState.gl_depth_clamp = boolean;
                return true;
            }
            break;
        } case GL_DEPTH_TEST: {
            if (enabledState.gl_depth_test != boolean) {
                glEnable(capability);
                enabledState.gl_depth_test = boolean;
                return true;
            }
            break;
        } case GL_FRAMEBUFFER_SRGB: {
            if (enabledState.gl_framebuffer_srgb != boolean) {
                glEnable(capability);
                enabledState.gl_framebuffer_srgb = boolean;
                return true;
            }
            break;
        } case GL_LINE_SMOOTH: {
            if (enabledState.gl_line_smooth != boolean) {
                glEnable(capability);
                enabledState.gl_line_smooth = boolean;
                return true;
            }
            break;
        } case GL_POLYGON_OFFSET_FILL: {
            if (enabledState.gl_polygon_offset_fill != boolean) {
                glEnable(capability);
                enabledState.gl_polygon_offset_fill = boolean;
                return true;
            }
            break;
        } case GL_POLYGON_OFFSET_LINE: {
            if (enabledState.gl_polygon_offset_line != boolean) {
                glEnable(capability);
                enabledState.gl_polygon_offset_line = boolean;
                return true;
            }
            break;
        } case GL_POLYGON_OFFSET_POINT: {
            if (enabledState.gl_polygon_offset_point != boolean) {
                glEnable(capability);
                enabledState.gl_polygon_offset_point = boolean;
                return true;
            }
            break;
        } case GL_POLYGON_SMOOTH: {
            if (enabledState.gl_polygon_smooth != boolean) {
                glEnable(capability);
                enabledState.gl_polygon_smooth = boolean;
                return true;
            }
            break;
        } case GL_PRIMITIVE_RESTART: {
            if (enabledState.gl_primitive_restart != boolean) {
                glEnable(capability);
                enabledState.gl_primitive_restart = boolean;
                return true;
            }
            break;
        } case GL_PRIMITIVE_RESTART_FIXED_INDEX: {
            if (enabledState.gl_primitive_restart_fixed_index != boolean) {
                glEnable(capability);
                enabledState.gl_primitive_restart_fixed_index = boolean;
                return true;
            }
            break;
        } case GL_RASTERIZER_DISCARD: {
            if (enabledState.gl_rasterizer_discard != boolean) {
                glEnable(capability);
                enabledState.gl_rasterizer_discard = boolean;
                return true;
            }
            break;
        } case GL_SAMPLE_ALPHA_TO_COVERAGE: {
            if (enabledState.gl_sample_alpha_to_coverage != boolean) {
                glEnable(capability);
                enabledState.gl_sample_alpha_to_coverage = boolean;
                return true;
            }
            break;
        } case GL_SAMPLE_ALPHA_TO_ONE: {
            if (enabledState.gl_sample_alpha_to_one != boolean) {
                glEnable(capability);
                enabledState.gl_sample_alpha_to_one = boolean;
                return true;
            }
            break;
        } case GL_SAMPLE_COVERAGE: {
            if (enabledState.gl_sample_coverage != boolean) {
                glEnable(capability);
                enabledState.gl_sample_coverage = boolean;
                return true;
            }
            break;
        } case GL_SAMPLE_SHADING: {
            if (enabledState.gl_sample_shading != boolean) {
                glEnable(capability);
                enabledState.gl_sample_shading = boolean;
                return true;
            }
            break;
        } case GL_SAMPLE_MASK: {
            if (enabledState.gl_sample_mask != boolean) {
                glEnable(capability);
                enabledState.gl_sample_mask = boolean;
                return true;
            }
            break;
        } case GL_STENCIL_TEST: {
            if (enabledState.gl_stencil_test != boolean) {
                glEnable(capability);
                enabledState.gl_stencil_test = boolean;
                return true;
            }
            break;
        } case GL_TEXTURE_CUBE_MAP_SEAMLESS: {
            if (enabledState.gl_texture_cube_map_seamless != boolean) {
                glEnable(capability);
                enabledState.gl_texture_cube_map_seamless = boolean;
                return true;
            }
            break;
        } case GL_PROGRAM_POINT_SIZE: {
            if (enabledState.gl_program_point_size != boolean) {
                glEnable(capability);
                enabledState.gl_program_point_size = boolean;
                return true;
            }
            break;
        } case GL_BLEND: {
            for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
                auto& blend = enabledState.blendState[i];
                if (blend.gl_blend != boolean) {
                    glEnablei(capability, (GLuint)i);
                    blend.gl_blend = boolean;
                }
            }
            return true;
        } case GL_SCISSOR_TEST: {
            for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
                auto& scissor = enabledState.scissorState[i];
                if (scissor.gl_scissor_test != boolean) {
                    glEnablei(capability, (GLuint)i);
                    scissor.gl_scissor_test = boolean;
                }
            }
            return true;
        }
    }
    return false;
}
bool OpenGLState::GL_glDisable(GLenum capability) {
    auto boolean = GL_FALSE;
    switch (capability) {
        case GL_MULTISAMPLE: {
            if (enabledState.gl_multisample != boolean) {
                glDisable(capability);
                enabledState.gl_multisample = boolean;
                return true;
            }
            break;
        }case GL_DITHER: {
            if (enabledState.gl_dither != boolean) {
                glDisable(capability);
                enabledState.gl_dither = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE1: {
            if (enabledState.gl_clip_distance_1 != boolean) {
                glDisable(capability);
                enabledState.gl_clip_distance_1 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE2: {
            if (enabledState.gl_clip_distance_2 != boolean) {
                glDisable(capability);
                enabledState.gl_clip_distance_2 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE3: {
            if (enabledState.gl_clip_distance_3 != boolean) {
                glDisable(capability);
                enabledState.gl_clip_distance_3 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE4: {
            if (enabledState.gl_clip_distance_4 != boolean) {
                glDisable(capability);
                enabledState.gl_clip_distance_4 = boolean;
                return true;
            }
            break;
        }case GL_CLIP_DISTANCE5: {
            if (enabledState.gl_clip_distance_5 != boolean) {
                glDisable(capability);
                enabledState.gl_clip_distance_5 = boolean;
                return true;
            }
            break;
        }case GL_COLOR_LOGIC_OP: {
            if (enabledState.gl_color_logic_op != boolean) {
                glDisable(capability);
                enabledState.gl_color_logic_op = boolean;
                return true;
            }
            break;
        }case GL_CULL_FACE: {
            if (enabledState.gl_cull_face != boolean) {
                glDisable(capability);
                enabledState.gl_cull_face = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT: {
            if (enabledState.gl_debug_output != boolean) {
                glDisable(capability);
                enabledState.gl_debug_output = boolean;
                return true;
            }
            break;
        }case GL_DEBUG_OUTPUT_SYNCHRONOUS: {
            if (enabledState.gl_debug_output_syncronous != boolean) {
                glDisable(capability);
                enabledState.gl_debug_output_syncronous = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_CLAMP: {
            if (enabledState.gl_depth_clamp != boolean) {
                glDisable(capability);
                enabledState.gl_depth_clamp = boolean;
                return true;
            }
            break;
        }case GL_DEPTH_TEST: {
            if (enabledState.gl_depth_test != boolean) {
                glDisable(capability);
                enabledState.gl_depth_test = boolean;
                return true;
            }
            break;
        }case GL_FRAMEBUFFER_SRGB: {
            if (enabledState.gl_framebuffer_srgb != boolean) {
                glDisable(capability);
                enabledState.gl_framebuffer_srgb = boolean;
                return true;
            }
            break;
        }case GL_LINE_SMOOTH: {
            if (enabledState.gl_line_smooth != boolean) {
                glDisable(capability);
                enabledState.gl_line_smooth = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_FILL: {
            if (enabledState.gl_polygon_offset_fill != boolean) {
                glDisable(capability);
                enabledState.gl_polygon_offset_fill = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_LINE: {
            if (enabledState.gl_polygon_offset_line != boolean) {
                glDisable(capability);
                enabledState.gl_polygon_offset_line = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_OFFSET_POINT: {
            if (enabledState.gl_polygon_offset_point != boolean) {
                glDisable(capability);
                enabledState.gl_polygon_offset_point = boolean;
                return true;
            }
            break;
        }case GL_POLYGON_SMOOTH: {
            if (enabledState.gl_polygon_smooth != boolean) {
                glDisable(capability);
                enabledState.gl_polygon_smooth = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART: {
            if (enabledState.gl_primitive_restart != boolean) {
                glDisable(capability);
                enabledState.gl_primitive_restart = boolean;
                return true;
            }
            break;
        }case GL_PRIMITIVE_RESTART_FIXED_INDEX: {
            if (enabledState.gl_primitive_restart_fixed_index != boolean) {
                glDisable(capability);
                enabledState.gl_primitive_restart_fixed_index = boolean;
                return true;
            }
            break;
        }case GL_RASTERIZER_DISCARD: {
            if (enabledState.gl_rasterizer_discard != boolean) {
                glDisable(capability);
                enabledState.gl_rasterizer_discard = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_COVERAGE: {
            if (enabledState.gl_sample_alpha_to_coverage != boolean) {
                glDisable(capability);
                enabledState.gl_sample_alpha_to_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_ALPHA_TO_ONE: {
            if (enabledState.gl_sample_alpha_to_one != boolean) {
                glDisable(capability);
                enabledState.gl_sample_alpha_to_one = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_COVERAGE: {
            if (enabledState.gl_sample_coverage != boolean) {
                glDisable(capability);
                enabledState.gl_sample_coverage = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_SHADING: {
            if (enabledState.gl_sample_shading != boolean) {
                glDisable(capability);
                enabledState.gl_sample_shading = boolean;
                return true;
            }
            break;
        }case GL_SAMPLE_MASK: {
            if (enabledState.gl_sample_mask != boolean) {
                glDisable(capability);
                enabledState.gl_sample_mask = boolean;
                return true;
            }
            break;
        }case GL_STENCIL_TEST: {
            if (enabledState.gl_stencil_test != boolean) {
                glDisable(capability);
                enabledState.gl_stencil_test = boolean;
                return true;
            }
            break;
        }case GL_TEXTURE_CUBE_MAP_SEAMLESS: {
            if (enabledState.gl_texture_cube_map_seamless != boolean) {
                glDisable(capability);
                enabledState.gl_texture_cube_map_seamless = boolean;
                return true;
            }
            break;
        }case GL_PROGRAM_POINT_SIZE: {
            if (enabledState.gl_program_point_size != boolean) {
                glDisable(capability);
                enabledState.gl_program_point_size = boolean;
                return true;
            }
            break;
        }case GL_BLEND: {
            for (size_t i = 0; i < enabledState.blendState.capacity(); ++i) {
                auto& blend = enabledState.blendState[i];
                if (blend.gl_blend != boolean) {
                    glDisablei(capability, (GLuint)i);
                    blend.gl_blend = boolean;
                }
            }
            return true;
            break;
        }case GL_SCISSOR_TEST: {
            for (size_t i = 0; i < enabledState.scissorState.capacity(); ++i) {
                auto& scissor = enabledState.scissorState[i];
                if (scissor.gl_scissor_test != boolean) {
                    glDisablei(capability, (GLuint)i);
                    scissor.gl_scissor_test = boolean;
                }
            }
            return true;
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
                glEnablei(capability, index);
                blendState.gl_blend = boolean;
                return true;
            }
            break;
        } case GL_SCISSOR_TEST: {
            auto& scissorState = enabledState.scissorState[index];
            if (scissorState.gl_scissor_test != boolean) {
                glEnablei(capability, index);
                scissorState.gl_scissor_test = boolean;
                return true;
            }
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
                glDisablei(capability, index);
                blendState.gl_blend = boolean;
                return true;
            }
            break;
        } case GL_SCISSOR_TEST: {
            auto& scissorState = enabledState.scissorState[index];
            if (scissorState.gl_scissor_test != boolean) {
                glDisablei(capability, index);
                scissorState.gl_scissor_test = boolean;
                return true;
            }
            break;
        }
    }
    return false;
}
bool OpenGLState::GL_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    switch (target) {
        case GL_READ_FRAMEBUFFER: {
            if (framebufferState.framebuffer_read != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                framebufferState.framebuffer_read = framebuffer;
                return true;
            }
            break;
        } case GL_DRAW_FRAMEBUFFER: {
            if (framebufferState.framebuffer_draw != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                framebufferState.framebuffer_draw = framebuffer;
                return true;
            }
            break;
        } case GL_FRAMEBUFFER: {
            bool success = false;
            if (framebufferState.framebuffer_read != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                framebufferState.framebuffer_read = framebuffer;
                success = true;
            }
            if (framebufferState.framebuffer_draw != framebuffer) {
                glBindFramebuffer(target, framebuffer);
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
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        framebufferState.renderbuffer = renderBuffer;
        return true;
    }
    return false;
}
bool OpenGLState::GL_glBlendEquation(GLenum mode) {
    unsigned int buf = 0;
    for (auto& state : blendEquationState) {
        if (state.mode != mode) {
            glBlendEquationi(buf, mode);
            state.mode = mode;
        }
        ++buf;
    }
    return true;
}
bool OpenGLState::GL_glBlendEquationi(GLuint buf, GLenum mode) {
    auto& state = blendEquationState[buf];
    if (state.mode != mode) {
        glBlendEquationi(buf, mode);
        state.mode = mode;
        return true;
    }
    return false;   
}


std::string OpenGLState::getHighestGLSLVersion() noexcept {
    if (VERSION_MAP.size() == 0) {
        VERSION_MAP = POPULATE_VERSION_MAP();
    }
    std::string version = std::to_string(Engine::priv::OpenGLState::constants.MAJOR_VERSION) + "." + std::to_string(Engine::priv::OpenGLState::constants.MINOR_VERSION);
    return VERSION_MAP.contains(version) ? VERSION_MAP.at(version) : "";
}
int OpenGLState::getHighestGLSLVersionAsInt() {
    const auto version = OpenGLState::getHighestGLSLVersion();
    if (!version.empty()) {
        try {
            const auto res = std::stoi(version);
            return res;
        } catch (const std::invalid_argument&) {
            return -1;
        } catch (const std::out_of_range&) {
            return -1;
        }
    }
    return -1;
}