#pragma once
#ifndef ENGINE_BUILTINSHADERS_H
#define ENGINE_BUILTINSHADERS_H

#include <string>

namespace Engine{
    namespace Shaders{
        namespace Detail{
            class ShadersManagement{
                private:
                public:
					static void init();
					static std::string determinent_mat3;
                    static std::string normals_octahedron_compression_functions;
                    static std::string reconstruct_log_depth_functions;
                
                    static std::string fullscreen_quad_vertex;
                    static std::string vertex_basic;
                    static std::string vertex_hud;
                    static std::string vertex_skybox;

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

                    static std::string copy_depth_frag;

                    static std::string ssao_frag;
                    static std::string hdr_frag;
                    static std::string godRays_frag;
                    static std::string blur_frag;
                    static std::string edge_frag;
                    static std::string final_frag;
                    static std::string lighting_frag;
            };
        };
    };
};

#endif
