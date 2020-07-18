#pragma once
#ifndef ENGINE_MESH_BUILT_IN_MESHES_H
#define ENGINE_MESH_BUILT_IN_MESHES_H

class  Mesh;

namespace Engine::priv {
    class BuiltInMeshses final {
        public: struct BuiltInMeshEnum final { enum Mesh : unsigned int {
            PointLight,
            SpotLight,
            RodLight,
            ProjectionLight,
            Triangle,
            Cube,
            Plane,
            Font,
            Particle,
        _TOTAL,};};

        private:
            std::array<Mesh*, BuiltInMeshEnum::_TOTAL> m_BuiltInMeshes = { { nullptr} };

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
            BuiltInMeshses();
            ~BuiltInMeshses();

            Mesh& getPointLightBounds();
            Mesh& getSpotLightBounds();
            Mesh& getRodLightBounds();
            Mesh& getProjectionLightBounds();
            Mesh& getTriangleMesh();
            Mesh& getCubeMesh();
            Mesh& getPlaneMesh();
            Mesh& getFontMesh();
            Mesh& getParticleMesh();

            bool init();
            bool cleanup();
    };
};
#endif