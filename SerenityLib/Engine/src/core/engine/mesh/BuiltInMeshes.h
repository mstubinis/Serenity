#pragma once
#ifndef ENGINE_MESH_BUILT_IN_MESHES_H
#define ENGINE_MESH_BUILT_IN_MESHES_H

class  Mesh;

#include <vector>

namespace Engine {
    namespace epriv {
        class BuiltInMeshses final {

            public: struct BuiltInMeshEnum final {enum Mesh : unsigned int {
                PointLight,
                SpotLight,
                RodLight,
                Triangle,
                Cube,
                Plane,
                Font,
            _TOTAL,};};

            private:
                std::vector<Mesh*> m_BuiltInMeshes;

                const bool build_point_light_mesh();
                const bool build_spot_light_mesh();
                const bool build_rod_light_mesh();
                const bool build_triangle_mesh();
                const bool build_cube_mesh();
                const bool build_plane_mesh();
                const bool build_font_mesh();

            public:
                BuiltInMeshses();
                ~BuiltInMeshses();

                 Mesh& getPointLightBounds();
                 Mesh& getSpotLightBounds();
                 Mesh& getRodLightBounds();
                 Mesh& getTriangleMesh();
                 Mesh& getCubeMesh();
                 Mesh& getPlaneMesh();
                 Mesh& getFontMesh();

                const bool init();
                const bool cleanup();
        };
    };
};

#endif