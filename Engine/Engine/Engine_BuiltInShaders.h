#pragma once
#ifndef ENGINE_BUILT_IN_SHADERS_H
#define ENGINE_BUILT_IN_SHADERS_H

#include <string>

namespace Engine{
    namespace Shaders{
        namespace Detail{
            class ShadersManagement{
                public:
                    static void init();
                    static void convertShaderCode(std::string&);

                    static std::string version;

					static std::string constants;
					static std::string conditional_functions;
                    static std::string float_into_2_floats;
                    static std::string determinent_mat3;
                    static std::string normals_octahedron_compression_functions;
                    static std::string reconstruct_log_depth_functions;

                    static std::string fullscreen_quad_vertex;
                    static std::string vertex_basic;
                    static std::string vertex_hud;
                    static std::string vertex_skybox;
                
                    static std::string stencil_passover;

                    static std::string smaa_common;

                    static std::string smaa_frag_1_stencil;

                    static std::string smaa_vertex_1;
                    static std::string smaa_frag_1;

                    static std::string smaa_vertex_2;
                    static std::string smaa_frag_2;

                    static std::string smaa_vertex_3;
                    static std::string smaa_frag_3;

                    static std::string smaa_vertex_4;
                    static std::string smaa_frag_4;

                    static std::string fxaa_frag;

                    static std::string deferred_frag;
                    static std::string deferred_frag_hud;
                    static std::string deferred_frag_skybox;
					static std::string deferred_frag_skybox_fake;

                    static std::string copy_depth_frag;
                    static std::string cubemap_convolude_frag;
                    static std::string cubemap_prefilter_envmap_frag;
                    static std::string brdf_precompute;

                    static std::string ssao_frag;
                    static std::string hdr_frag;
                    static std::string godRays_frag;
                    static std::string blur_frag;

                    static std::string greyscale_frag;
                    static std::string edge_canny_blur;
                    static std::string edge_canny_frag;

                    static std::string final_frag;
                    static std::string lighting_frag;
					static std::string lighting_frag_optimized;
                    static std::string lighting_frag_gi;
            };
        };
    };
};

#endif
