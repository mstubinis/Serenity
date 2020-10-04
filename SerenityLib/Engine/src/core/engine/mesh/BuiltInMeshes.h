#pragma once
#ifndef ENGINE_MESH_BUILT_IN_MESHES_H
#define ENGINE_MESH_BUILT_IN_MESHES_H

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class BuiltInMeshses final {
        enum class BuiltInMeshEnum : unsigned int {
            PointLight,
            SpotLight,
            RodLight,
            ProjectionLight,
            Triangle,
            Cube,
            Plane,
            Font,
            Particle,
        _TOTAL};

        private:
            std::array<Handle, (size_t)BuiltInMeshEnum::_TOTAL> m_BuiltInMeshes;

            bool build_point_light_mesh();
            bool build_spot_light_mesh();
            bool build_rod_light_mesh();
            bool build_projection_light_mesh();
            bool build_triangle_mesh();
            bool build_cube_mesh();
            bool build_plane_mesh();
            bool build_font_mesh();
            bool build_particle_mesh();

        public:
            Handle getPointLightBounds();
            Handle getSpotLightBounds();
            Handle getRodLightBounds();
            Handle getProjectionLightBounds();
            Handle getTriangleMesh();
            Handle getCubeMesh();
            Handle getPlaneMesh();
            Handle getFontMesh();
            Handle getParticleMesh();

            bool init();
    };
};
#endif