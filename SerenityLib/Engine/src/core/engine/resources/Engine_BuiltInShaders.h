#pragma once
#ifndef ENGINE_BUILT_IN_SHADERS_H
#define ENGINE_BUILT_IN_SHADERS_H

namespace Engine::priv{
    class EShaders{
        public:
            static void init(const unsigned int openglVersion, const unsigned int glslVersion);

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

            static std::string blur_frag;

            static std::string normaless_diffuse_frag;

            static std::string final_frag;

            static std::string depth_and_transparency_frag;

            static std::string lighting_frag;
            static std::string lighting_frag_gi;
    };
};

#endif
