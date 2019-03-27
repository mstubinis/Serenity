#pragma once
#ifndef ENGINE_MESH_IMPL_INCLUDE_GUARD
#define ENGINE_MESH_IMPL_INCLUDE_GUARD

#include <string>
#include <vector>
#include <glm/glm.hpp>

typedef unsigned int uint;

struct VertexData;
namespace Engine {
    namespace epriv {
        class  MeshCollisionFactory;
        class  MeshSkeleton;
        class MeshImpl {
            private:
                VertexData*                  m_VertexData;
                epriv::MeshCollisionFactory* m_CollisionFactory;
                epriv::MeshSkeleton*         m_Skeleton;
                std::string                  m_File;
                glm::vec3                    m_radiusBox;
                float                        m_radius;
                float                        m_threshold;
                std::vector<glm::mat4>       m_InstanceMatrices;
                uint                         m_InstanceCount;

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