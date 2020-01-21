#pragma once
#ifndef ENGINE_BUILT_IN_SHADERS_H
#define ENGINE_BUILT_IN_SHADERS_H

#include <string>

namespace Engine{
    namespace priv{
        class EShaders{
            public:
                static void init();

                static std::string decal_vertex;
                static std::string decal_frag;

                static std::string bullet_physics_vert;
                static std::string bullet_physcis_frag;

                static std::string conditional_functions;

                static std::string fullscreen_quad_vertex;
                static std::string vertex_basic;
                static std::string vertex_2DAPI;
                static std::string vertex_skybox;
                static std::string particle_vertex;
                
                static std::string lighting_vert;
                static std::string stencil_passover;

                static std::string depth_of_field;

                static std::string smaa_common;
                static std::string smaa_vertex_1;
                static std::string smaa_frag_1;
                static std::string smaa_vertex_2;
                static std::string smaa_frag_2;
                static std::string smaa_vertex_3;
                static std::string smaa_frag_3;
                static std::string smaa_vertex_4;
                static std::string smaa_frag_4;

                static std::string fxaa_frag;

                static std::string forward_frag;
                static std::string particle_frag;
                static std::string deferred_frag;
                static std::string zprepass_frag;
                static std::string deferred_frag_hud;
                static std::string deferred_frag_skybox;

                static std::string copy_depth_frag;
                static std::string cubemap_convolude_frag;
                static std::string cubemap_prefilter_envmap_frag;
                static std::string brdf_precompute;

                static std::string ssao_frag;
                static std::string bloom_frag;
                static std::string hdr_frag;
                static std::string godRays_frag;
                static std::string blur_frag;
                static std::string ssao_blur_frag;

                static std::string greyscale_frag;

                static std::string final_frag;

                static std::string depth_and_transparency_frag;

                static std::string lighting_frag;
                static std::string lighting_frag_gi;
        };
    };
};

#endif
