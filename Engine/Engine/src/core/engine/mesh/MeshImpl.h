#pragma once
#ifndef ENGINE_MESH_IMPL_INCLUDE_GUARD
#define ENGINE_MESH_IMPL_INCLUDE_GUARD

#include <core/engine/mesh/Skeleton.h>

struct VertexData;
struct aiMesh;
struct aiScene;
struct MeshRequest;
namespace Engine {
    namespace epriv {
        class  MeshCollisionFactory;
        class  MeshLoader;
        class MeshImpl {
            friend struct ::MeshRequest;
            friend class  ::Engine::epriv::MeshLoader;
            private:
                VertexData*                    m_VertexData;
                epriv::MeshCollisionFactory*   m_CollisionFactory;
                epriv::MeshSkeleton*           m_Skeleton;
                std::string                    m_File;
                glm::vec3                      m_radiusBox;
                float                          m_radius;
                float                          m_threshold;

                void unload_cpu();
                void unload_gpu();

                void load_cpu();
                void load_gpu();

            public:
                MeshImpl();
                ~MeshImpl();
        };
    };
};

#endif