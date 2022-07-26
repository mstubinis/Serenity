#pragma once
#ifndef ENGINE_MESH_BUILT_IN_MESHES_H
#define ENGINE_MESH_BUILT_IN_MESHES_H

#include <serenity/resources/Handle.h>
#include <serenity/system/Macros.h>
#include <array>

namespace Engine::priv {
    class BuiltInMeshses final {
        class BuiltInMeshEnum {
            public:
                enum Type : uint32_t {
                    PointLight,
                    SpotLight,
                    RodLight,
                    ProjectionLight,
                    Triangle,
                    Cube,
                    Plane,
                    Plane2D,
                    Font,
                    Particle,
                    _TOTAL
                };
                BUILD_ENUM_CLASS_MEMBERS(BuiltInMeshEnum, Type)
        };

        private:
            std::array<Handle, BuiltInMeshEnum::_TOTAL> m_BuiltInMeshes;

            bool build_point_light_mesh();
            bool build_spot_light_mesh();
            bool build_rod_light_mesh();
            bool build_projection_light_mesh();
            bool build_triangle_mesh();
            bool build_cube_mesh();
            bool build_plane_mesh();
            bool build_plane_2d_mesh();
            bool build_font_mesh();
            bool build_particle_mesh();

        public:
            BuiltInMeshses();

            [[nodiscard]] Handle getPointLightBounds();
            [[nodiscard]] Handle getSpotLightBounds();
            [[nodiscard]] Handle getRodLightBounds();
            [[nodiscard]] Handle getProjectionLightBounds();
            [[nodiscard]] Handle getTriangleMesh();
            [[nodiscard]] Handle getCubeMesh();
            [[nodiscard]] Handle getPlaneMesh();
            [[nodiscard]] Handle getPlane2DMesh();
            [[nodiscard]] Handle getFontMesh();
            [[nodiscard]] Handle getParticleMesh();

            bool init();
    };
};
#endif