#pragma once
#ifndef ENGINE_OPENGL_STATE_MACHINE_H
#define ENGINE_OPENGL_STATE_MACHINE_H


#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class  RenderModule;

    /*
    This class provides a cpu side implementation of opengl's state machine, including default values
    */
    class  OpenGLState final {
        friend class  RenderModule;
        private:
            struct Constants final {
                std::vector<GLint> COMPRESSED_TEXTURE_FORMATS; //TODO: change this into an unordered_set so the texture types can be queried in O(1) time?

                GLint MAJOR_VERSION;
                GLint MAX_3D_TEXTURE_SIZE;
                GLint MAX_ARRAY_TEXTURE_LAYERS;
                GLint MAX_COLOR_ATTACHMENTS;
                GLint MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS;
                GLint MAX_COMBINED_TEXTURE_IMAGE_UNITS;
                GLint MAX_COMBINED_UNIFORM_BLOCKS;
                GLint MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS;
                GLint MAX_COMPUTE_SHADER_STORAGE_BLOCKS;
                GLint MAX_CUBE_MAP_TEXTURE_SIZE;
                GLint MAX_DRAW_BUFFERS;
                GLint MAX_ELEMENT_INDEX;
                GLint MAX_ELEMENTS_INDICES;
                GLint MAX_ELEMENTS_VERTICES;
                GLint MAX_FRAGMENT_INPUT_COMPONENTS;
                GLint MAX_FRAGMENT_UNIFORM_BLOCKS;
                GLint MAX_FRAGMENT_UNIFORM_COMPONENTS;
                GLint MAX_FRAGMENT_UNIFORM_VECTORS;
                GLint MAX_PROGRAM_TEXEL_OFFSET;
                GLint MAX_RENDERBUFFER_SIZE;
                GLint MAX_SAMPLES;
                GLint MAX_SERVER_WAIT_TIMEOUT;
                GLint MAX_TEXTURE_IMAGE_UNITS;
                GLint MAX_TEXTURE_LOD_BIAS;
                GLint MAX_TEXTURE_SIZE;
                GLint MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
                GLint MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS;
                GLint MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS;
                GLint MAX_UNIFORM_BLOCK_SIZE;
                GLint MAX_UNIFORM_BUFFER_BINDINGS;
                GLint MAX_VARYING_COMPONENTS;
                GLint MAX_VARYING_VECTORS;
                GLint MAX_VERTEX_ATTRIBS;
                GLint MAX_VERTEX_TEXTURE_IMAGE_UNITS;
                GLint MAX_VERTEX_OUTPUT_COMPONENTS;
                GLint MAX_VERTEX_UNIFORM_BLOCKS;
                GLint MAX_VERTEX_UNIFORM_COMPONENTS;
                GLint MAX_VERTEX_UNIFORM_VECTORS;
                GLint MAX_VIEWPORT_DIMS;
                GLint MIN_PROGRAM_TEXEL_OFFSET;
                GLint MINOR_VERSION;
                GLint NUM_COMPRESSED_TEXTURE_FORMATS;
                GLint NUM_EXTENSIONS;
                GLint NUM_PROGRAM_BINARY_FORMATS;
                GLint NUM_SHADER_BINARY_FORMATS;
                GLint GLSL_VERSION;

                GLfloat MAX_TEXTURE_MAX_ANISOTROPY;

                void calculate();

                inline bool supportsVAO() const noexcept { return (MAJOR_VERSION >= 3 && MINOR_VERSION >= 0); }

                inline bool supportsAniosotropicFiltering() const noexcept { return (MAJOR_VERSION >= 4 && MINOR_VERSION >= 6); }

                //can run real slowly on some gpu's / drivers. its core 32 but just to be safe, set it to 40
                inline bool supportsCubemapSeamless() const noexcept { return (MAJOR_VERSION >= 4 && MINOR_VERSION >= 0); }

                inline bool supportsClipControl() const noexcept { return (MAJOR_VERSION > 4 || (MAJOR_VERSION == 4 && MINOR_VERSION >= 5)); }

                inline bool supportsInstancing() const noexcept { return (MAJOR_VERSION >= 3 && MINOR_VERSION >= 1); }

                inline bool supportsUBO() const noexcept { return GLSL_VERSION >= 140; }
            };

            #pragma region TextureUnits
            struct TextureUnitState final {
                std::array<GLuint, TextureType::_TOTAL> openglIDs;

                TextureUnitState() {
                    openglIDs.fill(std::numeric_limits<GLuint>().max());
                }
            };
            std::vector<TextureUnitState> textureUnits;
            GLuint                        currentActiveTextureUnit = 0;

            uint32_t internal_get_enum_index_from_gl_texture_type(GLenum textureType) noexcept;

            #pragma endregion

            #pragma region ClearColor
            struct ClearColorState final {
                GLfloat r = 0.0f;
                GLfloat g = 0.0f;
                GLfloat b = 0.0f;
                GLfloat a = 0.0f;
                ClearColorState() = default;
            };
            ClearColorState clearColor;
            #pragma endregion

            #pragma region ColorMask
            struct ColorMaskState final {
                GLboolean r = GL_TRUE;
                GLboolean g = GL_TRUE;
                GLboolean b = GL_TRUE;
                GLboolean a = GL_TRUE;
                ColorMaskState() = default;
            };
            ColorMaskState colorMaskState;
            #pragma endregion

            #pragma region ClearDepth
            struct ClearDepthState final {
                GLdouble depth  = 1.0;
                GLfloat  depthf = 1.0f;
                ClearDepthState() = default;
            };
            ClearDepthState clearDepth;
            #pragma endregion

            #pragma region ClearStencil
            struct ClearStencilState final {
                GLint stencil = 0;
                ClearStencilState() = default;
            };
            ClearStencilState clearStencil;
            #pragma endregion

            #pragma region StencilMask
            struct StencilMaskState final {
                GLuint front_mask = 0xFFFFFFFF;
                GLuint back_mask  = 0xFFFFFFFF;
                StencilMaskState() = default;
            };
            StencilMaskState stencilMask;
            #pragma endregion

            #pragma region StencilOp
            struct StencilOpState final {
                GLenum sFail_front  = GL_KEEP;
                GLenum dpFail_front = GL_KEEP;
                GLenum dpPass_front = GL_KEEP;

                GLenum sFail_back   = GL_KEEP;
                GLenum dpFail_back  = GL_KEEP;
                GLenum dpPass_back  = GL_KEEP;

                StencilOpState() = default;
            };
            StencilOpState stencilOp;
            #pragma endregion

            #pragma region UseProgram
            struct UseProgramState final {
                GLuint program = 0;

                UseProgramState() = default;
            };
            UseProgramState useProgram;
            #pragma endregion

            #pragma region VAO
            struct VertexArrayObjState final {
                GLuint vao = 0;
                VertexArrayObjState() = default;
            };
            VertexArrayObjState vaoState;
            #pragma endregion

            #pragma region Viewport
            struct ViewportState final {
                GLint   x      = 0;
                GLint   y      = 0;
                GLsizei width  = 0;
                GLsizei height = 0;
                ViewportState() = default;
                //note - this is the real default glViewport state
                ViewportState(GLsizei width_, GLsizei height_) 
                    : width{ width_ }
                    , height{ height_ }
                {}
            };
            ViewportState viewportState;
            #pragma endregion

            #pragma region CullFace
            struct CullFaceState final {
                GLenum mode = GL_BACK;
                CullFaceState() = default;
            };
            CullFaceState cullFaceState;
            #pragma endregion

            #pragma region FrontFace
            struct FrontFaceState final {
                GLenum mode = GL_CCW;
                FrontFaceState() = default;
            };
            FrontFaceState frontFaceState;
            #pragma endregion

            #pragma region DepthFunc
            struct DepthFuncState final {
                GLenum func = GL_LESS;
                DepthFuncState() = default;
            };
            DepthFuncState depthFuncState;
            #pragma endregion

            #pragma region PixelStorei
            struct PixelStoreiState final {
                GLenum pack_alignment   = 4;
                GLenum unpack_alignment = 4;
                PixelStoreiState() = default;
            };
            PixelStoreiState pixelStoreiState;
            #pragma endregion

            #pragma region StencilFunc
            struct StencilFuncState final {
                GLenum func_front  = GL_ALWAYS;
                GLint  ref_front   = 0;
                GLuint mask_front  = 0xFFFFFFFF;

                GLenum func_back   = GL_ALWAYS;
                GLint  ref_back    = 0;
                GLuint mask_back   = 0xFFFFFFFF;

                StencilFuncState() = default;
            };
            StencilFuncState stencilFuncState;
            #pragma endregion

            #pragma region Enable
            struct EnabledState final {
                struct GLBlendState final {
                    GLenum gl_blend = GL_FALSE; //index​ must be less than GL_MAX_DRAW_BUFFERS
                    GLBlendState() = default;
                };
                struct GLScissorTestState final {
                    GLenum gl_scissor_test = GL_FALSE; //index​ must be less than GL_MAX_VIEWPORTS
                    GLScissorTestState() = default;
                };
                std::vector<GLBlendState>       blendState;
                std::vector<GLScissorTestState> scissorState;

                GLenum gl_clip_distance_1               = GL_FALSE;
                GLenum gl_clip_distance_2               = GL_FALSE;
                GLenum gl_clip_distance_3               = GL_FALSE;
                GLenum gl_clip_distance_4               = GL_FALSE;
                GLenum gl_clip_distance_5               = GL_FALSE;
                GLenum gl_color_logic_op                = GL_FALSE;
                GLenum gl_cull_face                     = GL_FALSE;
                GLenum gl_debug_output                  = GL_FALSE;
                GLenum gl_debug_output_syncronous       = GL_FALSE;
                GLenum gl_depth_clamp                   = GL_FALSE;
                GLenum gl_depth_test                    = GL_FALSE;
                GLenum gl_dither                        = GL_TRUE;
                GLenum gl_framebuffer_srgb              = GL_FALSE;
                GLenum gl_line_smooth                   = GL_FALSE;
                GLenum gl_multisample                   = GL_TRUE;
                GLenum gl_polygon_offset_fill           = GL_FALSE;
                GLenum gl_polygon_offset_line           = GL_FALSE;
                GLenum gl_polygon_offset_point          = GL_FALSE;
                GLenum gl_polygon_smooth                = GL_FALSE;
                GLenum gl_primitive_restart             = GL_FALSE;
                GLenum gl_primitive_restart_fixed_index = GL_FALSE;
                GLenum gl_rasterizer_discard            = GL_FALSE;
                GLenum gl_sample_alpha_to_coverage      = GL_FALSE;
                GLenum gl_sample_alpha_to_one           = GL_FALSE;
                GLenum gl_sample_coverage               = GL_FALSE;
                GLenum gl_sample_shading                = GL_FALSE;
                GLenum gl_sample_mask                   = GL_FALSE;
                GLenum gl_stencil_test                  = GL_FALSE;
                GLenum gl_texture_cube_map_seamless     = GL_FALSE;
                GLenum gl_program_point_size            = GL_FALSE;

                EnabledState() = default;
            };
            EnabledState enabledState;
            #pragma endregion

            #pragma region Framebuffer
            struct FramebufferState final {
                GLuint framebuffer_read = 0;
                GLuint framebuffer_draw = 0;
                GLuint renderbuffer     = 0;
                FramebufferState() = default;
            };
            FramebufferState framebufferState;
            #pragma endregion

            #pragma region BlendEquation
            struct BlendEquationState final {
                GLenum mode = GL_FUNC_ADD;
                BlendEquationState() = default;
            };
            std::vector<BlendEquationState> blendEquationState;
            #pragma endregion

        public:
            static Constants constants;

            static std::string getHighestGLSLVersion() noexcept;
            static int getHighestGLSLVersionAsInt();

            GLuint getCurrentlyBoundTextureOfType(GLenum textureType) noexcept;

            bool GL_glActiveTexture(GLenum textureUnit);
            bool GL_glUnbindTexture(GLenum textureUnit, GLenum textureTarget);
            bool GL_glBindTextureForModification(GLenum textureTarget, GLuint textureObject);
            bool GL_glBindTextureForRendering(GLenum textureTarget, GLuint textureObject);
            bool GL_glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
            bool GL_glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
            bool GL_glClearDepth(GLdouble depth);
            bool GL_glClearDepthf(GLfloat depth_float);
            bool GL_glClearStencil(GLint stencil);
            bool GL_glStencilMaskSeparate(GLenum face, GLuint mask);
            bool GL_glStencilMask(GLuint mask);
            bool GL_glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
            bool GL_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
            bool GL_glUseProgram(GLuint program);
            bool GL_glBindVertexArray(GLuint vao);
            bool GL_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
            bool GL_glCullFace(GLenum mode);
            bool GL_glFrontFace(GLenum mode);
            bool GL_glDepthFunc(GLenum func);
            bool GL_glPixelStorei(GLenum pname, GLint param);
            bool GL_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
            bool GL_glStencilFunc(GLenum func, GLint ref, GLuint mask);

            bool GL_glEnable(GLenum capability);
            bool GL_glDisable(GLenum capability);

            bool GL_glEnablei(GLenum capability, GLuint index);
            bool GL_glDisablei(GLenum capability, GLuint index);

            bool GL_glBindFramebuffer(GLenum target, GLuint framebuffer);
            bool GL_glBindRenderbuffer(GLuint renderBuffer);

            bool GL_glBlendEquation(GLenum mode);
            bool GL_glBlendEquationi(GLuint buf, GLenum mode);

            void GL_INIT_DEFAULT_STATE_MACHINE(uint32_t windowWidth, uint32_t windowHeight);
            void GL_RESTORE_DEFAULT_STATE_MACHINE(uint32_t windowWidth, uint32_t windowHeight);
            void GL_RESTORE_CURRENT_STATE_MACHINE();
    };
};

#endif